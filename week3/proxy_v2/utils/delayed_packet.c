#include "./delayed_packet.h"

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

struct Packet* peek_packet(struct Packets *packets){
    return packets->head;
}

int is_empty_packets(struct Packets *packets){
    return (packets->pkt_count > 0) ? 0 : 1;
}

int ready_to_send(struct Packets *packets){
    if (is_empty_packets(packets) == 0){
        return interval_elapsed_cur(packets->head->time_received, packets->delay_ms);
    }
    return 0;
}