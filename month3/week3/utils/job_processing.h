/*
 * job_processing.h -- handles the processing of various job types, utilized by workers
 */

#ifndef JOB_PROCESSING_H
#define JOB_PROCESSING_H

#include "./jobs.h"
#include "../common.h"
#include "./file_transfer.h"
#include "./csv/parse_csv.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

int determine_job_type(unsigned char buf[MAXBUFSIZE], int size);

int job_charcount(FILE *results, FILE *content);

int job_wordcount(FILE *results, FILE *content);

int job_echo(FILE *results, FILE *content);

int job_capitalize(FILE *results, FILE *content);

int job_csvstats(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]);

int job_csvsort(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]);

int job_csvfilter(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]);

int process_job(unsigned char content[MAXBUFSIZE], unsigned char fname[MAXFILEPATH]);

#endif