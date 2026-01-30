
#include "./utils/time_custom.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>


int main(void){

    int t1;
    int t2;

    t1 = get_time_ms();

    for (int i = 0; i < 12; i++){
        usleep(10 * 1000);
        t2 = get_time_ms();
        printf("t1: %d\nt2: %d\ninterval elapsed: %d\n\n", t1, t2, interval_elapsed(t1, t2, 100));
    }
}