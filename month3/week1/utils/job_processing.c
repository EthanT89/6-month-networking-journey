/*
 * job_processing.c -- job handling logic
 */

#include "./job_processing.h"

int determine_job_type(unsigned char buf[MAXBUFSIZE], int size){
    unsigned char keyword[size];
    int i = 0;

    for (i; i < size; i++){
        if (buf[i] == ' '){
            break;
        }
    }

    strncpy(keyword, buf, i);
    keyword[i] = '\0';

    memmove(buf, buf+i, MAXBUFSIZE-i);

    if (strlen(keyword) == 0){
        printf("no keyword...\n");
        return -1;
    }

    if (strcmp(keyword, "wordcount") == 0){
        return JTYPE_WORDCOUNT;
    }

    if (strcmp(keyword, "echo") == 0){
        return JTYPE_ECHO;
    }

    if (strcmp(keyword, "capitalize") == 0){
        return JTYPE_CAPITALIZE;
    }

    if (strcmp(keyword, "charcount") == 0){
        return JTYPE_CHARCOUNT;
    }

    return -1;
}

int job_wordcount(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]){
    int count = 0;
    int len = strlen(content);

    int seen_word = 0;

    for (int i = 0; i < len; i++){
        if (content[i] == ' ') seen_word = 0;

        if (content[i] != ' ' && seen_word == 0){
            seen_word = 1;
            count++;
        }
    }

    sprintf(result, "word count: %d", count);
    return 1;
}

int job_charcount(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]){
    int count = 0;
    int len = strlen(content);

    for (int i = 0; i < len; i++){
        if (content[i] != ' ') count++;
    }

    sprintf(result, "char count: %d", count);
    return 1;
}

int job_echo(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]){
    strncpy(result, content, MAXRESULTSIZE);
    return 1;
}

int job_capitalize(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]){
    for (int i = 0; i < strlen(content); i++){
        if (islower(content[i])) {
            content[i] = toupper(content[i]);
        }
    }

    strncpy(result, content, MAXRESULTSIZE);
    return 1;
}

int process_job(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]){
    int job_type = determine_job_type(content, strlen(content));

    if (job_type == -1){
        return WERR_INVALIDJOB;
    }

    if (job_type == JTYPE_WORDCOUNT){
        return job_wordcount(result, content);
    }

    if (job_type == JTYPE_CHARCOUNT){
        return job_charcount(result, content);
    }

    if (job_type == JTYPE_ECHO){
        return job_echo(result, content);
    }

    if (job_type == JTYPE_CAPITALIZE){
        return job_capitalize(result, content);
    }

    return 1;
}

