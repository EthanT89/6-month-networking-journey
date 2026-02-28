/*
 * workers.c -- worker utilities for the server
 */

#include "./workers.h"

/*
 * create_empty_worker() -- allocate and initialize a worker struct with default values
 */
struct Worker *create_empty_worker(){
    struct Worker *worker = malloc(sizeof *worker);
    worker->id = -1;
    worker->jobs_completed = 0;
    worker->next = NULL;
    worker->status = W_READY;
    worker->cur_job_id = -1;
    worker->errcode = 1;

    return worker;
}

/*
 * add_worker() -- add a worker to the workers SLL
 */
void add_worker(struct Workers *workers, struct Worker *worker){
    if (worker == NULL){
        return;
    }

    if (workers->count == 0){
        workers->head = workers->tail = worker;
        workers->count++;
        return;
    }

    workers->tail->next = worker;
    workers->tail = worker;
    workers->count++;
    return;
}

/*
 * remove_worker() -- remove a worker from the SLL by worker_id and free its memory
 */
void remove_worker(struct Workers *workers, int worker_id){
    if (workers->count == 0){
        return;
    }

    struct Worker *res = workers->head;
    struct Worker *prev = NULL;

    if (workers->count == 1){
        if (workers->head->id != worker_id) return;
        free(workers->head);
        workers->head = workers->tail = NULL;
        workers->count--;
        return;
    }

    for (res; res != NULL && res->id != worker_id; res = res->next){
        prev = res;
    }

    if (res == NULL) return;

    if (res == workers->head){
        workers->head = res->next;
    } else if (res == workers->tail){
        workers->tail = prev;
        prev->next = NULL;
    } else {
        prev->next = res->next;
    }

    free(res);
    workers->count--;
    return;
}

/*
 * get_worker_by_id() -- find and return worker with matching worker_id, NULL if not found
 */
struct Worker *get_worker_by_id(struct Workers *workers, int worker_id){
    if (workers->count != 0){
        for (struct Worker *cur = workers->head; cur != NULL; cur = cur->next){
            if (cur->id == worker_id) return cur;
        }
    }
    return NULL;
}

/*
 * get_available_worker() -- find and return first worker with W_READY status, NULL if none available
 */
struct Worker *get_available_worker(struct Workers *workers){
    if (workers->count != 0){
        for (struct Worker *cur = workers->head; cur != NULL; cur = cur->next){
            if (cur->status == W_READY) return cur;
        }
    }
    return NULL;
}
