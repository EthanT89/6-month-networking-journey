/*
 * job_queue.c -- job queue logic
 */

#include "./job_queue.h"

/*
 * create_jobq() -- create a job queue node with job_id initialized and next set to NULL
 */
struct JobQ *create_jobq(int job_id){
    struct JobQ *jobq = malloc(sizeof *jobq);
    jobq->job_id = job_id;
    jobq->next = NULL;

    return jobq;
}

/*
 * print_queue() -- display all jobs in queue with their positions for debugging
 */
void print_queue(struct JobQueue *queue){
    struct JobQ *job = queue->head;
    int position = 1;

    printf("\n=== JOBS IN QUEUE ===\n");
    for (job; job != NULL; job = job->next){
        printf("%d: Job %d\n", position, job->job_id);
        position++;
    }

    if (queue->count == 0){
        printf("0 jobs in queue.\n");
    }

    printf("\n");
}

/*
 * create_queue() -- initialize an empty job queue
 */
struct JobQueue *create_queue(){
    struct JobQueue *queue = malloc(sizeof *queue);
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;

    return queue;
}

/*
 * add_to_queue() -- append a job to the tail of the queue
 */
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

/*
 * pop_queue() -- remove and return the job_id from the head of the queue
 *
 * Frees the removed node's memory. Returns -1 if queue is empty.
 */
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