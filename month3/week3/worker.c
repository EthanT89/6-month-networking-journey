/*
 * worker.c -- Worker process for receiving and executing file-based jobs from server
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

    char dir[MAXFILEPATH];
    char ext[MAXFILEEXT];

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

    char dir_path[MAXFILEPATH];
    char clear_cmd[MAXFILEPATH];
    sprintf(dir_path, "./worker_storage/worker-%d", id);
    sprintf(clear_cmd, "cd ./worker_storage/worker-%d/ && rm -rf *", id);

    if (system(clear_cmd) == 0) {
        printf("All files deleted successfully.\n");
    } else {
        printf("Failed to delete files.\n");
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
void handle_job_success(struct Self *self){
    printf("job complete.\n");
    self->errcode = 1;
    self->status = W_SUCCESS;
    unsigned char update[MAXBUFSIZE];
    char file_path[MAXFILEPATH+15];
    int offset = 0;

    packi16(update+offset, APPID); offset += 2;
    packi16(update+offset, WPACKET_STATUS); offset += 2;
    packi16(update+offset, self->status); offset += 2;
    packi16(update+offset, self->errcode); offset += 2;
    send(self->servfd, update, offset, 0);

    sprintf(file_path, "%sresults%s", self->dir, self->ext);

    if (strcmp(self->ext, ".txt") == 0) send_file_text_based(self->servfd, file_path);
    else if (strcmp(self->ext, ".jpg") == 0) send_file_img_based(self->servfd, file_path);
}

/*
 * handle_job_assignment() -- process incoming job, execute it, and report results
 *
 * Sets status to W_BUSY, processes the job content, reports success/failure to server
 */
void handle_job_assignment(struct Self *self){
    self->status = W_BUSY;
    // sleep(5);

    unsigned char buf[MAXBUFSIZE];
    memset(buf, 0, MAXBUFSIZE);
    if (read(self->servfd, buf, 2) <= 0) return;

    int spec_size = unpacki16(buf);
    memset(buf, 0, MAXBUFSIZE);
    if (read(self->servfd, buf, spec_size) <= 0) return;

    unsigned char file_type[MAXBUFSIZE];
    if (read(self->servfd, file_type, 2) <= 0) return;
    int file_type_id = unpacki16(file_type);

    char fname[MAXFILEPATH];
    strcpy(fname, self->dir);

    if (file_type_id == TXT_FILE){
        strcpy(self->ext, ".txt");
        strcat(fname, "content.txt");
        fclose(fopen(fname, "w"));
        receive_file_text_based(fname, self->servfd);
    } else if (file_type_id == IMG_FILE){
        strcpy(self->ext, ".jpg");
        strcat(fname, "content.jpg");
        fclose(fopen(fname, "wb"));
        receive_file_img_based(self->servfd, fname);
    }
    

    int rv = process_job(buf, self->dir, self->ext);
    if (rv <= -1){
        printf("errcode %d\n", rv);
        self->errcode = rv;
        handle_job_failure(self);
        self->errcode = 1;
        self->status = W_READY;
        return;
    }

    self->jobs_completed++;
    handle_job_success(self);
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
        (void)unpacki16(buf+offset); offset += 2;
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

    sprintf(self->dir, "./worker_storage/worker-%d/", self->id);
    (void)mkdir(self->dir, 0755);

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
            //printf(".");
            //fflush(stdout);
        }
    }

    close(sockfd);
}