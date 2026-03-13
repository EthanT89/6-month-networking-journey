/*
 * buffer_manipulation.c -- a test file for trying out various buffer (char[]) operations
 */

#include "./buffer_manipulation.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

void packi8 (unsigned char *buf, unsigned int i){
    *buf = i;
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
