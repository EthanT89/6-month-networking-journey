/*
 * udp_listener.c -- A test program for a simple udp server that receives and prints a message
 */

// Import required modules
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "buffer_manipulation.h"

#define MYPORT "4950"

#define MAXBUFLEN 100

#define ACK "ack"

// Returns a pointer to the IPv4 or IPv6 addr:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET ){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// main program -- no args because args are read from terminal, none needed here
int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv, numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN]; // is this even used?

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) == -1){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through the results

    for (p = servinfo; p != NULL; p = p->ai_next){
        // attempt to create a socket with this address, if it fails go to the next result
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket");
            continue;
        }

        // attempt to bind the socket to the port, if it fails go to the next result
        if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) == -1){
            perror("bind");
            continue;
        }

        break;
    }

    // if p is NULL, no bindable results were found, exit the program
    if (p == NULL){
        fprintf(stderr, "listener: failed to find a usable address\n");
        exit(1);
    }

    // Now that we have a binded socket, we can listen for a sender
    printf("listening for connections with recvfrom...\n");

    addr_len = sizeof their_addr;

    while (1) {
        memset(buf, 0, sizeof buf);
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1){
            perror("listener recvfrom");
            exit(1);
        }

        unsigned int seq;

        // Need to extract sequence num first
        seq = unpacki16(buf);
        printf("Received %d bytes from sender: %d: %s", numbytes, seq, buf+2);

        // Send confirmation packet to sender
        if ((numbytes = sendto(sockfd, ACK,  strlen(ACK), 0, (struct sockaddr *)&their_addr, addr_len)) == -1){
            perror("listener ack");
            exit(1);
        }
    }

    close(sockfd);

    return 1;
}