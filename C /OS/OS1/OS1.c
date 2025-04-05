#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

int main(int argc,char *argv[])
{

    if(argc != 2)
    {
        write(STDOUT_FILENO, "Error: Missing or too arguments.\n", 38);
        exit(1);
    }

    int N = atoi(argv[1]);

    if(N<=0)
    {
        write(STDOUT_FILENO, "N must be a positive integer\n", 32);
        exit(1);
    }

    pid_t *child_pids = (pid_t *)malloc(N * sizeof(pid_t));
    if (child_pids == NULL)
    {
        perror("malloc failed for child_pids");
        exit(1);
    }

    for(int i = 0; i < N; i++)
    {
        pid_t pid = fork();

        if( pid < 0)
        {
            write(STDERR_FILENO, "Error: fork failed.", 30);
            free(child_pids);
            exit(1);
        }

   
        if(pid == 0)
        {   
            //Create message
            char *msg = (char*)malloc(50 * sizeof(char));
            if (msg == NULL)
            {
                perror("malloc failed for msg");
                exit(1);
            }
            int msg_len = snprintf(msg, 50, "Child process created: %d.\n", getpid());
            write(STDOUT_FILENO, msg, msg_len);
            free(msg);

            // Create the filename and open the file
            char *filename = (char*)malloc(50 * sizeof(char));
            if (filename == NULL)
            {
                perror("malloc failed for filename");
                exit(1);
            }
            snprintf(filename, 100,"output_%d.txt", getpid());
            int pd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

            // Check if file opening was successful
             if (pd == -1) 
             {
                write(STDERR_FILENO, "Error opening file", 30);
                free(child_pids);
                free(filename);
                exit(1);
            }

            // Write content to the file
            char *buffer = (char *)malloc(100 * sizeof(char));
            if (buffer == NULL)
            {
                perror("malloc failed for buffer");
                exit(1);
            }
            int len = snprintf(buffer, 100, "Child PID: %d, Parent PID: %d", getpid(), getppid());
            write(pd, buffer, len);
            close(pd);

            // Free allocated memory
            free(filename);
            free(buffer);
            exit(0);
        }
        else 
        {
            child_pids[i] = pid;
        }

    }

     // Parent process waits for all children
    for (int i = 0; i < N; i++) 
    {
        int status;
        pid_t child_pid = wait(&status); // Parent waits for child to finish and see how terminates
        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {

            printf("Child %d terminated abnormally with status %d\n", child_pid, WEXITSTATUS(status));
   
        }
    }

    write(STDOUT_FILENO, "\nParent reading file contents:\n", 31);

    for(int i=0; i < N ; i++)
{
    char *filename = (char *)malloc(50 * sizeof(char));
    if (filename == NULL)
    {
        perror("malloc failed for filename");
        exit(1);
    }
    snprintf(filename, 100,"output_%d.txt", child_pids[i]);
    int fd = open(filename, O_RDONLY);

    // Check if the file opened successfully
    if (fd == -1) {
        perror("Error opening file");
        free(filename);
        exit(1);
    }

    char *buffer = (char *)malloc(100 * sizeof(char));
    if (buffer == NULL)
    {
        perror("malloc failed for buffer");
        exit(1);
    }
    size_t bytesRead;
    while ((bytesRead = read(fd, buffer, 100)) > 0) 
    {
        // Write the contents of the buffer to the standard output (stdout)
        write(STDOUT_FILENO, buffer, bytesRead);

    }
    close(fd);
    free(filename);
    free(buffer);
    write(STDOUT_FILENO, "\n", 2);




}

    return 0;
}