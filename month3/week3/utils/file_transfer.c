/*
 * file_transfer.c -- Generic file send/receive functions using "FILE OK" sentinel protocol
 */

#include "./file_transfer.h"

/*
 * get_file_size() -- return file size in bytes using fseek/ftell, resets position to start
 */
long get_file_size(FILE *file) {
    long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET); // Reset to beginning
    return size;
}

void get_file_extension(char *fname, char *ext){
    int namelen = strlen(fname);
    int i = namelen-1;

    for (i; i >= 0; i--){
        if (fname[i] == '.') break;
    }

    if (i == 0) ext[0] = '\0';
    else strcpy(ext, fname+i);
}

/*
 * receive_file_text_based() -- receive file from socket in chunks until "FILE OK" marker detected
 */
int receive_file_text_based(char *fname, int sockfd){
    printf("receiving text file\n");
    int total_bytes = 0;
    int expected_bytes;
    int bytes_read;
    int epollfd = create_epoll();
    struct epoll_event events[1];

    char buf[MAXBUFSIZE];
    add_epoll_fd(epollfd, sockfd);

    fclose(fopen(fname, "w"));

    memset(buf, 0, MAXBUFSIZE);
    if ((bytes_read = read(sockfd, buf, 8)) == 0){
        return -1;
    }

    printf("bytes read: %d\n", bytes_read);
    expected_bytes = unpacki64(buf);
    printf("total expected bytes: %d\n", expected_bytes);
 
    while (1) {
        if (total_bytes == expected_bytes) return 1;
        else if (total_bytes > expected_bytes) return -1;

        int nfds = epoll_wait(epollfd, events, 1, 50);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                if ((bytes_read = read(sockfd, buf, MAXBUFSIZE)) == 0){
                    return -1;
                }
                FILE *fptr = fopen(fname ,"a");

                total_bytes += bytes_read;
                fprintf(fptr, "%s", buf);
                printf("total bytes received: %d - expected: %d\n", total_bytes, expected_bytes);   
                fclose(fptr); // Always close the file
                memset(buf, 0, MAXBUFSIZE);
            }
        }
    }

    if (expected_bytes == total_bytes) return 1;
    return -1;
}

/*
 * send_file_text_based() -- send file in MAXBUFSIZE chunks followed by "FILE OK" sentinel marker
 */
int send_file_text_based(int sockfd, char *file_name){
	printf("\nSending %s...\n", file_name);
    FILE *fs = fopen(file_name, "r");
    long file_size = get_file_size(fs);
    char sdbuf[MAXBUFSIZE]; 

    if(fs == NULL){
        printf("ERROR: File %s not found.\n", file_name);
        return -1;
    }

    memset(sdbuf, 0, MAXBUFSIZE);
    int fs_block_sz;
    int bytes_sent;
    int total_bytes = 0;

    packi16(sdbuf, TXT_FILE);
    packi64(sdbuf+2, file_size);
    if ((bytes_sent = send(sockfd, sdbuf, 10, 0)) <= 0){
        fprintf(stderr, "ERROR: Failed to send file %s.\n", file_name);
        return -1;
    }
    printf("file id: %d, size: %ld\n", unpacki16(sdbuf), unpacki64(sdbuf+2));
    memset(sdbuf, 0, MAXBUFSIZE);

    while((fs_block_sz = fread(sdbuf, sizeof(char), MAXBUFSIZE, fs)) > 0)
    {
        if((bytes_sent = send(sockfd, sdbuf, fs_block_sz, 0)) < 0)
        {
            fprintf(stderr, "ERROR: Failed to send file %s.\n", file_name);
            return -1;
        }
        total_bytes += bytes_sent;
        memset(sdbuf, 0, MAXBUFSIZE);
    }

    printf("File %s was Sent! - %d bytes\n\n", file_name, total_bytes);
    return 1;
}

/*
 * receive_file_text_based() -- receive binary image file from socket in chunks (in binary mode)
 *
 * NOTE: Cannot rely on FILE OK marker, need to receive file size first.
 */
int receive_file_img_based(int sockfd, char *fname){
    int total_bytes = 0;
    int expected_bytes;
    int bytes_read;
    int epollfd = create_epoll();
    struct epoll_event events[1];

    if (strncmp(fname + (strlen(fname) - 3), "jpg", 3) > 0 ){
        printf("file extension invalid. IS: .%s NOT: .jpg\n", fname + (strlen(fname) - 3));
        return -1;
    }

    char buf[MAXBUFSIZE];
    add_epoll_fd(epollfd, sockfd);

    fclose(fopen(fname, "wb"));

    memset(buf, 0, MAXBUFSIZE);
    if ((bytes_read = read(sockfd, buf, 8)) == 0){
        return -1;
    }
    expected_bytes = unpacki64(buf);
    printf("total expected bytes: %d\n", expected_bytes);

    while (1) {
        if (total_bytes == expected_bytes) return 1;
        else if (total_bytes > expected_bytes) return -1;

        int nfds = epoll_wait(epollfd, events, 1, 50);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {

                if ((bytes_read = read(sockfd, buf, MAXBUFSIZE)) == 0){
                    return -1;
                }
                FILE *fptr = fopen(fname ,"ab");

                total_bytes += bytes_read;
                fwrite(buf, 1, bytes_read, fptr);
                // printf("total bytes received: %d - expected: %d\n", total_bytes, expected_bytes);
                fclose(fptr); // Always close the file
                memset(buf, 0, MAXBUFSIZE);
            }
        }
    }

    if (total_bytes == expected_bytes) return 1;
    return -1;
}

/*
 * send_file_text_based() -- send binary image file in MAXBUFSIZE chunks (in binary mode)
 *
 * NOTE: Cannot rely on FILE OK marker, need to send file size first.
 */
int send_file_img_based(int sockfd, char *fname){
	printf("\nSending %s...\n", fname);
    FILE *fs = fopen(fname, "rb");

    if(fs == NULL){
        printf("ERROR: File %s not found.\n", fname);
        return -1;
    }

    long file_size = get_file_size(fs);
    printf("file size: %ld\n", file_size);
    char sdbuf[MAXBUFSIZE];

    if (strncmp(fname + (strlen(fname) - 3), "jpg", 3) > 0 ){
        printf("file extension invalid. IS: .%s NOT: .jpg\n", fname + (strlen(fname) - 3));
        return -1;
    }

    memset(sdbuf, 0, MAXBUFSIZE);
    int fs_block_sz;
    int bytes_sent;
    int total_bytes = 0;

    packi16(sdbuf, IMG_FILE);
    packi64(sdbuf+2, file_size);
    if ((bytes_sent = send(sockfd, sdbuf, 10, 0)) <= 0){
        fprintf(stderr, "ERROR: Failed to send file %s.\n", fname);
        return -1;
    }
    memset(sdbuf, 0, MAXBUFSIZE);
    
    while((fs_block_sz = fread(sdbuf, sizeof(char), MAXBUFSIZE, fs)) > 0)
    {
        if((bytes_sent = send(sockfd, sdbuf, fs_block_sz, 0)) < 0)
        {
            fprintf(stderr, "ERROR: Failed to send file %s.\n", fname);
            return -1;
        }
        total_bytes += bytes_sent;
        memset(sdbuf, 0, MAXBUFSIZE);
    }

    printf("File %s was Sent! - %d bytes\n\n", fname, total_bytes);
    return 1;
}

