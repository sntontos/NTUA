#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define EXIT_INVALID_ARGS 1
#define EXIT_MALLOC_FAILED 2
#define EXIT_FORK_FAILED 3

int N = 1;
pid_t *child_pids;
int PARENTTERM = 0; //Parent termination flag - not revive children

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

// Signal handler for SIGALRM, SIGUSR1, SIGUSR2
void sig_handler(int signum)
{
    if (child_pids == NULL || N <= 0) 
    {
    fprintf(stderr, "Error: child_pids is NULL or N is invalid in signal handler.\n");
    return;
    }

    if(signum == SIGTERM)
    {   
        PARENTTERM = 1;
    }

    // Forward the signal to all child processes
    for (int i = 0; i < N; i++)
    {
        if (kill(child_pids[i], signum) == -1)
        {
            perror("Failed to forward signal to child");
        }
    }

    if(signum == SIGTERM)
    {   
        printf("All child processes and parent have been terminated successfully");
        free(child_pids);
        exit(0);
    }

}

void revive_child(int i) //Revive terminated child process, i: position of terminated child on child_pids[]
{
    pid_t new_child_pid = fork();
    if(new_child_pid < 0)
    {
        perror("new child fork failed");
        exit(EXIT_FORK_FAILED);
    }

    if(new_child_pid == 0)
    {
        // Child process: execute the child program
        char *args[] = {"./child", NULL};
        if (execv("./child", args) == -1)
        {
            perror("execv failed"); // Print error if execv fails
            exit(1);
        }
    }

    else
    {
        child_pids[i] = new_child_pid;
        printf("Revived child at index %d with PID %d\n", i, new_child_pid);
    }

}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        write(STDERR_FILENO, "Error: Missing or too many arguments.\n", 38);
        write(STDERR_FILENO, "Example: ./OS_first.c 5\n", 23);
        exit(EXIT_INVALID_ARGS);
    }
    if (!is_valid_integer(argv[1]))
    {
        write(STDERR_FILENO, "N must be a positive integer.\n", 30);
        exit(EXIT_INVALID_ARGS);
    }

    N = atoi(argv[1]);
    if (N <= 0)
    {
        write(STDERR_FILENO, "N must be a positive integer.\n", 30);
        exit(EXIT_INVALID_ARGS);
    }

    child_pids = (pid_t *)malloc(N * sizeof(pid_t));

    if (child_pids == NULL)
    {
        perror("malloc failed for child_pids");
        exit(EXIT_MALLOC_FAILED);
    }

    struct sigaction sigac;
    sigac.sa_handler = sig_handler;
    sigemptyset(&sigac.sa_mask);  // Ensure no signals are blocked during handler execution
    sigac.sa_flags = SA_RESTART; // Restart syscalls interrupted by the signal
   
    // Register the signal handler for SIGUSR1, SIGUSR2, SIGTERM
    if (sigaction(SIGUSR1, &sigac, NULL) == -1)
    {
        perror("sigaction(SIGUSR1) failed");
        exit(1);
    }

    if (sigaction(SIGUSR2, &sigac, NULL) == -1)
    {
        perror("sigaction(SIGUSR2) failed");
        exit(1);
    }

    if (sigaction(SIGTERM, &sigac, NULL) == -1)
    {
        perror("sigaction(SIGTERM) failed");
        free(child_pids);
        exit(1);
    }


    // Fork N child processes
    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork failed");
            free(child_pids);
            exit(EXIT_FORK_FAILED);
        }

        if (pid == 0)
        {
            // Child process: execute the child program
            char *args[] = {"./child", NULL};
            if (execv("./child", args) == -1)
            {
                perror("execv failed"); // Print error if execv fails
                exit(1);
            }
        }
        else
        {
            // Parent process: record the PID of the child
            child_pids[i] = pid;
        }
    }

    // Parent process: wait for all child processes to finish
    for (int i = 0; i < N; i++)
    {
        int status;
        pid_t child_pid = wait(&status); // Parent waits for child to finish and check status
        if (child_pid == -1)
        {
            perror("wait failed");
            continue;
        }
        if ((WIFSIGNALED(status) || WEXITSTATUS(status) != 0) && !PARENTTERM)
        {
            printf("Child %d terminated with status %d\n", child_pid, WEXITSTATUS(status));
            for (int j = 0; j < N; j++)
            {
                if (child_pids[j] == child_pid) // Found the correct index
                {
                    revive_child(j); // Pass the correct index to revive_child
                    break;
                }
            }
        }
        else
        {
            printf("Child %d terminated with status %d by parent\n", child_pid, WEXITSTATUS(status));
        }
    }

    free(child_pids);
    return 0;
}
