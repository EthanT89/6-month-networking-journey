/*
 * pollclient.c -- a client focused test to create a socket and communicate with the poll server.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT "9034"

#define MAXDATASIZE 100

int main(void)
{
    // Declare variables
    int sockfd, len, bytes_received;
    struct addrinfo hints, *res, *p;
    char *msg;
    char output[MAXDATASIZE];
    msg = "Hello I am here!\n";


    len = strlen(msg);

    // can't forget to set the hints memory to 0.
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    // First I need to get the addrinfo for the server?
    if (getaddrinfo("localhost", PORT, &hints, &res ) != 0 ) {
        perror("getaddrinfo");
        exit(1);
    }

    // After calling addrinfo, I will loop through the results and find a usable port
    for (p = res; p != NULL; p = p->ai_next){

        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd <= 0){
            perror("socket");
            continue;
        }

        // Successfully created socket, leave loop
        break;
    }

    if (p == NULL){
        perror("socket");
        exit(1);
    }

    // Once I create a socket, I will be able to connect to the server. 
    // I will call connect() and then send() a message. once I send a message, I will recv() a server message.
    printf("Attempting connection to socket %d...\n", sockfd);
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) != 0){
        perror("connect");
        exit(1);
    }


    printf("Successfully connected to socket %d!!\n", sockfd);

    freeaddrinfo(res);

    printf("Sending message: %s\n", msg);
    send(sockfd, msg, len, 0);
    printf("Successfully sent message!\n");

    

    while (1) {
        bytes_received = recv(sockfd, output, MAXDATASIZE, 0);
        if (bytes_received <= 0){
            if (bytes_received == 0){
                printf("Server closed connection. :(");
                break;
            } else {
                perror("recv");
                break;
            }
        } else {
            printf("Message received: %s\n", output);
        }    
    }
    
    printf("broke the while loop, exiting\n");
    close(sockfd);
}
    