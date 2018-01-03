

#include <malloc.h>
#include <string.h>
#include "oar_packet_queue.h"

#define _JNILOG_TAG "oar_packet_queue"
#include "_android.h"
static OARPacket *newPacket(const uint8_t *data, int size, PktType_e type)
{
    OARPacket *pkt = (OARPacket *)calloc(1, sizeof(OARPacket) + size);
    if (!pkt) {
        LOGE("failed in malloc OARPacket");
        return NULL;
    }
    if (data) {
        memcpy(pkt->data, data, size);
        pkt->size = size;
    }
    else {
        pkt->size = 0;
    }

    pkt->type = type;
    pkt->next = NULL;
    return pkt;
}

void freePacket(OARPacket *pkt)
{
    if (pkt) {
        free(pkt);
    }
}

oar_packet_queue * oar_queue_create(){
    oar_packet_queue *queue = (oar_packet_queue*)malloc(sizeof(oar_packet_queue));
    queue->mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    queue->cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(queue->mutex, NULL);
    pthread_cond_init(queue->cond, NULL);
    queue->count = 0;
    queue->max_duration = 0;
    queue->total_bytes = 0;
    queue->cachedPackets = NULL;
    queue->lastPacket = NULL;
    queue->full_cb = NULL;
    queue->empty_cb = NULL;

    return queue;
}
void oar_queue_set_duration(oar_packet_queue* queue, uint64_t max_duration){
    queue->max_duration = max_duration;
}
void oar_packet_queue_free(oar_packet_queue *queue){
    pthread_mutex_destroy(queue->mutex);
    pthread_cond_destroy(queue->cond);
    OARPacket *tempPacket,*packet = queue->cachedPackets;
    while(packet){
        tempPacket = packet;
        packet = packet->next;
        free(tempPacket);
    }
    queue->cachedPackets = NULL;
    queue->lastPacket = NULL;
    free(queue);
}
int oar_packet_queue_put(oar_packet_queue *queue,
                         int size,
                         PktType_e type,
                         int64_t dts,
                         int64_t pts,
                         int isKeyframe,
                         uint8_t *data){
    int64_t cachedDur = 0;
    pthread_mutex_lock(queue->mutex);
    if (queue->cachedPackets && queue->lastPacket) {
        LOGE("type = %d;last:%lld, %p, cached:%lld, %p", type, queue->lastPacket->dts, queue->cachedPackets, queue->cachedPackets->dts, queue->lastPacket);
        cachedDur = queue->lastPacket->dts - queue->cachedPackets->dts;
        LOGE("type = %d; duration:%lld" , type, cachedDur);
        if (cachedDur >= queue->max_duration * USEC_PER_SEC) {
            if (queue->full_cb != NULL) {
                queue->full_cb(queue->cb_data);
            }
//            LOGE("type = %d; oar_packet_queue_put wait...", type);
            pthread_cond_wait(queue->cond, queue->mutex);
//            LOGE("type = %d; oar_packet_queue_put wait end", type);
        }
    }
    pthread_mutex_unlock(queue->mutex);

    OARPacket *p = newPacket(data, size, type);
    if (!p) {
        LOGE("failed in newPacket");
        return -1;
    }

    p->dts = dts;
    p->pts = pts;
    p->isKeyframe = isKeyframe;

    pthread_mutex_lock(queue->mutex);
    //LOGE("last = %p", queue->lastPacket);
    if (queue->lastPacket) {
        queue->lastPacket->next = p;
        queue->lastPacket = p;
    }
    else {
        queue->lastPacket = queue->cachedPackets = p;
    }
    queue->count++;
    queue->total_bytes += size;
    pthread_cond_signal(queue->cond);
    pthread_mutex_unlock(queue->mutex);
    return 0;
}
OARPacket*  oar_packet_queue_get(oar_packet_queue *queue){
//    LOGI("oar_packet_queue_get start");
    pthread_mutex_lock(queue->mutex);
//    LOGI("oar_packet_queue_get count: %d", queue->count);
    if (queue->count == 0) {
        pthread_cond_signal(queue->cond);
        pthread_mutex_unlock(queue->mutex);
        if (queue->empty_cb != NULL) {
            queue->empty_cb(queue->cb_data);
        }
        return NULL;
    }
//    LOGI("oar_packet_queue_get");
    OARPacket *ret = NULL;
    if (queue->cachedPackets) {
        ret = queue->cachedPackets;
        if (queue->cachedPackets == queue->lastPacket) {
            queue->lastPacket = NULL;
        }
        queue->cachedPackets = queue->cachedPackets->next;
        if (queue->count > 0) {
            queue->count --;
        }
    }
    else {
        pthread_cond_signal(queue->cond);
        pthread_mutex_unlock(queue->mutex);
        if (queue->empty_cb != NULL) {
            queue->empty_cb(queue->cb_data);
        }
        return NULL;
    }
    queue->total_bytes -= ret->size;
//    LOGE("type = %d,oar_packet_queue_get cond signal", ret->type);
    pthread_cond_signal(queue->cond);
    pthread_mutex_unlock(queue->mutex);
    return ret;
}