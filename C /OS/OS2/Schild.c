#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int count = 0;
long start;
// Signal handler for SIGALRM
void sig_handler(int signum) {

    if(signum == SIGALRM)
    {
        printf("PPID: %d | PID: %d | Count: %d | Time: %ld \n", getppid(), getpid(), count, (time(NULL)-start));
        alarm(10);
    }

    if(signum == SIGUSR1)
    {   
        count++;
    }
    if(signum == SIGUSR2)
    {   
        count--;
    }
    if (signum == SIGINT) 
    {
    printf("\nExiting by SIGINT \n");
    exit(0);
    }

// Exit the program
}

int main() {
    time_t tstart = time(NULL);
    start = tstart;
    // Register the alarm handler for SIGALRM, SIGUSR1, SIGUSR2, SIGTERM
    struct sigaction sigac;
    memset(&sigac, 0, sizeof(sigac));
    sigac.sa_handler = sig_handler;
    sigemptyset(&sigac.sa_mask);
    sigac.sa_flags = 0;

    // Register the signal handler for SIGALRM
    if (sigaction(SIGALRM, &sigac, NULL) == -1) {
        perror("sigaction(SIGALRM) failed");
        exit(1);
    }

    // Register the signal handler for SIGUSR1
    if (sigaction(SIGUSR1, &sigac, NULL) == -1) {
        perror("sigaction(SIGUSR1) failed");
        exit(1);
    }

    // Register the signal handler for SIGUSR2
    if (sigaction(SIGUSR2, &sigac, NULL) == -1) {
        perror("sigaction(SIGUSR2) failed");
        exit(1);
    }

    // Register the signal handler for SIGINT case
    if (sigaction(SIGINT, &sigac, NULL) == -1) {
        perror("sigaction(SIGINT) failed");
        exit(1);
    }

    // Set an alarm for 5 seconds
    alarm(10);

    while (1)
    {
    // Keep the program running until the alarm goes off
    }

    return 0;
}