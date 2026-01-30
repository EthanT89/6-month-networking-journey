/*
 * udp_server_vc.c -- A quick demo illustrating knowledge of udp virtual connections. This is a rough draft and first attempt
 * at setting up a simple 'connection' on UDP
 */

// Include necessary libraries -- TODO: memorize these libraries and better understand their usage
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
#include "./utils/buffer_manipulation.h"
#include "./utils/ack.h"


// Define program-wide variables
#define PORT "4950"
#define MAXBUFSIZE 100
#define APPID 1122
#define MAXPENDINGPACKETS 100

 
/*
 * is_complete_sequence -- Checks to see if a sequence of packets is consistent, ie there are no dropped packets
 * returns 0 on success, 1 if any packets were missing. First n elements in seq is replaced with the seq id of each
 * of the n packets that are missing.
 * 
 * 
 */

/*
 * main() -- the mother function for this server (yes, it should be broken down, but I am still learning.)
 */
int main(void){

    // establish variables -- TODO: understand each variable usage better
    int sockfd, rv, bytes_received;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage *theiraddr;
    socklen_t addr_len;
    unsigned char buf[MAXBUFSIZE];
    unsigned char msg[MAXBUFSIZE-2];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    
    // First, get the address info. Use hints for basic standards and servinfo for 
    // storing the results (p for looping). Both of type addrinfo
    if ( (rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "server getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    printf("\n╔════════════════════════════════════╗\n");
    printf("║   UDP Server Virtual Connection    ║\n");
    printf("╚════════════════════════════════════╝\n\n");

    printf("[SERVER] Attempting to create and bind socket...\n");
    for (p = servinfo; p != NULL; p = p->ai_next){
        printf("[SOCKET] Trying candidate addrinfo entry...\n");

        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("server: bind");
            continue;
        }

        printf("[SOCKET] Socket created and bound successfully\n");
        break;
    }

    if (p == NULL){
        perror("server: getaddr");
        exit(1);
    }

    addr_len = sizeof theiraddr;

    printf("[SERVER] Listening for connections on port %s...\n\n", PORT);
    // Now, a main socket was setup. We can create a main loop and whatever we want to do from here on out.
    // We will need a 'theiraddr' to recvfrom messages, as well as their addr len.
    // The loop works as follows:
    //     - First, wait for a message using recvfrom()
    //     - Second, determine if the sender is valid (either id or known addr)
    //     - Third, Handle the message (store new addr, respond, update, ignore, etc.)
    unsigned int remote_ack = 1;
    unsigned char remote_ack_bitmap[4];
    memset(remote_ack_bitmap, 0, sizeof remote_ack_bitmap);

    while (1){

        // wait for msg
        memset(buf, 0, sizeof buf);
        if (( bytes_received = recvfrom(sockfd, buf, MAXBUFSIZE, 0, (struct sockaddr *)&theiraddr, &addr_len)) == 0){
            perror("recvfrom");
            exit(1);
        }

        int protocol_id, packet_id;
        unpack_msg_metadata(buf, &protocol_id, &packet_id);
        strcpy(msg, buf+4);

        printf("[RECV] Packet #%d (%d bytes)\n", packet_id, bytes_received);

        if (protocol_id != APPID){
            continue;
        }

        unsigned char ack_buf[12];
        update_outgoing_acks(&remote_ack, packet_id, remote_ack_bitmap);
        construct_ack_packet(remote_ack, remote_ack_bitmap, ack_buf);
        int bytes_sent;

        if ((bytes_sent = sendto(sockfd, ack_buf, 12, 0,  (struct sockaddr *)&theiraddr, addr_len)) == 0){
            perror("send ack");
        }

        printf("[MSG ] Packet #%d: %s (%zu bytes)\n\n", packet_id, msg, strlen((char *)msg));

    }
    close(sockfd);
    return 1;
}