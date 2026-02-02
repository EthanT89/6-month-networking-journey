/*
 * time_custom.h -- Time and clock related util functions
 */

#ifndef TIME_CUSTOM_H
#define TIME_CUSTOM_H

#include <time.h>
#include <math.h>

/*
 * get_time_ms() -- Returns the time in ms. Limited to 1 second timeframe (time_ms % 1)
 */
int get_time_ms();

/*
 * interval_lapsed() -- Compares two given times in milliseconds. Returns 1 if the difference is greater than the specified interval, otherwise 0
 */
int interval_elapsed(int t1, int t2, int interval);

/*
 * interval_lapsed() -- Compares a timestamp to the current time in milliseconds. 
 * Returns 1 if the difference is greater than the specified interval, otherwise 0
 */
int interval_elapsed_cur(int t, int interval);

#endif
