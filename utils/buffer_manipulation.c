/*
 * buffer_manipulation.c -- a test file for trying out various buffer (char[]) operations
 */

#include "./buffer_manipulation.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

void append_buf_after_sequence (unsigned char *buf, unsigned char *newbuf){
    size_t buf_size = 4;

    for (int i = 0; i < strlen(newbuf); i++){
        // Sequence number is 2 bytes long, so append 2 positions after the starting spot
        // Would like to find a cleaner way rather than hardcoding 2, but strlen() does not
        // work when raw bit data is contained in the buffer :(. Would like to come back to 
        // this someday.
        //
        // It also doesn't allow for multiple append operations, which is fine for my current
        // use case.
        buf[i+buf_size] = newbuf[i];
    }
}

void packi16 (unsigned char *buf, unsigned int i){
    *buf++ = i>>8; *buf++ = i;
}

void packi32 (unsigned char *buf, unsigned long int i){
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8; *buf++ = i;
}

void packi64 (unsigned char *buf, unsigned long long int i){
    *buf++ = i>>56; *buf++ = i>>48;
    *buf++ = i>>40; *buf++ = i>>32;
    *buf++ = i>>24; *buf++ = i>>16;
    *buf++ = i>>8; *buf++ = i;
}

int unpacki16(unsigned char *buf){
    unsigned int i2 = ((unsigned int)buf[0]<<8) | buf[1];
    int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffu) {i = i2; }
    else {i = -1 - (unsigned int)(0xffffu - i2); }

    return i;
}

int unpacki32(unsigned char *buf){
    unsigned int i2 = ((unsigned int)buf[0]<<24) | ((unsigned int)buf[1]<<16) | ((unsigned int)buf[2]<<8) | buf[3];
    int i;

    //change unsigned numbers to signed
    if (i2 <= 0x7fffffffu) {i = i2; }
    else {i = -1 - (0x7fffffff - i2);}

    return i;
}

void prepend_i16(unsigned char buf[MAXBUFSIZE], int i16){
    // Shift existing content 2 bytes to the right
    memmove(buf + 2, buf, MAXBUFSIZE - 2);
    
    // Pack the new i16 at the beginning
    packi16(buf, i16);
}

/*
 * Currently giving lots of warnings in compiler, annoying so commenting out since it isn't used.
int unpacki64(unsigned char *buf){
    if (sizeof buf >= 64) {
        int i2 = ((unsigned int)buf[0]<<56) | ((unsigned int)buf[1]<<48) |
            ((unsigned int)buf[2]<<40) | ((unsigned int)buf[3]<<32) |
            ((unsigned int)buf[4]<<24) | ((unsigned int)buf[5]<<16) |
            ((unsigned int)buf[6]<<8) | buf[7];
        int i;

        //change signed integers to signed
        if (i2 < 0x7fffffffffffffff) {i = i2; }
        else {i = -1 - (0x7fffffffffffffff - i2); }

        return i;
    }
    return -1;
}
*/

/*
int main (void){
    unsigned char buf[32];

    signed long long int num64 = -123456789;
    unsigned long int num32 = 532;
    signed int num16 = -1233;

    packi64(buf, num64);
    packi32(buf+8, num32);
    packi16(buf+12, num16);

    printf("buf: \n long long int - %02X %02X %02X %02X %02X %02X %02X %02X\n long int - %02X %02X %02X %02X\n int - %02X %02X\n\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12], buf[13]);
    
    printf("num64 before packing: %d\n", num64 );
    printf("num32 before packing: %d\n", num32 );
    printf("num16 before packing: %d\n\n", num16 );

    unsigned long long int num64_out = unpacki64(buf);
    unsigned long int num32_out = unpacki32(buf+8);
    unsigned long int num16_out = unpacki16(buf+12);

    printf("unpacked 64bit: %d\n", num64_out);
    printf("unpacked 32bit: %d\n", num32_out);
    printf("unpacked 16bit: %d\n", num16_out);

    unsigned char newbuf[100];
    unsigned char oldbuf[110];

    fgets(newbuf, 100, stdin);

    packi16(oldbuf, num16);

    printf("buf before appended: %s\n", oldbuf);
    append_buf(oldbuf, newbuf);

    num16 = unpacki16(oldbuf);

    printf("buf after appended: %s\nnum extracted: %d\n", oldbuf + 2, num16);

}
*/
