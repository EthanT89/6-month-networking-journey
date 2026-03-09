#ifndef EPOLL_HELPER_H
#define EPOLL_HELPER_H

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

/*
 * create_epoll() -- create an epoll instance and return it's file descriptor
 */
int create_epoll();

/*
 * add_epoll_fd() -- register a file descriptor with epoll for read event monitoring
 */
void add_epoll_fd(int epoll_fd, int new_fd);

#endif