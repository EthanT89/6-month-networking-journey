/*
 * udp_client_vc.c -- A test program for a simple udp server that sends and prints a message through a vc
 * This is a copy of udp_listener.c that will be adjusted for virtual connection needs
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
#include <sys/time.h>
#include "./utils/buffer_manipulation.h"
#include "./utils/ack.h"
#include <poll.h>
#include <stdbool.h>

#define SERVERPORT "1209"
#define MAXBUFSIZE 100
#define APPID 1122
#define MAXPENDINGPACKETS 100

// dev configs
#define ACK_ENABLED 0
#define SIM_PACKET_DROP 1
#define PACKET_DROP_RATE 30

void send_packet(
            unsigned char packet[MAXBUFSIZE], 
            int sockfd,
            size_t len, 
            __CONST_SOCKADDR_ARG  addr,
            socklen_t addrlen
        )
{
    bool drop = false;
    int numbytes;
    unsigned int packet_id = unpacki16(packet+2);
    
    // Simulate packets dropping
    if (SIM_PACKET_DROP){
        int random = rand() % 100;
        if (random < PACKET_DROP_RATE) {
            printf("[SIM] Packet #%u dropped\n", packet_id);
            drop = true;
        }
    }
    
    if (!drop && (numbytes = sendto(sockfd, packet, len+4, 0, addr, addrlen)) == -1){
        perror("talker: sendto");
        exit(1);
    }

    if (!drop) {
        printf("[SEND] Packet #%u sent (%d bytes)\n", packet_id, numbytes);
    }
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int insert_at_pending = 0;

    struct pending_packet pending_packets[MAXPENDINGPACKETS];
    memset(pending_packets, 0, sizeof pending_packets);

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
    char input[MAXBUFSIZE - 4];
    unsigned char rec[12];

    printf("\n╔════════════════════════════════════╗\n");
    printf("║   UDP Client Virtual Connection    ║\n");
    printf("╚════════════════════════════════════╝\n\n");

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

    unsigned int packet_id = 1;
    unsigned int pkts_ack_pending[100];

    fgets(input, MAXBUFSIZE - 4, stdin);

    while (1) {

        if (strcmp(input, "0") == 0) {
            printf("\n[CLIENT] Exiting program...\n");
            break;
        }

        int buf_index = 0;

        if (packet_id > 99) { return 1; }

        // Need to pack sequence number and input into one buffer.
        construct_packet(packet_id++, input, buf);

        for (int i = 0; i < 5; i++){
            int ready = poll(pfds, 1, 10);
            if (ready){
                if (recvfrom(sockfd, rec, 12, 0, p->ai_addr, &p->ai_addrlen) == -1){
                    perror("listener recvfrom");
                    exit(1);
                }
                int protocol_id, ack;
                unsigned char ack_bitmap[4];
                unsigned int bits[32];
                unsigned int all_acks[33];
                deconstruct_ack_packet(&protocol_id, &ack, ack_bitmap, rec);

                if (protocol_id != APPID){
                    continue;
                }

                int updates;

                unpack_acks(ack_bitmap, bits);
                all_acks[0] = ack;
                memcpy(&all_acks[1], bits, sizeof(bits));
                convert_bits_to_id(all_acks);
                update_pending_pkts(pending_packets, all_acks);
            }
        }

        for (int i = 0; i < MAXPENDINGPACKETS; i++){
            struct pending_packet *cur_packet = &(pending_packets[i]);

            if (cur_packet->packet_id == 0 || cur_packet->ack == 1) {
                continue;  // Skip empty or already-acked slots
            }

            if (get_seconds() - cur_packet->time_sent >= 1){
                // Need to resent packet.
                unsigned char new_packet[MAXBUFSIZE];
                construct_packet(cur_packet->packet_id, cur_packet->msg, new_packet);
                printf("[RETX] Timeout - resending packet #%u\n", cur_packet->packet_id);
                send_packet(new_packet, sockfd, strlen(cur_packet->msg), p->ai_addr, p->ai_addrlen);
                add_pending_packet(cur_packet->packet_id, pending_packets, &insert_at_pending, cur_packet->msg);
                cur_packet->ack = 1;
            }
        }

        send_packet(buf, sockfd, strlen(input), p->ai_addr, p->ai_addrlen);
        add_pending_packet(packet_id, pending_packets, &insert_at_pending, input);
    }

    freeaddrinfo(servinfo);
    close(sockfd);
}