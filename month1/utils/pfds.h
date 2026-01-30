#ifndef _PFDS_H
#define _PFDS_H

#include <poll.h>
#include <stdlib.h>

void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size);
void remove_from_pfds(struct pollfd **pfds, int i, int *fd_count);

#endif