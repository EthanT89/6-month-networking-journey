#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

int is_all_digits(const char *str) {
    if (str == NULL || *str == '\0') return 0; // Handle null or empty string

    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return 0; // Found a non-digit character
        }
    }
    return 1; // All characters are digits
}

int main(int argc, char **argv) {

    if (argc != 2 || is_all_digits(argv[1]) != 1){
        printf("usage: ./create_workers [NUMWORKERS]\n");
        exit(1);
    } 


    int n = atoi(argv[1]);  // Number of server instances
    pid_t pid;

    for (int i = 0; i < n; i++) {
        pid = fork();

        if (pid == 0) {
            // Child process
            execl("./worker", "./worker", NULL);
            perror("execl failed");  // Only reached if execl fails
            exit(1);
        } else if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        // Parent continues to spawn next child
    }

    // Optional: Wait for all children to finish
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    printf("All server instances have finished.\n");
    return 0;
}   