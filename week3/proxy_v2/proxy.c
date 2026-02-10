/*
 * proxy.c -- A simple proxy wrapper that intermediates two or more servers. Handles routing and data management, offering 
 * custom latency simulation.
 */

// Custom utility files
#include "./proxy_config.h"
#include "./utils/proxy_utils.h"
#include "../utils/time_custom.h"
#include "./utils/delayed_packet.h"

// Standard socket/server libraries
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <netinet/in.h>

/*
 * get_socket() -- create a socket and return its file descriptor
 */
int get_socket(){
    int sockfd, rv;
    int yes = 1;
    struct addrinfo hints, *results, *p;


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // first, use getaddrinfo() to find some addresses!
    if ((rv = getaddrinfo(NULL, PROXY_PORT_S, &hints, &results)) != 0){
        perror("server: getaddrinfo\n");
        exit(1);
    }

    for (p = results; p != NULL; p = p->ai_next){
        // first, try to make a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket\n");
            continue;
        }

        // no more "address already in use" error :)
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("server: bind\n");
            continue;
        }

        printf("Successfully created socket!\n");
        break;
    }

    if (p == NULL){
        perror("server: start\n");
        exit(1);
    }

    return sockfd;
}

void handle_data(int sockfd, struct Packets *packets){
    unsigned char buf[MAXBUFSIZE];
    unsigned char final_packet[MAXBUFSIZE];
    unsigned char rawdata[MAXBUFSIZE];

    struct sockaddr *their_addr = malloc(sizeof *their_addr);
    socklen_t their_len = sizeof *their_addr;
    int bytes_received;

    memset(buf, 0, MAXBUFSIZE);
    memset(final_packet, 0, MAXBUFSIZE);
    memset(rawdata, 0, MAXBUFSIZE);

    // Accept data
    if ((bytes_received = recvfrom(sockfd, buf, MAXBUFSIZE, 0, their_addr, &their_len)) == -1){
        perror("proxy: recvfrom");
        return;
    }

    int offset = sizeof(struct sockaddr);
    memcpy(rawdata, buf+offset, bytes_received-offset);

    // Pack sender address
    prepend_address(their_addr, final_packet, rawdata, bytes_received-offset);

    // Extract destination address
    struct sockaddr *destination_addr = malloc (sizeof *destination_addr);
    memcpy(destination_addr->sa_data, buf, sizeof(destination_addr->sa_data));
    memcpy(&destination_addr->sa_family, buf+sizeof(destination_addr->sa_data), sizeof(destination_addr->sa_family));

    struct Packet *packet = malloc(sizeof *packet);
    packet->bytes = bytes_received;
    memcpy(packet->data, final_packet, MAXBUFSIZE);
    packet->dest_addr = destination_addr;
    packet->dest_addrlen = sizeof(*destination_addr);
    packet->time_received = get_time_ms();
    packet->next = NULL;

    enqueue_packet(packets, packet);
}

void send_packet(int sockfd, struct Packet *packet){
    int bytes_sent;
    if ((bytes_sent = sendto(sockfd, packet->data, packet->bytes, 0, packet->dest_addr, packet->dest_addrlen)) == -1){
        perror("proxy: sendto");
        return;
    }
}

int main(){
    int sockfd = get_socket();

    struct pollfd *pfds = malloc(sizeof *pfds);
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    pfds[0].revents = 0;

    struct Packets *packets = malloc(sizeof *packets);
    packets->delay_ms = DELAY_MS;
    packets->head = NULL;
    packets->tail = NULL;
    packets->pkt_count = 0;

    while (1){
        if (poll(pfds, 1, 0) >= 1){
            printf("Received data. Routing...\n");

            handle_data(sockfd, packets);
        }
    }
}