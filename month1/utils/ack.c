/* 
 * ack.c -- a util file for managing ack packets
 */
#include "./ack.h"
#include "./buffer_manipulation.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int check_ack_membership(unsigned int acks[33], int packet_id){
    for (int i = 0; i < 33; i++){
        if (*(acks + i) == packet_id){
            printf("[ACK] Packet #%d ACK'd\n", packet_id);
            return 1;
        }
    }
    return 0;
}

/*
 * unpack_acks -- given a bitmap of "acks", migrate these bits into a list for easier handling.
 *
 * This makes it easier to later convert these bits to packet_ids that have been ack'd.
 * Used in conjunction with convert_bits_to_id
 */
void unpack_acks(unsigned char bitmap[4], unsigned int bits[32] ) {
    int array_size = 4; // Number of bytes in the array
    int total_bits = array_size * 8;

    for(int i = 0; i < total_bits; i++) {
        unsigned char mask = 1 << (7 - (i % 8)); // Create mask for the i-th bit (MSB to LSB)
        unsigned int bit_value = (bitmap[i / 8] & mask) ? 1 : 0; // Extract bit

        bits[i] = bit_value;
    }
}

/*
 * convert_bits_to_id -- given a list of bits (with the exception of the first element, which will
 * already be in int form), convert each bit to its integer form. By this, every element n steps after
 * the first (max) element represent the integer (max-n). If the bit is 1, assign it it's numeric value,
 * otherwise, leave as 0.
 * 
 * These integers represent the packet_id of pending packets that have been sent out. "1" indicates
 * that this packet was received and ack'd.
 */
void convert_bits_to_id(unsigned int bits[33]){
    unsigned int max_num = bits[0];

    for (int i = 1; i < 33; i++){
        if (bits[i] == 0){
            continue; // ack not received for this packet_id
        }
        bits[i] = max_num - i;
    }
}

void update_pending_pkts(struct pending_packet *packets, unsigned int acks[33])
{
    for (int i = 0; i < MAXPENDINGPACKETS; i++){
        struct pending_packet *pkt = packets + i;
        if (pkt->ack == 1){
            continue;
        }
        if (pkt->packet_id == 0){
            break;
        }
        int check = check_ack_membership(acks, (packets + i)->packet_id);
        if (check) {
            (packets + i)->ack = 1;
            continue;
        }
    }
}

void unpack_msg_metadata(unsigned char* buf, int *protocol_id, int *packet_id){
    *protocol_id = unpacki16(buf);
    *packet_id = unpacki16(buf+2);
}

void set_bit(unsigned char bitmap[4], unsigned int bit_index) {
    unsigned int byte_index = bit_index / 8;
    unsigned int bit_position = bit_index % 8;

    // MSB to LSB ordering (consistent with unpack_acks)
    unsigned char mask = 1 << (7 - bit_position);
    bitmap[byte_index] |= mask;
}  

void shift_right_by_byte(unsigned char bitmap[4]){
    bitmap[3] = bitmap[2];
    bitmap[2] = bitmap[1];
    bitmap[1] = bitmap[0];
    bitmap[0] = 0;
}

/*
 * update_outgoing_acks -- updates the state of outgoing acks. The newest ack packet number
 * replaces the old, and the entire ack bitmap shifts over (new-old) positions. Each bit represents 
 * base_ack - n packet id. So, the bitmap needs to update corresponding to the new max (new ack). If the new
 * ack is 5 greater than before, then everything needs to shift by 5, and the position indicative of the old number
 * (in this case, 5 less than the new number, so 5 bits over) is assigned 1.
 */
void update_outgoing_acks(unsigned int *old, unsigned int new, unsigned char acks[4]){
    if (new < *old){
        memset(acks, 0, 4);
        *old = new;
        return;
    }
    
    unsigned int diff = new - *old;

    if (diff == 0){
        return;
    }
    
    if(diff >= 32){
        memset(acks, 0, 4);
    } else {
        // Shift each byte right by diff positions
        // Process from right to left, capturing carry bits before modifying
        int bytes_shifted = diff / 8;
        int bits_remain = diff % 8;

        for (int i = 0; i < bytes_shifted; i++){
            shift_right_by_byte(acks);
        }

        for (int i = 3; i >= 0; i--) {
            unsigned char new_val = acks[i] >> bits_remain;
            if (i > 0) {
                // Take the rightmost 'diff' bits from the byte to the left (before it's modified)
                unsigned char carry = acks[i-1] & ((1 << bits_remain) - 1);
                // Shift them to the leftmost positions of this byte
                new_val |= (carry << (8 - bits_remain));
            }
            acks[i] = new_val;
        }
    }

    // Set the bit at position (diff-1) to mark the newly ack'd packet
    set_bit(acks, diff - 1);
    *old = new; 
}

void construct_packet(unsigned int packet_id, unsigned char input[MAXBUFSIZE-4], unsigned char packet[MAXBUFSIZE]){
    packi16(packet, APPID);
    packi16(packet+2, packet_id);
    append_buf_after_sequence(packet, input);
}

void construct_ack_packet(unsigned int ack, unsigned char ack_bitmap[4], unsigned char buf[12]){
    packi16(buf, APPID);
    packi32(buf+2, ack);
    memcpy(buf+6, ack_bitmap, 4);
}

void deconstruct_ack_packet(unsigned int *protocol_id, unsigned int *ack, unsigned char ack_bitmap[4], unsigned char buf[12]){
    *protocol_id = unpacki16(buf);
    *ack = unpacki32(buf+2);
    memcpy(ack_bitmap, buf+6, 4);
}

void add_pending_packet(unsigned int packet_id,
                        struct pending_packet packets[MAXPENDINGPACKETS],
                        unsigned int *insert_at,
                        const unsigned char msg[MAXBUFSIZE-4])
{
    if (insert_at == NULL) {
        return;
    }

    if (*insert_at >= MAXPENDINGPACKETS){
        *insert_at = 0;
    }

    struct pending_packet new_packet;
    memset(&new_packet, 0, sizeof new_packet);
    new_packet.packet_id = packet_id;
    new_packet.ack = 0;

    new_packet.time_sent = get_seconds();

    size_t msg_len = strnlen((const char *)msg, MAXBUFSIZE - 4);
    memcpy(new_packet.msg, msg, msg_len);
    if (msg_len < MAXBUFSIZE) {
        new_packet.msg[msg_len] = '\0';
    } else {
        new_packet.msg[MAXBUFSIZE - 1] = '\0';
    }

    packets[*insert_at] = new_packet;
    (*insert_at)++;
}

double get_seconds(){
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    double s = (double)time.tv_sec;
    double ns = (double)time.tv_nsec;
    double ms = truncf(ns / pow(10, 6));

    s += (ms / pow(10, 3));
    return s;
}