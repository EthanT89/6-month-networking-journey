/*
 * jobs.c -- management and storage of jobs in the task queue server
 */

 #include "./jobs.h"

 /*
 * create_blank_job() -- create a 0-initialized job on the heap
 */
struct Job *create_blank_job(){
    struct Job *job = malloc(sizeof *job);
    job->job_id = -1;
    job->job_type = -1;
    job->retry_ct = 0;
    job->status = J_IN_QUEUE;
    job->next = NULL;
    job->time_start = -1;
    job->worker_id = -1;
    job->results[0] = '\0';

    return job;
}

/*
 * add_job() -- add an initialized job to the jobs SLL
 */
void add_job(struct Jobs *jobs, struct Job *job){
    if (job == NULL){
        return;
    }

    if (jobs->count == 0){
        jobs->head = jobs->tail = job;
        jobs->count++;
        return;
    }

    jobs->tail->next = job;
    jobs->tail = job;
    jobs->count++;
    return;
}

/*
 * remove_job() -- remove a job from the SLL
 */
void remove_job(struct Jobs *jobs, int job_id){
    if (jobs->count == 0){
        return;
    }

    struct Job *res = jobs->head;
    struct Job *prev = NULL;

    if (jobs->count == 1){
        if (jobs->head->job_id != job_id) return;
        free(jobs->head);
        jobs->head = jobs->tail = NULL;
        jobs->count--;
        return;
    }

    for (res; res != NULL && res->job_id != job_id; res = res->next){
        prev = res;
    }

    if (res == NULL) return;

    if (res == jobs->head){
        jobs->head = res->next;
    } else if (res == jobs->tail){
        jobs->tail = prev;
        prev->next = NULL;
    } else {
        prev->next = res->next;
    }

    free(res);
    jobs->count--;
    return;
}

/*
 * get_job_by_id() -- return a pointer to the corresponding job given an id, NULL if not found
 */
struct Job *get_job_by_id(struct Jobs *jobs, int job_id){
    if (jobs->count != 0){
        for (struct Job *cur = jobs->head; cur != NULL; cur = cur->next){
            if (cur->job_id == job_id) return cur;
        }
    }
    return NULL;
}

/*
 * get_job_status() -- return the status code of a job, given it's ID
 */
int get_job_status(struct Jobs *jobs, int job_id){
    if (jobs->count != 0){
        for (struct Job *cur = jobs->head; cur != NULL; cur = cur->next){
            if (cur->job_id == job_id) return cur->status;
        }
    }
    return -1;
}

/*
 * get_job_type() -- given a command string, identify the corresponding job type, then create and return a pointer to the new job
 */
struct Job *get_job(unsigned char command[MAXJOBCOMMANDSIZE]){
    // TODO: setup modular job type extraction, identify starting job types and formatting
}