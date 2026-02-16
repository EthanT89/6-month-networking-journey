#include "./time_custom.h"


/*
 * get_time_ms() -- Returns the time in ms (includes seconds). Suitable for sessions up to 1 hour.
 */
int get_time_ms(){
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    int ms = (time.tv_sec * 1000) + (time.tv_nsec / 1000000);

    return ms;
}

/*
 * interval_lapsed() -- Compares two given times in milliseconds. Returns 1 if the difference is greater than the specified interval, otherwise 0
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>

int interval_elapsed(int t1, int t2, int interval){
    int diff = t2 - t1;

    return diff >= interval ? 1 : 0;
}

/*
 * interval_lapsed() -- Compares a timestamp to the current time in milliseconds. 
 * Returns 1 if the difference is greater than the specified interval, otherwise 0
 */
int interval_elapsed_cur(int t, int interval){
    int cur = get_time_ms();
    int diff = cur - t;

    return diff >= interval ? 1 : 0;
}

int get_diff_ms(int t1, int t2){
    int diff = t1 - t2;

    return diff;
}