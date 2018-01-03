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
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include "oar_frame_queue.h"
#define _JNILOG_TAG "oar_frame_queue"
#include "_android.h"

/*static OARFrame *newFrame(const uint8_t *data, int size, PktType_e type)
{
    OARFrame *frame = (OARFrame *)calloc(1, sizeof(OARFrame) + size);
    if (!frame) {
        LOGE("failed in malloc OARPacket");
        return NULL;
    }
    if (data) {
        memcpy(frame->data, data, size);
        frame->size = size;
    }
    else {
        frame->size = 0;
    }

    frame->type = type;
    return frame;
}

void freeFrame(OARFrame *frame)
{
    //LOGI("freeFrame ");
    if (frame) {
        free(frame);
        //pktCreatedCount --;
        //LOGI("free, pktCreatedCount: %d", pktCreatedCount);
    }
}*/
oar_frame_queue * oar_frame_queue_create(unsigned int size){
    oar_frame_queue * queue = (oar_frame_queue *)malloc(sizeof(oar_frame_queue));
    queue->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    queue->cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(queue->mutex, NULL);
    pthread_cond_init(queue->cond, NULL);
    queue->count = 0;
    queue->cachedFrames = NULL;
    queue->lastFrame = NULL;
    queue->size = size;
    return queue;
}
void oar_frame_queue_free(oar_frame_queue *queue){
    pthread_mutex_destroy(queue->mutex);
    pthread_cond_destroy(queue->cond);
    OARFrame *tempFrame,*frame = queue->cachedFrames;
    while(frame){
        tempFrame = frame;
        frame = frame->next;
        free(tempFrame);
    }
    queue->cachedFrames = NULL;
    queue->lastFrame = NULL;
    free(queue);
}

int oar_frame_queue_put(oar_frame_queue *queue, OARFrame *f){
    //LOGE("start oar_frame_queue_put:%d", queue->count);
    pthread_mutex_lock(queue->mutex);
    while(queue->count == queue->size){
        if(f->type == PktType_Audio){
            LOGE("frame queue wait...");
        }

        pthread_cond_wait(queue->cond, queue->mutex);
    }
    if (queue->lastFrame) {
        queue->lastFrame->next = f;
        queue->lastFrame = f;
    }
    else {
        queue->lastFrame = queue->cachedFrames = f;
    }
    queue->count++;
    //LOGE("oar_frame_queue_put:%d", queue->count);
    pthread_mutex_unlock(queue->mutex);
    return 0;
}

OARFrame *  oar_frame_queue_get(oar_frame_queue *queue){
    //LOGE("start oar_frame_queue_get:%d", queue->count);
    pthread_mutex_lock(queue->mutex);
    if (queue->count == 0) {
        pthread_mutex_unlock(queue->mutex);
        return NULL;
    }
    OARFrame *ret = NULL;
    if (queue->cachedFrames) {
        ret = queue->cachedFrames;
        if (queue->cachedFrames == queue->lastFrame) {
            queue->lastFrame = NULL;
        }
        queue->cachedFrames = queue->cachedFrames->next;
        if (queue->count > 0) {
            queue->count--;
        }
    }
    else {
        pthread_mutex_unlock(queue->mutex);
        return NULL;
    }
    pthread_cond_signal(queue->cond);
    pthread_mutex_unlock(queue->mutex);
//    LOGI("pts=%lld", ret->pts);
    return ret;
}

