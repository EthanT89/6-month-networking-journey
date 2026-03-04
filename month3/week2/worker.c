/*
 * worker.c -- worker process for receiving and executing jobs from the server
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
#include <sys/stat.h>

// custom imports
#include "./common.h"
#include "./utils/buffer_manipulation.h"
#include "./utils/job_processing.h"
#include "./utils/file_transfer.h"
#include "./utils/epoll_helper.h"

/*
 * Self -- worker state struct tracking current status and server connection
 *
 * jobs_completed -- total jobs successfully processed
 * id -- unique worker ID assigned by server
 * status -- current worker status (W_READY, W_BUSY, W_FAILURE, W_SUCCESS)
 * errcode -- error code for failed jobs
 * servfd -- socket file descriptor for server connection
 */
struct Self {
    int jobs_completed;
    int id;
    int status;
    int errcode;

    int servfd;
};

/*
 * get_socket() -- create and return a TCP connection to the server's worker port
 */
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

void reset_storage(int id){

    char filename[MAXFILEPATH];
    char dir_path[MAXFILEPATH];
    
    sprintf(filename, "./worker_storage/worker-%d/temp.txt", id);
    sprintf(dir_path, "./worker_storage/worker-%d", id);

    // Step 1: Delete the file inside the directory
    if (remove((const char*)filename) == 0) {
        printf("File deleted successfully.\n");
    } else {
        perror("Error deleting file");
    }

    // Step 2: Delete the now-empty directory
    if (remove((const char*)dir_path) == 0) {
        printf("Directory deleted successfully.\n");
    } else {
        perror("Error deleting directory");
    }
}

/*
 * handle_job_failure() -- notify server of job failure and send error code
 */
void handle_job_failure(struct Self *self){
    printf("job failed.\n");
    self->status = W_FAILURE;
    unsigned char update[MAXBUFSIZE];
    int offset = 0;

    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, WPACKET_STATUS); offset += 2;
    packi16(update+offset, self->status); offset += 2;
    packi16(update+offset, self->errcode); offset += 2;

    send(self->servfd, update, offset, 0);
}

/*
 * handle_job_success() -- notify server of job completion and send results
 */
void handle_job_success(struct Self *self, unsigned char buf[MAXRESULTSIZE]){
    printf("job complete.\n");
    self->errcode = 1;
    self->status = W_SUCCESS;
    unsigned char update[MAXBUFSIZE];
    int offset = 0;

    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, WPACKET_RESULTS); offset += 2;
    strncpy(update+offset, buf, MAXBUFSIZE-offset); offset += strlen(buf);

    send(self->servfd, update, offset, 0);
}

/*
 * handle_job_assignment() -- process incoming job, execute it, and report results
 *
 * Sets status to W_BUSY, processes the job content, reports success/failure to server
 */
void handle_job_assignment(struct Self *self){
    self->status = W_BUSY;
    unsigned char result[MAXRESULTSIZE];
    sleep(5);

    unsigned char buf[MAXBUFSIZE];
    memset(buf, 0, MAXBUFSIZE);
    read(self->servfd, buf, 2);

    int spec_size = unpacki16(buf);
    memset(buf, 0, MAXBUFSIZE);
    read(self->servfd, buf, spec_size);
    printf("spec size: %d\n", spec_size);
    printf("buf: %s\n", buf);

    char fname[MAXFILEPATH];
    sprintf(fname, "./worker_storage/worker-%d/temp.txt", self->id);
    printf("yo\n");
    receive_file(fname, self->servfd);

    int rv = process_job(result, buf);
    if (rv <= -1){
        printf("errcode %d\n", rv);
        self->errcode = rv;
        handle_job_failure(self);
        self->errcode = 1;
        self->status = W_READY;
        return;
    }

    self->jobs_completed++;
    handle_job_success(self, result);
    self->status = W_READY;
}

/*
 * handle_status_update() -- send current worker status to server
 */
void handle_status_update(struct Self *self){
    unsigned char update[MAXBUFSIZE];
    int offset = 0;

    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, WPACKET_STATUS); offset += 2;
    packi16(update+offset, self->status); offset += 2;
    packi16(update+offset, self->errcode); offset += 2;

    send(self->servfd, update, offset, 0);
}

/*
 * handle_server_data() -- parse and handle incoming packets from server
 *
 * Handles WPACKET_NEWJOB (job assignment) and WPACKET_STATUS (status request) packets
 */
void handle_server_data(struct Self *self, unsigned char buf[MAXBUFSIZE]){
    int offset = 0;
    int appid = unpacki16(buf+offset); offset += 2;
    int packetid = unpacki16(buf+offset); offset += 2;

    if (appid != APPID){
        return;
    }

    if (packetid == WPACKET_NEWJOB){
        printf("received new job. processing...\n");
        int jobid = unpacki16(buf+offset); offset += 2;
        handle_job_assignment(self);
    }

    if (packetid == WPACKET_STATUS){
        handle_status_update(self);
    }
}

void handle_shutdown(int serverfd, int epollfd, int id){
    printf("shutting down...\n");
    close(serverfd);
    close(epollfd);

    reset_storage(id);
    printf("goodbye.\n");
    exit(EXIT_SUCCESS);
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
    self->errcode = 1;

    unsigned char buf[MAXBUFSIZE];
    recv(sockfd, buf, MAXBUFSIZE, 0);
    self->id = unpacki16(buf+4);
    printf("ID: %d\nwaiting for jobs...", self->id);
    fflush(stdout);

    char dir_name[MAXFILEPATH];
    sprintf(dir_name, "./worker_storage/worker-%d", self->id);
    int status = mkdir(dir_name, 0755);

    if (status != 0){
        //close(sockfd);
        //exit(1);
    }

    struct epoll_event events[MAXEPOLLEVENTS];
    while (1) {

        int nfds = epoll_wait(epollfd, events, MAXEPOLLEVENTS, 2000);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                printf("\n\n");
                int fd = events[i].data.fd;
                if (fd != self->servfd){
                    break;
                }

                unsigned char buffer[MAXBUFSIZE];
                memset(buffer, 0, sizeof buffer);

                int rv = read(fd, buffer, 6);
                if (rv == 0) {
                    printf("server disconnected.\n");
                    handle_shutdown(sockfd, epollfd, self->id);
                }
                handle_server_data(self, buffer);
                printf("\n");
            }
        }

        if (self->status == W_READY){
            printf(".");
            fflush(stdout);
        }
    }

    close(sockfd);
}