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
#include <wand/MagickWand.h>

int determine_job_type(unsigned char buf[MAXBUFSIZE], int size);

/* String parsing utilities for job argument extraction */
void strip_whitespace(char *string);
void extract_first_word(char *dest, char *orig);

/* Simple job types (no structured data parsing) */
int job_charcount(FILE *results, FILE *content);

int job_wordcount(FILE *results, FILE *content);

int job_echo(FILE *results, FILE *content);

int job_capitalize(FILE *results, FILE *content);

/* CSV job types (require CSV parsing) */
int job_csvstats(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]);

int job_csvsort(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]);

int job_csvfilter(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]);

int job_scale(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path);

int job_resize(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path);

int job_filter_img(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path);

/* CSV sorting helpers (merge sort on index array) */
void job_csvsort_mergesort(char ***csv, int col, int *idx_arr, int left, int right);
void job_csvsort_mergesort_helper(char ***csv, int col, int *idx_arr, int left, int middle, int right);

int process_job(unsigned char content[MAXBUFSIZE], char fname[MAXFILEPATH], char ext[MAXFILEEXT]);

#endif