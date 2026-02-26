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

    return -1;
}

int job_wordcount(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]){
    int count = 0;
    int len = strlen(content);

    for (int i = 0; i < len; i++){
        if (content[i] != ' ') count++;
    }

    sprintf(result, "word count: %d", count);
    return 1;
}

int job_echo(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]){
    strncpy(result, content, MAXRESULTSIZE);
}

int job_capitalize(unsigned char result[MAXRESULTSIZE], unsigned char content[MAXBUFSIZE]){
    for (int i = 0; i < strlen(content); i++){
        if (islower(content[i])) {
            content[i] = toupper(content[i]);
        }
    }

    strncpy(result, content, MAXRESULTSIZE);
}