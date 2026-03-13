/*
 * delayed_packet.c -- structures and logic for delayed packets. intended to be used with the proxy server in week 3
 */

#include "./delayed_packet.h"

/*
 * enqueue_packet() -- enqueue a packet to the packets struct. requires the packet to be constructed beforehand
 */
void enqueue_packet(struct Packets *packets, struct Packet *packet){
    if (is_empty_packets(packets) == 1){
        packets->head = packets->tail = packet;
        packets->pkt_count++;
        return;
    }

    packets->tail->next = packet;
    packets->tail = packet;
    packets->pkt_count++;
}

/*
 * pop_packet() -- pop off and return a pointer to the next packet in queue (FIFO)
 */
struct Packet* pop_packet(struct Packets *packets){
    if (is_empty_packets(packets) == 1){
        return NULL;
    }

    struct Packet *target = packets->head;

    if (packets->pkt_count == 1){
        packets->head = packets->tail = NULL;
        packets->pkt_count = 0;
        return target;
    }

    packets->head = target->next;
    packets->pkt_count--;
    target->next = NULL;
    return target;
}

/*
 * peek_packet() -- return a const pointer to the next packet in queue
 */
const struct Packet* peek_packet(struct Packets *packets){
    return packets->head;
}

/*
 * is_empty_packets() -- returns 1 if packets struct is empty, 0 otherwise
 */
int is_empty_packets(struct Packets *packets){
    return (packets->pkt_count > 0) ? 0 : 1;
}

/*
 * ready_to_send() -- returns 1 if the next packet in queue is ready to be sent (delay finished), 0 otherwise
 */
int ready_to_send(struct Packets *packets){
    if (is_empty_packets(packets) == 0){
        return interval_elapsed_cur(packets->head->time_received, packets->delay_ms);
    }
    return 0;
}