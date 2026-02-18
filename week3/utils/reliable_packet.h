/*
 * reliable_packet.h -- framework and logic for implementing reliable packets in the UDP mutliplayer treasure game
 */

#ifndef RELIABLE_PACKET_H
#define RELIABLE_PACKET_H

#include "../common.h"
#include "./time_custom.h"

#include <stddef.h>

struct ReliablePacket {
    unsigned char data[MAXBUFSIZE];
    size_t data_len;

    int time_sent;
    int retry_ct;
    int seq_num;

    struct ReliablePacket *next;
};

struct ReliablePacketSLL {
    struct ReliablePacket *head;
    struct ReliablePacket *tail;
    size_t count;
};

void add_reliable_packet(struct ReliablePacketSLL *packets, struct ReliablePacket *packet);

struct ReliablePacket* remove_reliable_packet(struct ReliablePacketSLL *packets, int seq_num);

struct ReliablePacket* find_rp_by_seq_num(struct ReliablePacketSLL *packets, int seq_num);

struct ReliablePacket* check_for_timeout(struct ReliablePacketSLL *packets, int timeout);


#endif