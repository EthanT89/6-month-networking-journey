#include "./epoll_helper.h"

/*
 * create_epoll() -- create an epoll instance and return it's file descriptor
 */
int create_epoll(){
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    return epoll_fd;
}

/*
 * add_epoll_fd() -- register a file descriptor with epoll for read event monitoring
 */
void add_epoll_fd(int epoll_fd, int new_fd){
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = new_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_fd, &event) == -1) {
        perror("epoll_ctl");
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }
}