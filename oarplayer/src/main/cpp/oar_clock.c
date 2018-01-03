/*
 The MIT License (MIT)

Copyright (c) 2017-2020 oarplayer(qingkouwei)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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