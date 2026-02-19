#include "./reliable_packet.h"

void add_reliable_packet(struct ReliablePacketSLL *packets, struct ReliablePacket *packet){
    if (packets->count == 0){
        packets->head = packets->tail = packet;
        ++packets->count;
        return;
    }

    packets->tail->next = packet;
    packets->tail = packet;
    ++packets->count;
}

struct ReliablePacket* remove_reliable_packet(struct ReliablePacketSLL *packets, int seq_num){
    if (packets->count == 0){
        return NULL;
    }

    struct ReliablePacket *res = packets->head;
    struct ReliablePacket *prev = NULL;

    for (res; res != NULL; res = res->next){
        if (res->seq_num == seq_num){
            if (packets->head == packets->tail){
                packets->head = packets->tail = NULL;
                --packets->count;
                return res;
            }

            if (packets->head == res){
                packets->head = res->next;
                res->next = NULL;
                --packets->count;
                return res;
            }

            if (packets->tail == res){
                packets->tail = prev;
                prev->next = NULL;
                res->next = NULL;
                --packets->count;
                return res;
            }

            prev->next = res->next;
            res->next = NULL;
            --packets->count;
            return res;
        }
        prev = res;
    }

    return NULL;

}

struct ReliablePacket* find_rp_by_seq_num(struct ReliablePacketSLL *packets, int seq_num){
    if (packets->count == 0){
        return NULL;
    }

    struct ReliablePacket *cur = packets->head;

    for (cur; cur != NULL; cur = cur->next){
        if (cur->seq_num == seq_num){
            return cur;
        }
    }

    return NULL;
}

struct ReliablePacket* check_for_timeout(struct ReliablePacketSLL *packets, int timeout){
    if (packets->count == 0){
        return NULL;
    }

    struct ReliablePacket *cur = packets->head;

    for (cur; cur != NULL; cur = cur->next){
        if (interval_elapsed_cur(cur->time_sent, timeout) == 1){
            return cur;
        }
    }
    return NULL;
}