/*
 * job_processing.h -- handles the processing of various job types, utilized by workers
 */

#ifndef JOB_PROCESSING_H
#define JOB_PROCESSING_H

#include "./jobs.h"
#include "../common.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

int determine_job_type(unsigned char buf[MAXBUFSIZE], int size);

int job_wordcount(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]);

int job_echo(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]);

int job_capitalize(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]);

#endif