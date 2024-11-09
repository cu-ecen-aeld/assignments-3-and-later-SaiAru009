#define _POSIX_C_SOURCE 200112L  // Enable POSIX features

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdbool.h>

#define PORT "9000"
#define BACKLOG 10
#define FILENAME "/var/tmp/aesdsocketdata"
#define BUF_SIZE 65536

int sockfd, clientfd;
struct addrinfo *servinfo;

void handle_signal(int sig) {
    if(sig == SIGTERM || sig == SIGINT) {
        printf("Caught signal, exiting");
        freeaddrinfo(servinfo);
        remove(FILENAME);
        close(clientfd);
        close(sockfd);
        exit(0);
    }
}

bool create_daemon() {
    bool status = false;
    pid_t pid, sid;

    // Fork the first time to create the child process
    pid = fork();
    if (pid < 0) {
        // If fork() fails, exit
        perror("Fork failed");
        return status;
    }

    if (pid > 0) {
        // Parent process exits, so the child becomes the daemon
        exit(EXIT_SUCCESS);
    }

    // Child process continues

    // Create a new session and detach from the terminal
    sid = setsid();
    if (sid < 0) {
        perror("Failed to create session");
        return status;
    }

    // Change the working directory to root
    if (chdir("/") < 0) {
        perror("Unable to change daemon's working dir");
        return status;
    }

    // Redirect standard file descriptors (stdin, stdout, stderr)

    return true;
}

int main(int argc, char *argv[]) 
{
    // int sockfd = socket(PF_INET6, SOCK_STREAM, 0);  //Creating a socket
    // if (sockfd == -1) return -1;

    struct addrinfo hints;
    struct sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(NULL, PORT, &hints, &servinfo) !=0) {
        perror("Error in getaddrinfo");
        return -1;
    }

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);  //Creating a socket
    if (sockfd == -1) {
        perror("Unable to create socket");
        freeaddrinfo(servinfo);
        return -1;
    }

    // Set SO_REUSEADDR option (allows newly created socket to reuse port address)
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt failed");
        freeaddrinfo(servinfo);
        close(sockfd);
        return -1;
    }

    if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) !=0) {
        perror("Bind failed");
        freeaddrinfo(servinfo);
        close(sockfd);
        return -1;
    }

    if(listen(sockfd, BACKLOG) < 0) {
        perror("Error in listen");
        freeaddrinfo(servinfo);
        close(sockfd);
        return -1;
    }

    // Implementing signal handler
    struct sigaction sa;
    sa.sa_handler = handle_signal;

    // Register signal handlers for SIGINT and SIGTERM
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting SIGINT handler");
        freeaddrinfo(servinfo);
        close(sockfd);
        return -1;
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error setting SIGTERM handler");
        freeaddrinfo(servinfo);
        close(sockfd);
        return -1;
    }

    if (argc>1 && strcmp(argv[1], "-d") == 0) {
        printf("Daemon to be created\n");
        if(create_daemon() == false) {
            freeaddrinfo(servinfo);
            close(sockfd);
            return -1;
        }
    }

    //Open file to append data acquired from client
    FILE *file = fopen(FILENAME, "w+");
    if (file == NULL) {
        perror("Failed to open file");
        freeaddrinfo(servinfo);
        close(sockfd);
        return -1;
    }

    while(1) {
        clientfd = accept(sockfd, (struct sockaddr *) &clientAddr, (socklen_t*)&addrLen);
        if(clientfd == -1) {
            perror("Error in accept");
            freeaddrinfo(servinfo);
            close(sockfd);
            return -1;
        }

        // Get and print the client's IP address
        char *client_ip = inet_ntoa(clientAddr.sin_addr);  // Convert IP to string
        printf("Accepted connection from %s\n", client_ip);

        //Receive data from client socket
        char buffer[BUF_SIZE];

        int bytes_received = recv(clientfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received == -1) 
        {
            printf("Failed to receive from client\n");
            close(clientfd);
            continue;
        }
        buffer[bytes_received] = '\0';

        // printf("Received data: %s", buffer);

        fprintf(file, "%s", buffer);

        // Check if data ends with newline
        // if (buffer[bytes_received - 1] == '\n') {
        //     printf("Received file contents (including newline):\n");
        // } else {
        //     printf("Received file contents without newline:\n");
        // }

        //     // Append the received data to the file
        //     if (fputs(buffer, file) == EOF) {
        //         perror("Failed to write to file");
        //         break;
        //     }

        // if (bytes_received == 0) {
        //     printf("Client disconnected.\n");
        // } else if (bytes_received < 0) {
        //     perror("recv failed");
        // }

        //Reset the file pointer
        fflush(file);
        rewind(file);

        //Send data back to client
        // char send_buf[BUF_SIZE];
        // while (fgets(send_buf, sizeof(send_buf), file) != NULL) 
        // {
        //     if (send(clientfd, send_buf, strlen(send_buf), 0) == -1) 
        //     {
        //         perror("Error sending file data");
        //         break;
        //     }
        // }

        // char sendBuf[BUF_SIZE];
        while ((bytes_received = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            if (send(clientfd, buffer, bytes_received, 0) == -1) {
                perror("Error sending file data");
                break;
            }
        }

        printf("Closing connection from %s\n", client_ip);
        close(clientfd);
    }
}