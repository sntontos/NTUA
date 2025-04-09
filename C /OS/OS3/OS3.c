#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/select.h>

#define EXIT_INVALID_ARGS 1
#define EXIT_MALLOC_FAILED 2
#define EXIT_FORK_FAILED 3
#define EXIT_WRITE_FAILED 4


#define USAGE "Usage: ask3 <nChildren> [--random] [--round-robin]\n"

bool args_handler(int *arg1, char *arg2, int *command)
{
 
    if (*arg1 <= 0 || strlen(arg2) == 0) {
        return false;
    }

    if (strcmp(arg2, "--random") == 0) {
        *command = 2; // Random mode
    }
    else if (strcmp(arg2, "--round-robin") == 0 || strcmp(arg2, "empty") == 0) {
        *command = 1; // Round Robin mode (default)
    }
    else {
        // Any other string is invalid.
        return false;
    }
    return true;
}

int is_valid_integer(char *str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{

    if (argc != 2 && argc != 3) {
        write(STDERR_FILENO, USAGE, strlen(USAGE));
        exit(EXIT_INVALID_ARGS);
    }

    int N = atoi(argv[1]);
    char *msg;
  
    if (argc == 3) {
        msg = argv[2];
    }
    else {
        msg = "empty";
    }

    int command = 0;
 
    if (!args_handler(&N, msg, &command)) {
        write(STDERR_FILENO, USAGE, strlen(USAGE));
        exit(EXIT_INVALID_ARGS);
    }


    int (*ptc)[2] = malloc(N * sizeof(int[2]));
    int (*ctp)[2] = malloc(N * sizeof(int[2]));
    if (ptc == NULL || ctp == NULL) {
        perror("malloc failed for pipes");
        exit(EXIT_MALLOC_FAILED);
    }
    for (int i = 0; i < N; i++) {
        if (pipe(ptc[i]) == -1 || pipe(ctp[i]) == -1) {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
    }

   
    pid_t *child_pids = (pid_t *)malloc(N * sizeof(pid_t));
    if (child_pids == NULL) {
        perror("malloc failed for child_pids");
        exit(EXIT_MALLOC_FAILED);
    }


    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            free(child_pids);
            exit(EXIT_FORK_FAILED);
        }
        if (pid == 0) {
            // Child process:
            close(ptc[i][1]); // Child reads from ptc.
            close(ctp[i][0]); // Child writes to ctp.
            int num;
            while (1) {
                if (read(ptc[i][0], &num, sizeof(num)) <= 0)
                    break;
              
                num *= 2;
                sleep(5);
                if (write(ctp[i][1], &num, sizeof(num)) <= 0)
                    break;
            }
            exit(0);
        }
        if (pid > 0) {
            child_pids[i] = pid;
        }
    }


    for (int j = 0; j < N; j++) {
        close(ptc[j][0]); // Parent writes to ptc.
        close(ctp[j][1]); // Parent reads from ctp.
    }

    char *buffer = (char *)malloc(100 * sizeof(char));
    if (buffer == NULL) {
        perror("malloc failed for buffer");
        exit(EXIT_MALLOC_FAILED);
    }

    int i = 0; // Index for scheduling among children.

    while(1)
    {   
        ssize_t bytes_read = read(STDIN_FILENO, buffer, 100);
        if (bytes_read <= 0)
            continue;
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
    
        // Check if the user requested to exit.
        if (strcmp(buffer, "exit") == 0)
        {
            // Terminate all child processes.
            for (int j = 0; j < N; j++) {
                kill(child_pids[j], SIGTERM);
                waitpid(child_pids[j], NULL, 0);
            }
            free(child_pids);
            free(buffer);
            free(ptc);
            free(ctp);
            write(STDOUT_FILENO, "Child processes terminated.....\nExiting parent process.....\n", 62);
            exit(0);
        }
        // If the input is a valid integer, process it as a job.
        else if (is_valid_integer(buffer))
        {
            int num = atoi(buffer);
            char assign_msg[100];
            snprintf(assign_msg, sizeof(assign_msg), "Parent Assigned %d to child %d\n", num, child_pids[i]);
            write(STDOUT_FILENO, assign_msg, strlen(assign_msg));
    
            write(ptc[i][1], &num, sizeof(num)); // Send the job to the selected child.
    
            
            if (command == 1)
                i = (i + 1) % N;
            else if (command == 2)
                i = rand() % N;
    
            // Set up select() to wait for responses.
            fd_set readfds;
            FD_ZERO(&readfds);
            int maxfd = -1;
            for (int j = 0; j < N; j++) {
                FD_SET(ctp[j][0], &readfds);
                if (ctp[j][0] > maxfd)
                    maxfd = ctp[j][0];
            }
            struct timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            int ready = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
            if (ready == -1) {
                perror("select failed");
            }
            else if (ready == 0) {
                write(STDOUT_FILENO, "Timeout: No response from children.\n", 37);
            }
            else {
                for (int j = 0; j < N; j++) {
                    if (FD_ISSET(ctp[j][0], &readfds)) {
                        int result;
                        if (read(ctp[j][0], &result, sizeof(result)) > 0) {
                            char result_msg[100];
                            snprintf(result_msg, sizeof(result_msg), "Parent received result %d from child %d\n", result, child_pids[j]);
                            write(STDOUT_FILENO, result_msg, strlen(result_msg));
                        }
                    }
                }
            }
        }
       
        else
        {
            write(STDOUT_FILENO, "Type an integer to send a job to a child or type 'exit' to terminate!\n", 70);
        }
    }
    

    // Failsafe cleanup (if loop ever breaks)
    for (int j = 0; j < N; j++) {
        kill(child_pids[j], SIGTERM);
        waitpid(child_pids[j], NULL, 0);
    }
    free(child_pids);
    free(buffer);
    free(ptc);
    free(ctp);
    exit(0);
}
