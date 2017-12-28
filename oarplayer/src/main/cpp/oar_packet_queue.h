//
// Created by gutou on 2017/4/18.
//

#ifndef __OAR_PACKET_QUEUE_H
#define __OAR_PACKET_QUEUE_H

#include <pthread.h>
#include "oarplayer_type_def.h"


oar_packet_queue * oar_queue_create();
void oar_queue_set_duration(oar_packet_queue * queue, uint64_t max_duration);
void oar_packet_queue_free(oar_packet_queue *queue);
int oar_packet_queue_put(oar_packet_queue *queue,
                         int size,
                         PktType_e type,
                         int64_t dts,
                         int64_t pts,
                         int isKeyframe,
                         uint8_t *data);
OARPacket *  oar_packet_queue_get(oar_packet_queue *queue);
void freePacket(OARPacket *pkt);
//void xl_packet_queue_flush(oar_packet_queue *queue, xl_pakcet_pool *pool);
#endif //__OAR_PACKET_QUEUE_H
