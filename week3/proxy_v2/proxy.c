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
 * Stats -- Custom struct to manage packet statistics
 *
 * packets_received -- total packets received in this session
 * packets_forwarded -- total packets forwarded in this session
 * packets_dropped -- total packets dropped in this session
 * latency -- latency in milliseconds
 */
struct Stats {
    int packets_received;
    int packets_forwarded;
    int packets_dropped;
    int latency; // TODO: is this really necessary?
};

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

/*
 * handle_data() -- handle all incoming packets passing through the proxy. Adds the new packet to the packet queue
 */
void handle_data(int sockfd, struct Packets *packets, struct Stats *stats){
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

    stats->packets_received++;

    int offset = sizeof(struct sockaddr);
    memcpy(rawdata, buf+offset, bytes_received-offset);

    // Pack sender address
    prepend_address(their_addr, final_packet, rawdata, bytes_received-offset);

    // Extract destination address
    struct sockaddr *destination_addr = malloc (sizeof *destination_addr);
    memcpy(destination_addr->sa_data, buf, sizeof(destination_addr->sa_data));
    memcpy(&destination_addr->sa_family, buf+sizeof(destination_addr->sa_data), sizeof(destination_addr->sa_family));

    // Create delayed packet struct
    struct Packet *packet = malloc(sizeof *packet);
    packet->bytes = bytes_received;
    memcpy(packet->data, final_packet, MAXBUFSIZE);
    packet->dest_addr = destination_addr;
    packet->dest_addrlen = sizeof(*destination_addr);
    packet->time_received = get_time_ms();
    packet->next = NULL;

    // queue the packet
    enqueue_packet(packets, packet);
}

/*
 * send_packet() -- unpack a packet struct and send to the indicated destination
 */
void send_packet(int sockfd, struct Packet *packet, struct Stats *stats){
    if ((rand() % 100 + 1) < DROP_RATE){
        stats->packets_dropped++;
        return;
    }
    int bytes_sent;
    if ((bytes_sent = sendto(sockfd, packet->data, packet->bytes, 0, packet->dest_addr, packet->dest_addrlen)) == -1){
        perror("proxy: sendto");
        return;
    }
    stats->packets_forwarded++;
}

/*
 * main() -- create and start the proxy server.
 *
 * Setup:
 * - create socket
 * - create poll fd struct
 * - create packet queue
 * - create baseline stats
 * 
 * Loop:
 * - Check for incoming data and handle it if present
 * - Check if any packets are ready to be sent, send them if so
 * - Print the session stats every 900ms
 */
int main(int argc, char **argv){
    int sockfd = get_socket();
    int last_update = 0;

    // Setup poll fd struct for data handling that does not block the program
    struct pollfd *pfds = malloc(sizeof *pfds);
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    pfds[0].revents = 0;

    // Setup packet queue struct to manage incoming packets
    struct Packets *packets = malloc(sizeof *packets);
    packets->delay_ms = DELAY_MS;
    packets->head = NULL;
    packets->tail = NULL;
    packets->pkt_count = 0;

    // Create stats struct to contain all packet statistics
    struct Stats *stats = malloc(sizeof *stats);
    stats->packets_dropped = 0;
    stats->packets_forwarded = 0;
    stats->packets_received = 0;

    // If user entered a command line argument, interpret it as the new delay time in ms
    if (argc > 1){
        int delay;
        sscanf(argv[1], "%d", &delay);
        packets->delay_ms = delay;
        printf("delay set to %dms\n", delay);
    }
    stats->latency = packets->delay_ms;

    printf("setup complete!\n");
    while (1){
        // Check for incoming packets
        if (poll(pfds, 1, 0) >= 1){
            printf("Received data. Routing...\n");
            handle_data(sockfd, packets, stats);
        }

        // Check if any packets are ready to be sent after a specified delay
        if (ready_to_send(packets) == 1){
            send_packet(sockfd, pop_packet(packets), stats);
        }

        // Print the current statistics every 900ms - TODO: enhance the statistics by 1) modularizing the printing functionality
        // and 2) exporting the stats somehow (file/database)
        if (interval_elapsed_cur(last_update, 900) == 1){
            printf("\n=== Current Stats ===\n");
            printf("Packets Received: %d\n", stats->packets_received);
            printf("Packets Forwarded: %d\n", stats->packets_forwarded);
            printf("Packets Dropped: %d\n", stats->packets_dropped);
            printf("Latency: %dms\n\n", stats->latency);
            last_update = get_time_ms();
        }

    }
}