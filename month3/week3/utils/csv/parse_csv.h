/*
 * parse_csv.h -- CSV parser interface and data structures
 */

#ifndef PARSE_CSV_H
#define PARSE_CSV_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "../../common.h"

/*
 * struct CSV -- Three-level pointer structure for CSV data
 *
 * Access: csv->csv_data[row][col] returns char* (string)
 * Example: csv->csv_data[1][2] = "Portland" (row 1, column 2)
 */
struct CSV {
    int rows;
    int cols;
    char ***csv_data;
};

/* Count CSV dimensions without full parsing (resets file position) */
void get_size(FILE *fptr, int *rows, int *cols);

/* Parse CSV into structure with buffer boundary handling */
void populate_csv(struct CSV *csv, FILE *fptr);

/* Allocate CSV structure and parse file (main entry point) */
void parse_csv(struct CSV *csv, FILE *fptr);

/* Debug output to stdout */
void print_csv(struct CSV *csv);

#endif