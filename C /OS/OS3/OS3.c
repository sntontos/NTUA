#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/select.h>

bool args_handler(int *arg1, char *arg2, int *command)
{
    if (*arg1 <= 0 || strlen(arg2) == 0){ return false; }
    
    if (arg2 == NULL || strcmp(arg2, "") == 0 || strcmp(arg2, "--random") == 0)
    {
        *command = 2; // Random
    }
    else if(strcmp(arg2, "--round-robin") == 0)
    {
        *command = 1; // Round Robin
    }
    else
    {
        return false; // Invalid command
    }
    return true;
}

int is_valid_integer(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (!isdigit(str[i]))
        {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        write(STDERR_FILENO, "Error: Missing or too many arguments.\n", 38);
        write(STDERR_FILENO, "Example: ./OS_first.c 5 --round-robin\n", 38);
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);
    char *msg = argv[2];
    int command = 0;
    int ptc[N][2]; // Parent to child
    int ctp[N][2]; // Child to parent
    for (int i = 0; i < N; i++) {
        if (pipe(ptc[i]) == -1 || pipe(ctp[i]) == -1) {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
    }

    pid_t *child_pids = (pid_t *)malloc(N * sizeof(pid_t));
    if (child_pids == NULL)
    {
        perror("malloc failed for child_pids");
        exit(EXIT_FAILURE);
    }

    if(!args_handler(&N, msg, &command))
    {
        write(STDERR_FILENO, "Error: Invalid arguments.\n", 26);
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < N; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
            free(child_pids);
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            //child process
            close(ptc[i][1]); // Close the write end of the pipe
            close(ctp[i][0]); // Close the read end of the pipe
            int num;
            while(1)
            {
                if (read(ptc[i][0], &num, sizeof(num)) <= 0) break;
                num *= 2; // Example operation: double the number
                sleep(5);
                if (write(ctp[i][1], &num, sizeof(num)) <= 0) break;
            }
        }
        if (pid > 0)
        {
            child_pids[i] = pid;
        }
    }

    // Parent process
    for (int j = 0; j < N; j++) {
        close(ptc[j][0]); // Parent closes read end of ptc
        close(ctp[j][1]); // Parent closes write end of ctp
    }

    char *buffer = (char *)malloc(100 * sizeof(char));
    if (buffer == NULL)
    {
        perror("malloc failed for buffer");
        exit(EXIT_FAILURE);
    }

    int i = 0;

    while(1)
    {
        printf("Enter a number or type help: ");
        scanf("%s", buffer);
        if(strcmp(buffer, "help") == 0)
        {
            printf("Type a number to send job to a child!");
        }
        if(is_valid_integer(buffer))
        {
            int num = atoi(buffer);
            printf("Parent Assigned %d to child %d\n", num, child_pids[i]);

            write(ptc[i][1], &num, sizeof(num)); // Write to the pipe

            // Use select to wait for response from any child
            fd_set readfds;
            FD_ZERO(&readfds);
            int maxfd = -1;

            for (int j = 0; j < N; j++) {
                FD_SET(ctp[j][0], &readfds);
                if (ctp[j][0] > maxfd) maxfd = ctp[j][0];
            }

            struct timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;

            int ready = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
            if (ready == -1) {
                perror("select failed");
            } else if (ready == 0) {
                printf("Timeout: No response from children.\n");
            } else {
                for (int j = 0; j < N; j++) {
                    if (FD_ISSET(ctp[j][0], &readfds)) {
                        int result;
                        if (read(ctp[j][0], &result, sizeof(result)) > 0) {
                            printf("Parent received result %d from child %d\n", result, child_pids[j]);
                        }
                    }
                }
            }
        }
        else if (strcmp(buffer, "exit") == 0)
        {
            for (int j = 0; j < N; j++)
            {
                kill(child_pids[j], SIGTERM); // Terminate child processes
                waitpid(child_pids[j], NULL, 0); // Wait for child processes to finish
            }
            free(child_pids);
            free(buffer);
            printf("Child processes terminated.....\nExiting parent process.....\n");
            exit(0);
        }

        if(command == 1)
        {
            i = (i + 1) % N; // Round Robin
        }
        else if(command == 2)
        {
            i = rand() % N; // Random
        }
    }

    // Failsafe cleanup (if somehow loop breaks)
    for (int j = 0; j < N; j++) {
        kill(child_pids[j], SIGTERM);
        waitpid(child_pids[j], NULL, 0);
    }
    free(child_pids);
    free(buffer);
    exit(0);
}