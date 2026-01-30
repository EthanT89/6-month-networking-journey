#ifndef _ACK_H
#define _ACK_H

#include <time.h>
#include "./buffer_manipulation.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MAXBUFSIZE 100
#define MAXPENDINGPACKETS 100
#define APPID 1122

struct pending_packet{
    unsigned int packet_id;
    unsigned int ack;
    float time_sent;
    unsigned char msg[MAXBUFSIZE-4];
};

/*
 * check_ack_membership -- checks if a packet_id is present in the acks array
 * returns 1 if found, 0 otherwise
 */
int check_ack_membership(unsigned int acks[33], int packet_id);

/*
 * unpack_acks -- given a bitmap of "acks", migrate these bits into a list for easier handling.
 * Makes it easier to later convert these bits to packet_ids that have been ack'd.
 * Used in conjunction with convert_bits_to_id
 */
void unpack_acks(unsigned char bitmap[4], unsigned int bits[32]);

void resend_dropped_pkt(struct pending_packet *pkt);

/*
 * convert_bits_to_id -- converts a bit array to packet IDs
 * Given a list of bits (with the exception of the first element, which is the max ack),
 * convert each bit to its integer form. Position n represents the integer (max-n).
 * If the bit is 1, assign its numeric value, otherwise leave as 0.
 */
void convert_bits_to_id(unsigned int bits[33]);

/*
 * update_pending_pkts -- updates the ack status of pending packets
 * Checks if any unack'd packets are in the incoming acks array and marks them as ack'd
 */
void update_pending_pkts(struct pending_packet *packets, unsigned int acks[33]);

/*
 * unpack_msg_metadata -- unpacks protocol id and packet id from message buffer
 */
void unpack_msg_metadata(unsigned char* buf, int *protocol_id, int *packet_id);

/*
 * set_bit -- sets a single bit in the bitmap at the specified index
 */
void set_bit(unsigned char bitmap[4], unsigned int bit_index);

/*
 * shift_right_by_byte -- shifts the entire 4-byte bitmap right by one byte
 * Byte 2 → Byte 3, Byte 1 → Byte 2, Byte 0 → Byte 1, Byte 0 = 0
 */
void shift_right_by_byte(unsigned char bitmap[4]);

/*
 * update_outgoing_acks -- updates the state of outgoing acks
 * Shifts the ack bitmap to make room for new acks and sets the bit for the newly ack'd packet
 * Handles shifts of any size (0-32+ bits)
 */
void update_outgoing_acks(unsigned int *old, unsigned int new, unsigned char acks[4]);

void construct_packet(unsigned int packet_id, unsigned char input[MAXBUFSIZE-4], unsigned char packet[MAXBUFSIZE]);

/*
 * construct_ack_packet -- constructs an ACK packet in the provided buffer
 * Packs APPID, ack value, and ack bitmap into a 12-byte buffer
 */
void construct_ack_packet(unsigned int ack, unsigned char ack_bitmap[4], unsigned char buf[12]);

/*
 * deconstruct_ack_packet -- deconstructs an ACK packet into the provided ack, protocol_id, and bitmap.
 */
void deconstruct_ack_packet(unsigned int *protocol_id, unsigned int *ack, unsigned char ack_bitmap[4], unsigned char buf[12]);

void add_pending_packet(unsigned int packet_id, struct pending_packet packets[MAXPENDINGPACKETS], unsigned int *insert_at, const unsigned char msg[MAXBUFSIZE-4]);

double get_seconds();

#endif
