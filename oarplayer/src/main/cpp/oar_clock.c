

#include <sys/time.h>
#include <malloc.h>
#include "oar_clock.h"
uint64_t oar_clock_get_current_time(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}
oar_clock * oar_clock_create(){
    oar_clock *clock = (oar_clock*)malloc(sizeof(oar_clock));
    return clock;
}
int64_t oar_clock_get(oar_clock * clock){
    if(clock->update_time == 0){
        return INT64_MAX;
    }
    return clock->pts + oar_clock_get_current_time()-clock->update_time;
}
void oar_clock_set(oar_clock * clock, int64_t pts){
    clock->update_time = oar_clock_get_current_time();
    clock->pts = pts;
}
void oar_clock_free(oar_clock * clock){
    free(clock);
}
void oar_clock_reset(oar_clock * clock){
    clock->pts=0;
    clock->update_time = 0;
}