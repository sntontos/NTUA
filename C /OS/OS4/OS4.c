#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>

#define default_port "41312"
#define default_host "os4.cloud.dslab.ece.ntua.gr"


char *gethost(char *hostname) {
    static char ip[INET_ADDRSTRLEN];
    struct hostent *he = gethostbyname(hostname);
    if (he == NULL) {
        perror("gethostbyname failed");
        exit(1);
    }
    inet_ntop(AF_INET, he->h_addr, ip, sizeof(ip));
    return ip; // Return the IP address as a string
}

void get_message(char *message){
    char *token;
    char buffer[64];
    time_t rawtime;
    struct tm *timeinfo;

    write(STDOUT_FILENO, "-------------------\n", 20);

    // Tokenize first word
    token = strtok(message, " ");
    if (token != NULL) {
        write(STDOUT_FILENO, "Interval(", 10);
        write(STDOUT_FILENO, token, strlen(token));
        write(STDOUT_FILENO, ")\n", 2);
    }

    // Tokenize second word
    token = strtok(NULL, " ");
    if (token != NULL) {
        write(STDOUT_FILENO, "Light Level: ", 13);
        write(STDOUT_FILENO, token, strlen(token));
        write(STDOUT_FILENO, "\n", 1);
    }

    // Tokenize third word
    token = strtok(NULL, " ");
    if (token != NULL) {
        write(STDOUT_FILENO, "Temperature: ", 13);
        int value = atoi(token); // Convert token to int, divide by 10
            float ftemp = (float)value / 100.0;
            snprintf(buffer,sizeof(buffer), "%.2f", ftemp);
        write(STDOUT_FILENO, buffer, strlen(buffer));
        write(STDOUT_FILENO, "\n", 2);
    }

    // Tokenize fourth word
    token = strtok(NULL, " ");
    if (token != NULL) {
        write(STDOUT_FILENO, "Timestamp is: ", 14);
        int value = atoi(token); // Convert token to int
        rawtime = (time_t)value;
        timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S\n\n", timeinfo);
        write(STDOUT_FILENO, buffer, strlen(buffer));
    }

}

int main(int argc, char *argv[]){

    if (argc != 6 && argc != 5 && argc != 2 && argc != 1) {
        write(STDERR_FILENO, "Missing one or too many arguments", 10);
        exit(1);
    }


    char *host = gethost(default_host);
    char *port = default_port;
    char *mode = "notdebug";

    int sock;
    struct sockaddr_in server_addr;

    if(argc == 6){
        if(strcmp(argv[1], "--host") == 0 && strcmp(argv[3], "--port") == 0 && strcmp(argv[5], "--debug") == 0){    
            if(strcmp(argv[2], "HOST") != 0){
                host = gethost(argv[2]);
            }
            if(strcmp(argv[2], "PORT") != 0){
                port = argv[4];
            }
            mode = argv[5];
        }
        else {
            write(STDERR_FILENO, "Invalid argument\n", 17);
            exit(1);
        }
    }
    else if(argc == 5){
        if(strcmp(argv[1], "--host") == 0 && strcmp(argv[3], "--port") == 0)
        {    
        if(strcmp(argv[2], "HOST") != 0){
            host = gethost(argv[2]);
        }
        if(strcmp(argv[4], "PORT") != 0){
            port = argv[4];
        }
        }
        else {
            write(STDERR_FILENO, "Invalid argument\n", 17);
            exit(1);
        }
    }
    else if(argc == 2){
        if(strcmp(argv[1], "--debug") == 0) {
            mode = argv[1];
        }
        else {
            write(STDERR_FILENO, "Invalid argument\n", 17);
            exit(1);
        }
    }



    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port)); // Server port number
    printf("Using host: %s\n", host);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) { // IP resolved for os4.cloud.dslab.ece.ntua.gr
        perror("Invalid server address");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sock);
        return 1;
    }

    struct pollfd fds[2];
    fds[0].fd = sock;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    char buffer[1024];
    char received[1024];


    while (1) {
        int ret = poll(fds, 2, -1); // Wait indefinitely for events

        if (ret < 0) {
            perror("Poll error");
            break;
        }

        if (fds[0].revents & POLLIN) { // Data from server
            ssize_t bytes = recv(sock, received, sizeof(received) - 1, 0);
            received[strcspn(received, "\n")] = '\0'; // Remove newline character
            if (bytes <= 0) {
                write(STDERR_FILENO, "Server closed connection\n", 24);
                break;
            }
            received[bytes] = '\0';

            if(strcmp(received, "try again") == 0){
                write(STDOUT_FILENO, "Wrong input type 'help'\n", 25);
                continue;
            }

            if(strcmp(mode, "--debug") == 0){
                write(STDOUT_FILENO, "[DEBUG] received '", 18);
                write(STDOUT_FILENO, received, bytes);
                write(STDOUT_FILENO, "'\n", 3);
            }
            else {
                write(STDOUT_FILENO, received, bytes);
                write(STDOUT_FILENO, "\n", 1);
            }

            if (strcmp(buffer, "get\n") == 0) {
                get_message(received);
            }
            
        }

        if (fds[1].revents & POLLIN) { // User input ready
            if (read(STDIN_FILENO, buffer, sizeof(buffer) - 1) <= 0) {
                write(STDERR_FILENO, "Error reading from stdin\n", 25);
                break;
            }
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
            if (strcmp(buffer, "exit") == 0) {
                write(STDOUT_FILENO, "Exiting...\n", 11);
                break;
            }
            if (strcmp(buffer, "help") == 0) {
                write(STDOUT_FILENO, "Actions type:\n  'exit' o terminate\n  'get' to retrieve data from server \n  'No. Name Surname Reason' to ask for permission\n", 125);
                continue;
            }
            if(strcmp(mode, "--debug") == 0){
                write(STDOUT_FILENO, "\n[DEBUG] sent '", 15);
                write(STDOUT_FILENO, buffer, strlen(buffer));
                write(STDOUT_FILENO, "'\n", 3);
            }
            strcat(buffer, "\n");
            send(sock, buffer, strlen(buffer), 0);

            
        }
    }

    close(sock);
    return 0;
}