/*
 * job_processing.c -- File-based job execution with streaming chunk processing
 */

#include "./job_processing.h"

/*
 * determine_job_type() -- parse job metadata to identify job type
 *
 * Extracts the first word from the buffer (up to space), moves remaining
 * content to start of buffer, and returns the corresponding JTYPE constant
 */
int determine_job_type(unsigned char buf[MAXBUFSIZE], int size){
    unsigned char keyword[size];
    int i = 0;
    int type = -1;

    for (i; i < size; i++){
        if (buf[i] == ' '){
            break;
        }
    }

    strncpy(keyword, buf, i);
    keyword[i] = '\0';

    i++;
    memmove(buf, buf+i, MAXBUFSIZE-i);

    if (strlen(keyword) == 0){
        printf("huh\n");
        return type;
    }

    if (strcmp(keyword, "wordcount") == 0){
        type =  JTYPE_WORDCOUNT;
    }

    if (strcmp(keyword, "echo") == 0){
        type = JTYPE_ECHO;
    }

    if (strcmp(keyword, "capitalize") == 0){
        type = JTYPE_CAPITALIZE;
    }

    if (strcmp(keyword, "charcount") == 0){
        type = JTYPE_CHARCOUNT;
    }

    if (strcmp(keyword, "csvsort") == 0){
        type = JTYPE_CSVSORT;
    }

    if (strcmp(keyword, "csvstats") == 0){
        type = JTYPE_CSVSTATS;
    }

    if (strcmp(keyword, "csvfilter") == 0){
        type = JTYPE_CSVFILTER;
    }

    return type;
}

void strip_whitespace(char *string){
    int len = strlen(string);
    int i = 0;

    for (i; i < len; i++){
        if (string[i] != ' ') break;
    }

    memmove(string, string+i, len-i);
}

void extract_first_word(char *dest, char *orig){
    int j = 0;
    for (j; j < strlen(orig); j++){
        if (orig[j] == ' ') break;
    }
    strncpy(dest, orig, j);
    dest[j] = '\0';

    memmove(orig, orig+j, MAXBUFSIZE-j);
    strip_whitespace(orig);
}

/*
 * job_wordcount() -- count words in content string
 *
 * Words are defined as sequences of non-space characters separated by spaces
 */
int job_wordcount(FILE *results, FILE *content){
    char content_read[MAXFILEREAD];
    int bytes_read;
    int wordcount = 0;
    int seen_space = 1;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){

        for (int i = 0; i < bytes_read; i++){
            if (content_read[i] == ' ') seen_space = 1;
            else if (seen_space == 1) {
                wordcount++;
                seen_space = 0;
            }
        }
    }

    fprintf(results, "%d total words", wordcount);
    return 1;
}

/*
 * job_charcount() -- count non-space characters in content string
 */
int job_charcount(FILE *results, FILE *content){
    char content_read[MAXFILEREAD];
    int bytes_read;
    int charcount = 0;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){
        for (int i = 0; i < bytes_read; i++){
            if (content_read[i] != ' ') charcount++;
        }
    }

    fprintf(results, "%d total characters", charcount);
    return 1;
}

/*
 * job_echo() -- echo content back as result
 */
int job_echo(FILE *results, FILE *content){
    char content_read[MAXFILEREAD];
    int bytes_read;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){
        fprintf(results, "%s", content_read);
    }
    return 1;
}

/*
 * job_capitalize() -- convert all lowercase letters in content to uppercase
 */
int job_capitalize(FILE *results, FILE *content){
    char content_read[MAXFILEREAD];
    int bytes_read;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){
        for (int i = 0; i < bytes_read; i++){
            if (islower(content_read[i])) {
                fprintf(results, "%c", toupper(content_read[i]));
                continue;
            }
            fprintf(results, "%c", content_read[i]);
        }
    }
    return 1;
}

int job_csvstats(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]){
    char content_read[MAXFILEREAD];
    int bytes_read;

    int total_newlines = 0;
    int cols = 1;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){
        for (int i = 0; i < bytes_read; i++){
            if (content_read[i] == '\n') {
                total_newlines++;
            }
            else if (total_newlines == 0 && content_read[i] == ',') {
                cols++;
            }
        }
    }

    fprintf(results, "%d total entries, %d columns", total_newlines-1, cols);
    return 1;
}

int job_csvsort(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]){
    return -1;
}

int job_csvfilter(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]){
    char filter_keyword[MAXFILEPATH];
    char filter_key[MAXFILEPATH];

    extract_first_word(filter_keyword, header);
    extract_first_word(filter_key, header);

    printf("keyword: -%s-\n", filter_keyword);
    printf("key: -%s-\n", filter_key);

    struct CSV *csv = malloc(sizeof *csv);
    csv->cols = 0;
    csv->rows = 0;
    parse_csv(csv, content);

    int col_idx = -1;

    for(int j = 0; j < csv->cols; j++){
        if (strcmp(csv->csv_data[0][j], filter_keyword) == 0){
            col_idx = j;
            break;
        }
    }
    if (col_idx == -1) return -1;
    
    for (int j = 0; j < csv->cols; j++){
        if (j > 0) fprintf(results, ",");
        fprintf(results, "%s", csv->csv_data[0][j]);
    }
    fprintf(results, "\n");

    for (int i = 0; i < csv->rows; i++){
        if (strcmp(csv->csv_data[i][col_idx], filter_key) == 0){
            for (int j = 0; j < csv->cols; j++){
                if (j > 0) fprintf(results, ",");
                fprintf(results, "%s", csv->csv_data[i][j]);
            }
            fprintf(results, "\n");
        }
    }

    return 1;
}

/*
 * process_job() -- route job to appropriate handler based on type
 *
 * Determines job type from content, calls appropriate job function, returns result or error code
 */
int process_job(unsigned char header[MAXBUFSIZE], unsigned char dir[MAXFILEPATH]){
    int job_type = determine_job_type(header, strlen(header));
    int rv = 1;

    if (job_type == -1){
        return WERR_INVALIDJOB;
    }

    char fcontent[MAXFILEPATH];
    strcpy(fcontent, dir);
    strcat(fcontent, "content.txt");
    FILE *content_file = fopen(fcontent ,"r");

    char fresults[MAXFILEPATH];
    strcpy(fresults, dir);
    strcat(fresults, "results.txt");
    FILE *results_file = fopen(fresults ,"w");
    
    fclose(fopen(fresults, "w"));

    if (job_type == JTYPE_WORDCOUNT){
        rv = job_wordcount(results_file, content_file);
    } 
    
    else if (job_type == JTYPE_CHARCOUNT){
        rv = job_charcount(results_file, content_file);
    }

    else if (job_type == JTYPE_ECHO){
        rv = job_echo(results_file, content_file);
    }

    else if (job_type == JTYPE_CAPITALIZE){
        rv = job_capitalize(results_file, content_file);
    }

    else if (job_type == JTYPE_CSVFILTER){
        rv = job_csvfilter(results_file, content_file, header);
    }

    else if (job_type == JTYPE_CSVSORT){
        rv = job_csvsort(results_file, content_file, header);
    }

    else if (job_type == JTYPE_CSVSTATS){
        rv = job_csvstats(results_file, content_file, header);
    }

    fclose(results_file);
    fclose(content_file);
    return rv;
}

