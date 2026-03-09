#ifndef PARSE_CSV_H
#define PARSE_CSV_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "../../common.h"

struct CSV {
    int rows;
    int cols;
    char ***csv_data; // List of a list of a list of chars. Ex: csv[1][2][3] accesses row 1, column 2, character 3. Likely to only use two index gets.
};

void get_size(FILE *fptr, int *rows, int *cols);

void populate_csv(struct CSV *csv, FILE *fptr);

void parse_csv(struct CSV *csv, FILE *fptr);

void print_csv(struct CSV *csv);

#endif