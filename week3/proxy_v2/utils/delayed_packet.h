/*
 * delayed_packet.h -- structures and logic for delayed packets. intended to be used with the proxy server in week 3
 */

#ifndef DELAYED_PACKET_H
#define DELAYED_PACKET_H

#include "../../common.h"
#include "../../utils/time_custom.h"
#include <sys/socket.h>
#include <stddef.h>

/*
 * Packet -- A struct defining a delayed packet
 *
 * data -- original packet data (untouched)
 * bytes -- total byte length of original function
 * *dest_addr -- sockaddr of destination address
 * dest_addrlen -- len of sockaddr of destination address
 * time_received -- time, in ms, that the packet was received. Limited to a 0-999 circular number array\
 * *next -- pointer to the next packet in queue
 */
struct Packet {
    // Content
    unsigned char data[MAXBUFSIZE];
    int bytes;

    // Destination Details
    struct sockaddr *dest_addr;
    socklen_t dest_addrlen;

    // Latency Simulation
    int time_received;

    struct Packet *next;
};

/*
 * Packets -- linked list-based queue of packets waiting to be sent out
 *
 * delay_ms -- time, in ms, that every packet will be delayed before being sent out
 * pkt_count -- total packets in queue
 * *head -- pointer to the first packet in queue
 * *tail -- pointer to the last packet in queue
 */
struct Packets {
    int delay_ms;
    int pkt_count;

    struct Packet *head;
    struct Packet *tail;
};

/*
 * enqueue_packet() -- enqueue a packet to the packets struct. requires the packet to be constructed beforehand
 */
void enqueue_packet(struct Packets *packets, struct Packet *packet);

/*
 * pop_packet() -- pop off and return a pointer to the next packet in queue (FIFO)
 */
struct Packet* pop_packet(struct Packets *packets);

/*
 * peek_packet() -- return a const pointer to the next packet in queue
 */
struct Packet* peek_packet(struct Packets *packets);

/*
 * is_empty_packets() -- returns 1 if packets struct is empty, 0 otherwise
 */
int is_empty_packets(struct Packets *packets);

/*
 * ready_to_send() -- returns 1 if the next packet in queue is ready to be sent (delay finished), 0 otherwise
 */
int ready_to_send(struct Packets *packets);

#endif