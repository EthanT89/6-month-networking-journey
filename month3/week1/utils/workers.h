/*
 * workers.h -- worker management for server use
 */

#ifndef WORKERS_H
#define WORKERS_H

#include "../common.h"

#include <stddef.h>
#include <stdlib.h>

/*
 * Worker -- struct for server managment of workers
 *
 * id -- unique worker id
 * status -- current status of the worker (ready, busy, failure, etc.)
 * jobs_completed -- total number of successful jobs completed by the worker
 */
struct Worker {
    int id;
    int status;
    int jobs_completed;
    int cur_job_id;
    unsigned char cur_job_results[MAXRESULTSIZE];

    struct Worker *next;
};

/*
 * Workers -- SLL data structure for server management of multiple workers
 *
 * *head -- pointer to the first worker in the list
 * *tail -- pointer to the last worker in the list
 * count -- total number of workers
 * available_workers -- total number of available workers
 */
struct Workers {
    struct Worker *head;
    struct Worker *tail;
    int count;
    int available_workers;
};

/*
 * create_empty_worker() -- initialize a worker struct and return a pointer to it
 */
struct Worker *create_empty_worker();

/*
 * add_worker() -- add a worker to the SLL 
 */
void add_worker(struct Workers *workers, struct Worker *worker);

/*
 * remove_worker() -- remove a worker from the SLL given its ID
 */
void remove_worker(struct Workers *workers, int worker_id);

/*
 * get_worker_by_id() -- given an id, search through the SLL and return a pointer to the corresponding worker, NULL if not found
 */
struct Worker *get_worker_by_id(struct Workers *workers, int worker_id);

/*
 * get_available_worker() -- search through the SLL and return a pointer to the first W_READY worker, NULL if not found
 */
struct Worker *get_available_worker(struct Workers *workers);

#endif