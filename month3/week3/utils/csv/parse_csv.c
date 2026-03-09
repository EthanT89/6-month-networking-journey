/*
 * parse_csv.c -- Two-pass CSV parser with chunked I/O and buffer boundary handling
 *
 * Parses CSV files into a 3D array structure (rows x columns x characters).
 * First pass counts dimensions, second pass allocates memory and populates data.
 * Handles quoted fields, embedded commas, and words split across read boundaries.
 */

#include "./parse_csv.h"

/*
 * get_size() -- count CSV dimensions (rows and columns) in first pass
 *
 * Reads file in chunks, counting newlines for rows and commas in first row
 * for columns. Resets file position to start when done. Handles files
 * without trailing newlines.
 */
void get_size(FILE *fptr, int *rows, int *cols){
    char content_read[MAXFILEREAD];
    int bytes_read;

    int total_newlines = 0;
    *cols = 1;
    char last_char = '\0';

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, fptr)) > 0){
        for (int i = 0; i < bytes_read; i++){
            if (content_read[i] == '\n') {
                total_newlines++;
            }
            else if (total_newlines <= 0 && content_read[i] == ',') {
                (*cols)++;
            }
            last_char = content_read[i];
        }
    }



    *rows = total_newlines;
    if (last_char != '\n' && last_char != '\0') {
        (*rows)++;
    }
    
    fseek(fptr, 0, SEEK_SET);   
}

/*
 * populate_csv() -- second pass: parse and store CSV data with buffer boundary handling
 *
 * Reads file in chunks, handling:
 * - Quoted fields (commas inside quotes don't delimit)
 * - Words split across chunk boundaries (tempbuf carries partial words)
 * - Space handling (spaces within fields, not just delimiters)
 * - Final cells without trailing newline
 *
 * Uses tempbuf to preserve partial words when chunk ends mid-field.
 * Example: "Port" at end of chunk combines with "land" in next chunk → "Portland"
 */
void populate_csv(struct CSV *csv, FILE *fptr){
    char content[MAXFILEREAD];
    int bytes_read;

    int ignore_comma = 0;
    int len_cur_word = 0;
    int start_index = 0;
    int reading_word = 0;

    int row = 0;
    int col = 0;

    char tempbuf[MAXFILEREAD];
    tempbuf[0] = '\0';

    while ((bytes_read = fread(content, sizeof(char), MAXFILEREAD, fptr)) > 0){
        for (int i = 0; i < bytes_read; i++){
            if (content[i] == '"'){
                if (ignore_comma) ignore_comma = 0;
                else ignore_comma = 1;

                if (reading_word == 0){
                    start_index = i;
                    reading_word = 1;
                } else {
                    len_cur_word++;
                }
                continue;
            }

            if ((content[i] == '\n' || content[i] == ',') && ignore_comma == 0){
                int buf_len = strlen(tempbuf);
                csv->csv_data[row][col] = malloc(len_cur_word+buf_len+1);

                if (buf_len > 0){
                    strncpy(csv->csv_data[row][col], tempbuf, buf_len);
                }

                strncpy(csv->csv_data[row][col]+buf_len, content+start_index, len_cur_word);
                csv->csv_data[row][col][len_cur_word+buf_len] = '\0';

                len_cur_word = 0;
                reading_word = 0;

                if (content[i] == '\n'){
                    row++;
                    col = 0;
                } else {
                    col++;
                }

                tempbuf[0] = '\0';
                continue;
            }

            if (content[i] == ' '){
                if (reading_word) len_cur_word++;  // Spaces inside fields are part of the word
                continue;
            }

            if (reading_word != 1){
                start_index = i;
                reading_word = 1;
            }

            len_cur_word++;
        }
        // Save incomplete word to tempbuf for next chunk
        if (len_cur_word > 0){
            strncpy(tempbuf, content+start_index, len_cur_word);
            tempbuf[len_cur_word] = '\0';
            start_index = 0;
            len_cur_word = 0;
        }
    }
    // Handle final cell if file doesn't end with newline
    int buf_len = strlen(tempbuf);
    if ( buf_len > 0){
        csv->csv_data[row][col] = malloc(buf_len+1);
        strncpy(csv->csv_data[row][col], tempbuf, buf_len);
        csv->csv_data[row][col][buf_len] = '\0';
    }
}

/*
 * parse_csv() -- main entry point: allocate 3-level CSV structure and populate
 *
 * Three-level malloc:
 *   Level 1: Array of row pointers
 *   Level 2: Array of column pointers per row
 *   Level 3: Individual cell strings (allocated in populate_csv)
 *
 * Access pattern: csv->csv_data[row][col] returns char*
 */
void parse_csv(struct CSV *csv, FILE *fptr){
    get_size(fptr, &csv->rows, &csv->cols);
    int rows = csv->rows;
    int cols = csv->cols;

    csv->csv_data = malloc(rows * sizeof(char**));
    for (int i = 0; i < rows; i++) {
        csv->csv_data[i] = malloc(cols * sizeof(char*));
    }

    populate_csv(csv, fptr);
}

/*
 * print_csv() -- debug function to output CSV structure to stdout
 */
void print_csv(struct CSV *csv){
    printf("Rows: %d, Cols: %d\n", csv->rows, csv->cols);
    for (int i = 0; i < csv->rows; i++) {
        for (int j = 0; j < csv->cols; j++) {
            printf("%s", csv->csv_data[i][j]);
            if (j < csv->cols - 1) printf(",");
        }
        printf("\n");
    }
}