
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

long get_file_size(FILE *file);

void receive_file(char *fname, int sockfd);

int send_file(int sockfd, char *file_name);

#endif