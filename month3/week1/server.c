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
#include "./utils/buffer_manipulation.h"
#include "./utils/time_custom.h"
#include "./utils/workers.h"
#include "./utils/job_queue.h"
#include "./common.h"

struct Stats {
    int jobs_processed;
    int jobs_failed;
    int jobs_succeeded;
    int success_rate;
    int workers_ct;
    int jobs_in_queue;
};

/*
 * Server -- custom struct containing tasks, workers, sockets, and other real-time data
 */
struct Server {
    int epoll_fd;
    int worker_listener; // socket listening for worker connections
    int client_listener; // socket listening for client connections

    int job_id_ct;

    struct Stats *stats;
    struct JobQueue *queue;
    struct Jobs *jobs;
    struct Workers *workers;
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

int assign_to_worker(struct Server *server, unsigned char metadata[MAXJOBMETADATASIZE], int job_id){
    struct Worker *worker = get_available_worker(server->workers);

    if (worker == NULL){
        return -1;
    }

    unsigned char job_packet[MAXBUFSIZE];
    int offset = 0;
    packi16(job_packet+offset, APPID); offset += 2;
    packi16(job_packet+offset, WPACKET_NEWJOB); offset += 2;
    packi16(job_packet+offset, job_id); offset += 2;
    memcpy(job_packet+offset, metadata, MAXJOBMETADATASIZE); offset += strlen(metadata);

    printf("assigning job %d to worker %d\n\n", job_id, worker->id);
    send(worker->id, job_packet, offset, 0);
    worker->cur_job_id = job_id;
    worker->status = W_BUSY;
    return worker->id;
}

void get_status_msg(unsigned char msg[MAXBUFSIZE], struct Server *server, int job_id){
    struct Job *job = get_job_by_id(server->jobs, job_id);

    int status_id = -1;
    if (job != NULL){
        status_id = job->status;
    }

    if (status_id == J_IN_QUEUE){
        sprintf(msg, "Job in queue.");
        return;
    }
    if (status_id == J_IN_PROGRESS){
        sprintf(msg, "Job in progress. Worker: %d", job->worker_id);
        return;
    }
    if (status_id == J_SUCCESS){
        sprintf(msg, "Job complete. see 'results' for more info.");
        return;
    }
    if (status_id == J_FAILURE){
        sprintf(msg, "Job failed.");
        return;
    }

    if (status_id == -1){
        sprintf(msg, "Job not found.");
        return;
    }
}

void handle_job_submission(struct Server *server, unsigned char metadata[MAXJOBMETADATASIZE], unsigned char return_msg[MAXBUFSIZE]){
    struct Job *job = create_blank_job();
    job->job_id = server->job_id_ct++;
    job->job_type = 0; // TODO: determine job types
    strncpy(job->results, metadata, MAXRESULTSIZE);

    add_job(server->jobs, job);
    add_to_queue(server->queue, job->job_id);
    server->stats->jobs_in_queue++;
    sprintf(return_msg, "Job ID: %d\n", job->job_id);
}

void check_queue(struct Server *server){
    struct Worker *worker = get_available_worker(server->workers);
    if (worker == NULL) return;

    int job_id = pop_queue(server->queue);
    if (job_id == -1) return;
    server->stats->jobs_in_queue--;

    struct Job *job = get_job_by_id(server->jobs, job_id);
    if (job == NULL) return;
    job->time_start = get_time_ms();

    int rv = assign_to_worker(server, job->results, job_id);
    job->worker_id = rv;
    job->status = J_IN_PROGRESS;
}

void handle_job_status(struct Server *server, unsigned char metadata[MAXJOBMETADATASIZE], unsigned char return_msg[MAXBUFSIZE]){
    int job_id = atoi(metadata);
    get_status_msg(return_msg, server, job_id);
}

void handle_job_get_results(struct Server *server, unsigned char metadata[MAXJOBMETADATASIZE], unsigned char return_msg[MAXBUFSIZE]){
    int job_id = atoi(metadata);
    int status = get_job_status(server->jobs, job_id);
    struct Job *job = get_job_by_id(server->jobs, job_id);
    if (status == J_SUCCESS || status == J_IN_QUEUE){
        strncpy(return_msg, job->results, MAXBUFSIZE);
    } else {
        printf("job not found or incomplete: %d\n", status);
        get_status_msg(return_msg, server, job_id);
    }
}

void fail_job(struct Job *job, struct Stats *stats){
    job->status = J_FAILURE;
    stats->jobs_failed++;
    stats->jobs_processed++;
    strcpy(job->results, "job failed.");
}

void retry_job(struct Server *server, struct Job *job){
    if (job->retry_ct++ >= 3){
        fail_job(job, server->stats);
    }

    add_to_queue(server->queue, job->job_id);
    job->status = J_IN_QUEUE;
    job->worker_id = -1;
}

void handle_worker_disconnection(struct Server *server, int worker_fd){
    printf("Worker %d disconnected.\n", worker_fd);
    close(worker_fd);

    struct Worker *worker = get_worker_by_id(server->workers, worker_fd);

    if (worker->cur_job_id >= 0){
        struct Job *job = get_job_by_id(server->jobs, worker->cur_job_id);
        if (job != NULL){
            retry_job(server, job);
        }
    }
    server->stats->workers_ct--;
    remove_worker(server->workers, worker_fd);
}

void handle_worker_data(struct Server *server, int worker_fd){
    int rv;
    unsigned char buf[MAXBUFSIZE];
    memset(buf, 0, MAXBUFSIZE);

    rv = read(worker_fd, buf, MAXBUFSIZE);
    if (rv == 0){
        handle_worker_disconnection(server, worker_fd);
    }

    int offset = 0;
    int appid = unpacki16(buf+offset); offset += 2;
    int msg_type = unpacki16(buf+offset); offset += 2;

    if (appid != APPID){
        return;
    }

    struct Worker *worker = get_worker_by_id(server->workers, worker_fd);

    if (msg_type == WPACKET_STATUS){
        int status = unpacki16(buf+offset); offset += 2;
        int errcode = unpacki16(buf+offset); offset += 2;
        printf("worker %d status: [ %d ] | err [ %d ]\n", worker_fd, status, errcode);
        worker->errcode = errcode;
        worker->status = status;
    }

    if (msg_type == WPACKET_RESULTS){
        worker->status = W_READY;
        struct Job *job = get_job_by_id(server->jobs, worker->cur_job_id);
        strcpy(job->results, buf+offset);
        job->status = J_SUCCESS;
        server->stats->jobs_succeeded++;
        server->stats->jobs_processed++;
        worker->cur_job_id = -1;
        worker->jobs_completed++;
    }
}

void manage_worker(struct Server *server, struct Worker *worker){
    if (worker->status == W_FAILURE){
        struct Job *job = get_job_by_id(server->jobs, worker->cur_job_id);
        if (job == NULL) return;

        if (worker->errcode == WERR_INVALIDJOB){
            fail_job(job, server->stats);
        } else {
            retry_job(server, job);
        }
        worker->cur_job_id = -1;
        worker->status = W_READY;
        return;
    }

    if (worker->status == W_SUCCESS){
        struct Job *job = get_job_by_id(server->jobs, worker->cur_job_id);
        if (job == NULL) return;
        job->status = J_SUCCESS;
        worker->cur_job_id = -1;
        worker->status = W_READY;
        worker->jobs_completed++;
        return;
    }
}

void manage_worker_statuses(struct Server *server){
    struct Worker *worker = server->workers->head;

    for (worker; worker != NULL; worker = worker->next){
        if (worker->status != W_READY && worker->status != W_BUSY){
            manage_worker(server, worker);
        }
    }
}

void handle_client_request(struct Server *server){
    printf("\nClient request!\n");

    struct sockaddr_storage *their_addr = malloc(sizeof *their_addr);
    unsigned char buf[MAXBUFSIZE];
    memset(buf, 0, MAXBUFSIZE);
    socklen_t len_t = sizeof *their_addr;

    unsigned char metadata[MAXJOBMETADATASIZE];
    unsigned char return_msg[MAXBUFSIZE];
    int app_id;
    int cmd_type;
    int offset = 0;

    int new_fd = accept(server->client_listener, (struct sockaddr*)their_addr,  &len_t);
    recv(new_fd, buf, MAXBUFSIZE, 0);

    app_id = unpacki16(buf+offset); offset += 2;
    cmd_type = unpacki16(buf+offset); offset += 2;
    memcpy(metadata, buf+offset, MAXJOBMETADATASIZE);

    if (app_id != APPID){
        printf("incorrect app id: %d - %d\n", app_id, cmd_type);
        return;
    }

    printf("job type id: %d, metadata: %ld bytes\n", cmd_type, strlen(metadata));

    if (cmd_type == JOBSUBMITID){
        handle_job_submission(server, metadata, return_msg);
    }

    if (cmd_type == JOBSTATUSID){
        handle_job_status(server, metadata, return_msg);
    }

    if (cmd_type == JOBRESULTID){
        handle_job_get_results(server, metadata, return_msg);
    }

    send(new_fd, return_msg, strlen(return_msg), 0);
    close(new_fd);

    printf("\n");
}

void handle_new_worker(struct Server *server){
    printf("New worker.\n");

    struct sockaddr_storage *their_addr = malloc(sizeof *their_addr);
    socklen_t their_len = sizeof *their_addr;

    int new_fd = accept(server->worker_listener, (struct sockaddr*)their_addr, &their_len);
    if (new_fd == -1){
        return;
    }
    add_epoll_fd(server->epoll_fd, new_fd);

    struct Worker *new_worker = create_empty_worker();
    new_worker->id = new_fd;
    add_worker(server->workers, new_worker);
    server->stats->workers_ct++;

    unsigned char buf[MAXBUFSIZE];
    int offset = 0;
    packi16(buf+offset, APPID); offset += 2;
    packi16(buf+offset, WPACKET_CONNECTED); offset += 2;
    packi16(buf+offset, new_worker->id); offset += 2;

    send(new_fd, buf, offset, 0);
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

void print_stats(struct Stats *stats){
    if (stats->jobs_processed > 0){
        stats->success_rate = stats->jobs_succeeded * 100 / stats->jobs_processed;
    }
    printf("\n\n===== CURRENT STATS =====\n\n");

    printf("Jobs Processed: %d\n", stats->jobs_processed);
    printf("Successful Jobs : %d\n", stats->jobs_succeeded);
    printf("Failed Jobs: %d\n", stats->jobs_failed);
    printf("Jobs In Queue: %d\n", stats->jobs_in_queue);
    printf("Success Rate: %d%%\n", stats->success_rate);
    printf("Active Workers: %d\n", stats->workers_ct);

    printf("\n=========================\n\n");

}

/*
 * handle_input() -- handle server-side stdin input
 */
int handle_input(int stdin_fd, struct Server *server){
    char buffer[100];
    ssize_t bytes_read = read(stdin_fd, buffer, 100 - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Input: %s", buffer);

        if (strncmp(buffer, "quit", 4) == 0){
            return -1;
        }

        if (strncmp(buffer, "stats", 5) == 0){
            print_stats(server->stats);
        }
    }

    return 0;
}

/*
 * setup_server_struct() -- create the base Server struct with the appropriate file descriptors, returns pointer to said struct
 */
struct Server *setup_server_struct(int cfd, int wfd, int pfd){
    struct Server *server = malloc(sizeof *server);
    server->client_listener = cfd;
    server->worker_listener = wfd;
    server->epoll_fd = pfd;
    server->job_id_ct = 0;

    struct Jobs *jobs = malloc(sizeof *jobs);
    jobs->count = 0;
    jobs->head = NULL;
    jobs->tail = NULL;

    struct Stats *stats = malloc(sizeof *stats);
    stats->jobs_failed = 0;
    stats->jobs_processed = 0;
    stats->jobs_succeeded = 0;
    stats->success_rate = 0;
    stats->workers_ct = 0;
    stats->jobs_in_queue = 0;

    struct Workers *workers = malloc(sizeof *workers);
    workers->available_workers = 0;
    workers->count = 0;
    workers->head = NULL;
    workers->tail = NULL;

    server->stats = stats;
    server->jobs = jobs;
    server->workers = workers;
    server->queue = create_queue();;

    add_epoll_fd(pfd, 0);
    add_epoll_fd(pfd, cfd);
    add_epoll_fd(pfd, wfd);

    return server;
}

/*
 * handle_shutdown() -- safely shut down the server
 */
void handle_shutdown(struct Server *server){
    printf("\nshutting down...\n");
    close(server->client_listener);
    close(server->worker_listener);
    close(server->epoll_fd);
    exit(EXIT_SUCCESS);
    printf("goodbye.\n");
}

int main(){
    printf("starting server...\n");
    int client_fd = get_listening_socket(CLIENT_PORT);
    int worker_fd = get_listening_socket(WORKER_PORT);
    int epoll_fd = create_epoll();

    struct Server *server = setup_server_struct(client_fd, worker_fd, epoll_fd);

    // Event loop
    struct epoll_event events[MAXEPOLLEVENTS];

    printf("server setup complete. waiting for connections...\n\n");
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAXEPOLLEVENTS, 0);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                int fd = events[i].data.fd;
                if (fd == 0){
                    if (handle_input(fd, server) == -1) handle_shutdown(server);
                    continue;
                }
                if (fd == client_fd){
                    handle_client_request(server);
                    continue;
                }
                if (fd == worker_fd){
                    handle_new_worker(server);
                    continue;
                }

                handle_worker_data(server, fd);

            }
        }

        check_queue(server);
        manage_worker_statuses(server);
    }

    handle_shutdown(server);
}