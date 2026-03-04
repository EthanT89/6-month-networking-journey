#include "./file_transfer.h"

long get_file_size(FILE *file) {
    long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET); // Reset to beginning
    return size;
}

void receive_file(char *fname, int sockfd){
    int total_bytes = 0;
    int expected_bytes;
    int epollfd = create_epoll();
    struct epoll_event events[1];

    char buf[MAXBUFSIZE];
    add_epoll_fd(epollfd, sockfd);
 
    while (1) {
        int nfds = epoll_wait(epollfd, events, 1, 50);
        if (nfds == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                int bytes_read;

                if ((bytes_read = read(sockfd, buf, MAXBUFSIZE)) == 0){
                    return;
                }
                FILE *fptr = fopen(fname ,"a");

                if (bytes_read > 7 && strncmp(buf+(bytes_read - 8), "FILE OK", 8) == 0){
                    buf[bytes_read - 8] = '\0';
                    fprintf(fptr, "%s", buf);   
                    fclose(fptr); // Always close the file
                    memset(buf, 0, MAXBUFSIZE);
                    return;
                }
                total_bytes += bytes_read;
                fprintf(fptr, "%s", buf);   
                fclose(fptr); // Always close the file
                memset(buf, 0, MAXBUFSIZE);
            }
        }
    }
}

int send_file(int sockfd, char *file_name){
	printf("\nSending %s...\n", file_name);
    FILE *fs = fopen(file_name, "r");
    long file_size = get_file_size(fs);
    char sdbuf[MAXBUFSIZE]; 

    if(fs == NULL){
        printf("ERROR: File %s not found.\n", file_name);
        exit(1);
    }

    memset(sdbuf, 0, MAXBUFSIZE);
    int fs_block_sz;
    int bytes_sent;
    int total_bytes = 0;
    while((fs_block_sz = fread(sdbuf, sizeof(char), MAXBUFSIZE, fs)) > 0)
    {
        if((bytes_sent = send(sockfd, sdbuf, fs_block_sz, 0)) < 0)
        {
            fprintf(stderr, "ERROR: Failed to send file %s.\n", file_name);
            exit(1);
        }
        total_bytes += bytes_sent;
        memset(sdbuf, 0, MAXBUFSIZE);
    }

    memset(sdbuf, 0, MAXBUFSIZE);
    strcpy(sdbuf, "FILE OK");
    if(send(sockfd, sdbuf, 8, 0) < 0)
    {
        fprintf(stderr, "ERROR: Failed to send file %s.\n", file_name);
        exit(1);
    }

    printf("File %s was Sent! - %d bytes\n\n", file_name, total_bytes);
    return 1;
}