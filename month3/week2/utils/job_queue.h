/*
 * job_queue.h -- job queue management and logic implementation
 */

#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <stdlib.h>
#include <stddef.h>

/*
 * JobQ -- extremely basic job queue node
 */
struct JobQ {
    int job_id;
    struct JobQ *next;
};

/*
 * JobQueue -- singly linked list-based queue implementation for jobs
 */
struct JobQueue {
    struct JobQ *head;
    struct JobQ *tail;
    int count;
};

/*
 * create_jobq() -- create a jobq struct and return its pointer
 */
struct JobQ *create_jobq(int job_id);

/*
 * create_queue() -- create an empty JobQueue struct and return its pointer
 */
struct JobQueue *create_queue();

/*
 * add_to_queue() -- add a new job to the job queue
 */
void add_to_queue(struct JobQueue *queue, int job_id);

/*
 * pop_queue() -- pop and return the id of the first job in line, returns -1 if empty
 */
int pop_queue(struct JobQueue *queue);

#endif