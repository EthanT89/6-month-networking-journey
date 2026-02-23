/*
 * jobs.h -- logic file for managing, but not processing, jobs
 */

#ifndef JOBS_H
#define JOBS_H

// imports
#include "../common.h"

/*
 * Job -- struct for containing Job data
 *
 * job_id -- id assigned by server to facilitate lookups
 * worker_id -- id of worker assigned to the job
 * 
 * status -- status code of task process
 * time_start -- time, since program start, that the job began
 * 
 * job_type -- job code type for the job being processed
 * *next -- pointer to the next job in the linked list
 */
struct Job {
    int job_id;
    int worker_id;

    int status;
    unsigned char results[MAXRESULTSIZE];
    int time_start;

    int job_type;
    struct Job *next;
};

/*
 * Jobs -- simple linked list for managing multiple jobs
 */
struct Jobs {
    struct Job *head;
    struct Job *tail;
    int count;
};

/*
 * create_blank_job() -- create a 0-initialized job on the heap
 */
struct Job *create_blank_job();

/*
 * add_job() -- add an initialized job to the jobs SLL
 */
void add_job(struct Jobs *jobs, struct Job *job);

/*
 * remove_job() -- remove a job from the SLL
 */
void remove_job(struct Jobs *jobs, int job_id);

/*
 * get_job_by_id() -- return a pointer to the corresponding job given an id, NULL if not found
 */
struct Job *get_job_by_id(struct Jobs *jobs, int job_id);

/*
 * get_job_status() -- return the status code of a job, given it's ID
 */
int get_job_status(struct Jobs *jobs, int job_id);

/*
 * get_job_type() -- given a command string, identify the corresponding job type, then create and return a pointer to the new job
 */
struct Job *get_job(unsigned char command[MAXJOBCOMMANDSIZE]);

#endif