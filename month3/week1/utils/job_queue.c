/*
 * job_queue.c -- job queue logic
 */

#include "./job_queue.h"

struct JobQ *create_jobq(int job_id){
    struct JobQ *jobq = malloc(sizeof *jobq);
    jobq->job_id = job_id;
    jobq->next = NULL;

    return jobq;
}

struct JobQueue *create_queue(){
    struct JobQueue *queue = malloc(sizeof *queue);
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;

    return queue;
}

void add_to_queue(struct JobQueue *queue, int job_id){
    struct JobQ *job = create_jobq(job_id);

    if (queue->count == 0){
        queue->head = queue->tail = job;
        queue->count++;
        return;
    }

    queue->tail->next = job;
    queue->tail = job;
    queue->count++;
}

int pop_queue(struct JobQueue *queue){
    if (queue->count <= 0){
        return -1;
    }

    int job_id = queue->head->job_id;
    struct JobQ *head = queue->head;

    if (queue->count == 1){
        queue->head = queue->tail = NULL;
    } else {
        queue->head = head->next;
    }

    free(head);
    queue->count--;
    return job_id;
}