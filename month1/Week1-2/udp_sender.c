/*
 * udp_sender.c -- a test program for a udp server and client, sends a single message
 */

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
#include <time.h>
#include "buffer_manipulation.h"
#include <poll.h>

#define SERVERPORT "4950"
#define MAXBUFSIZE 100

int main(void)
{
    int sockfd;
    unsigned int seq = 0;
    struct addrinfo hints, *servinfo, *p;
    int rv, numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo);
    if (rv != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through the results and make a socket, no need to bind
    for (p = servinfo; p != NULL; p = p->ai_next){

        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){
            perror("socket");
            continue;
        }
        break;
    }

    // if p = NULL, no valid results
    if (p == NULL){
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }


    char buf[MAXBUFSIZE];
    char input[MAXBUFSIZE - 2];
    char rec[4];

    printf("starting client program...\n\n");

    /* 
     * We want to test sending multiple packets out with sequence identifiers. So, we will track an index
     * variable on each loop, representing each packet sent. This helps give sequence and ordering to the
     * server, but also will allow for ACK's.
     * 
     * Will implement re-sending for ACK's momentarily.
     */
    struct pollfd pfds[1];
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;

    seq = 0;
    while (1) {

        // clear the buf
        memset(buf, 0, sizeof buf);
        fgets(input, MAXBUFSIZE - 2, stdin);
        if (strcmp(input, "0") == 0) {
            printf("exiting program...\n");
            break;
        }

        // Need to pack sequence number and input into one buffer.
        packi16(buf, seq++);
        append_buf_after_sequence(buf, input);
        
        // Simulate packets dropping
        
        int random = rand() % 100;
        if (random > 50){            
            if ((numbytes = sendto(sockfd, buf, strlen(input) + 2, 0, p->ai_addr, p->ai_addrlen)) == -1){
                perror("talker: sendto");
                exit(1);
            }
        }
        printf("msg sent: %s", buf+2);

        // Polls for receiving events, if there are none, poll() will come back = 0
        // If there are none, we did not receive an ACK package.
        // This is very specific to our use case, and just a POC
        while (poll(pfds, 1, 1000) == 0){
            printf("ack not received, resending...\n");
            // if poll comes back 0, resend the packet.
            if ((numbytes = sendto(sockfd, buf, strlen(input) + 2, 0, p->ai_addr, p->ai_addrlen)) == -1){
                perror("talker: sendto");
                exit(1);
            }
        }

        if ((numbytes = recvfrom(sockfd, rec, 3, 0, p->ai_addr, &p->ai_addrlen)) == -1){
            perror("listener recvfrom");
            exit(1);
        }
        printf("message successfully sent!\n\n");
    }

    freeaddrinfo(servinfo);
    close(sockfd);
}