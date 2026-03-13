
#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

// Main imports
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "../common.h"
#include "./epoll_helper.h"
#include "./buffer_manipulation.h"

long get_file_size(FILE *file);

void get_file_extension(char *fname, char *ext);

int receive_file_text_based(char *fname, int sockfd);

int send_file_text_based(int sockfd, char *file_name);

int receive_file_img_based(int sockfd, char *fname);

int send_file_img_based(int sockfd, char *fname);

#endif