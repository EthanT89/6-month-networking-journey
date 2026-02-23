/*
 * server.c -- Main processing server for robust task queue implementation (may be split across servers in time)
 */

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

// Custom imports
#include "./utils/jobs.h"
#include "./common.h"

/*
 * Server -- custom struct containing tasks, workers, sockets, and other real-time data
 */
struct Server {
    int worker_listener; // socket listening for worker connections
    int client_listener; // socket listening for client connections
};

/*
 * get_listening_socket -- create and bind a listening socket, return the file descriptor
 */
int get_listening_socket(unsigned char *port){
    int sockfd, rv;
    int yes = 1;
    struct addrinfo *res, *p, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

        // first, use getaddrinfo() to find some addresses!
    if ((rv = getaddrinfo(NULL, port, &hints, &res)) != 0){
        perror("server: getaddrinfo\n");
        exit(1);
    }

    for (p = res; p != NULL; p = p->ai_next){
        // first, try to make a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket\n");
            continue;
        }

        // no more "address already in use" error :)
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("server: bind\n");
            continue;
        }

        printf("Successfully created socket!\n");
        break;
    }

    if (p == NULL){
        perror("server: start\n");
        exit(1);
    }

    if (listen(sockfd, MAXBACKLOG) == -1){
        perror("server: listen\n");
        exit(1);
    }

    return sockfd;
}

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

int main(){
    int client_fd = get_listening_socket(CLIENT_PORT);
    int worker_fd = get_listening_socket(WORKER_PORT);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    add_epoll_fd(epoll_fd, 0);
    add_epoll_fd(epoll_fd, client_fd);
    add_epoll_fd(epoll_fd, worker_fd);

    // Event loop
    struct epoll_event events[MAXEPOLLEVENTS];
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAXEPOLLEVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                char buffer[100];
                ssize_t bytes_read = read(events[i].data.fd, buffer, 100 - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("Received: %s", buffer);
                }
            }
        }
    }

    close(epoll_fd);
    close(client_fd);
    close(worker_fd);

    return EXIT_SUCCESS;
}