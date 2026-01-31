#include "./time_custom.h"


/*
 * get_time_ms() -- Returns the time in ms. Limited to 1 second timeframe (time_ms % 1)
 */
int get_time_ms(){
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    double ns = (double)time.tv_nsec;
    int ms = (ns / pow(10, 6));

    return ms; // value 0-999.999
}

/*
 * interval_lapsed() -- Compares two given times in milliseconds. Returns 1 if the difference is greater than the specified interval, otherwise 0
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>

int interval_elapsed(int t1, int t2, int interval){
    int diff = t2 - t1;

    if (diff < 0){
        diff += 1000;
    }

    return diff >= interval ? 1 : 0;
}

/*
 * interval_lapsed() -- Compares a timestamp to the current time in milliseconds. 
 * Returns 1 if the difference is greater than the specified interval, otherwise 0
 */
int interval_elapsed_cur(int t, int interval){
    int cur = get_time_ms();
    int diff = cur - t;

    if (diff < 0){
        diff += 1000;
    }

    return diff >= interval ? 1 : 0;
}