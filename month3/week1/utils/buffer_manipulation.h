#ifndef _BUFFER_MANIPULATION_H
#define _BUFFER_MANIPULATION_H

#include "../common.h"

void append_buf_after_sequence (unsigned char *buf, unsigned char *newbuf);

void packi16 (unsigned char *buf, unsigned int i);

void packi32 (unsigned char *buf, unsigned long int i);

void packi64 (unsigned char *buf, unsigned long long int i);

int unpacki16(unsigned char *buf);

int unpacki32(unsigned char *buf);

int unpacki64(unsigned char *buf);

void prepend_i16(unsigned char buf[MAXBUFSIZE], int i16);

#endif