#include "./pfds.h"

void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size)
{
    if (*fd_count == *fd_size){
        *fd_size *= 2;
        *pfds = realloc(*pfds, sizeof **pfds * *fd_size);
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

void remove_from_pfds(struct pollfd **pfds, int i, int *fd_count)
{
    (*pfds)[i] = (*pfds)[*fd_count - 1];
    (*fd_count)--;
}
