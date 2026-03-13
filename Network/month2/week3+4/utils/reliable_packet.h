/*
 * reliable_packet.h -- framework and logic for implementing reliable packets in the UDP mutliplayer treasure game
 */

#ifndef RELIABLE_PACKET_H
#define RELIABLE_PACKET_H

#include "../common.h"
#include "./time_custom.h"

#include <stddef.h>

/*
 * ReliablePacket -- struct for containing data relevant to a reliable packet implemented with the UDP protocol
 *
 * data -- original packet data (includes seq num)
 * data_len -- size, in bytes, of the data
 * 
 * time_sent -- time, in ms, the packet was sent
 * retry_ct -- number of times re-sent
 * seq_num -- ordered id of the packet, relative to the server it was sent from
 * client_id -- used by server to track which client this was sent to
 * 
 * *next -- pointer to next node in the linked list
 */
struct ReliablePacket {
    unsigned char data[MAXBUFSIZE];
    size_t data_len;

    int time_sent;
    int retry_ct;
    int seq_num;
    int client_id;

    struct ReliablePacket *next;
};

/*
 * ReliablePacketSLL -- struct for containing a linked list of ReliablePackets
 *
 * *head -- pointer to the first packet in the list
 * *tail -- pointer to the last packet in the list
 * count -- total number of packets in the list
 */
struct ReliablePacketSLL {
    struct ReliablePacket *head;
    struct ReliablePacket *tail;
    size_t count;
};

/*
 * add_reliable_packet() -- add a reliable packet to the linked list
 */
void add_reliable_packet(struct ReliablePacketSLL *packets, struct ReliablePacket *packet);

/*
 * remove_reliable_packet() -- remove a reliable packet from the linked list given it's sequence number
 */
struct ReliablePacket* remove_reliable_packet(struct ReliablePacketSLL *packets, int seq_num);

/*
 * find_rp_by_seq_num() -- given a sequence number, return the corresponding packet if any. NULL otherwise
 */
struct ReliablePacket* find_rp_by_seq_num(struct ReliablePacketSLL *packets, int seq_num);

/*
 * check_for_timeout() -- check the linked list for the first packet that was sent longer than `timeout` milliseconds ago
 */
struct ReliablePacket* check_for_timeout(struct ReliablePacketSLL *packets, int timeout);

#endif