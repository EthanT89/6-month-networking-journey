/*
 * client.c -- client request management and facilitation
 */

// Main imports
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

// custom imports
#include "./common.h"
#include "./utils/buffer_manipulation.h"
#include "./utils/job_processing.h"

struct Self {
    int jobs_completed;
    int id;
    int status;

    int servfd;
};

int get_socket(){
    int sockfd, rv;
    int yes = 1;
    struct addrinfo *res, *p, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

        // first, use getaddrinfo() to find some addresses!
    if ((rv = getaddrinfo(NULL, WORKER_PORT, &hints, &res)) != 0){
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

        printf("Successfully connected to server!\n\n");
        break;
    }

    if (p == NULL){
        perror("server: start\n");
        exit(EXIT_FAILURE);
    }

    if ((rv = connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1){
        perror("client: connect");
        exit(EXIT_FAILURE);
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

void handle_job_failure(struct Self *self){
    self->status = W_FAILURE;
    unsigned char update[MAXBUFSIZE];
    int offset = 0;

    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, WPACKET_STATUS); offset += 2;
    packi16(update+offset, self->status); offset += 2;

    send(self->servfd, update, offset, 0);
}

void handle_job_assignment(struct Self *self, unsigned char buf[MAXBUFSIZE]){
    self->status = W_BUSY;

    int job_type = determine_job_type(buf, strlen(buf));
    if (job_type == -1){
        printf("unknown job type\n");
        handle_job_failure(self);
        self->status = W_READY;
        return;
    }
    printf("job type: %d\n", job_type);

}

void handle_status_update(struct Self *self){
    unsigned char update[MAXBUFSIZE];
    int offset = 0;

    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, WPACKET_STATUS); offset += 2;
    packi16(update+offset, self->status); offset += 2;

    send(self->servfd, update, offset, 0);
}

void handle_server_data(struct Self *self, unsigned char buf[MAXBUFSIZE]){
    int offset = 0;
    int appid = unpacki16(buf+offset); offset += 2;
    int packetid = unpacki16(buf+offset); offset += 2;

    if (appid != APPID){
        return;
    }

    if (packetid == WPACKET_NEWJOB){
        printf("received new job.\n");
        int jobid = unpacki16(buf+offset); offset += 2;
        printf("job metadata: %s - len %ld\n", buf+offset, strlen(buf+offset));
        handle_job_assignment(self, buf+offset);
    }

    if (packetid == WPACKET_STATUS){
        handle_status_update(self);
    }
}

int main(){

    printf("\nConnecting to server...\n");
    int sockfd = get_socket();
    int epollfd = create_epoll();
    add_epoll_fd(epollfd, sockfd);
    

    struct Self *self = malloc(sizeof *self);
    self->jobs_completed = 0;
    self->status = W_READY;
    self->id = -1;
    self->servfd = sockfd;

    unsigned char buf[MAXBUFSIZE];
    recv(sockfd, buf, MAXBUFSIZE, 0);
    self->id = unpacki16(buf+4);
    printf("ID: %d\n", self->id);

    struct epoll_event events[MAXEPOLLEVENTS];
    while (1) {

        int nfds = epoll_wait(epollfd, events, MAXEPOLLEVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                int fd = events[i].data.fd;
                if (fd != self->servfd){
                    break;
                }

                unsigned char buffer[MAXBUFSIZE];
                memset(buffer, 0, sizeof *buffer);

                read(fd, buffer, MAXBUFSIZE);
                handle_server_data(self, buffer);
            }
        }

        if (self->status == W_READY){
            sleep(1);
            printf(".");
            fflush(stdout);
        }
    }

    close(sockfd);
}