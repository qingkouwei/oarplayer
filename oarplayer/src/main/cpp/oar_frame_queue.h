//
// Created by gutou on 2017/5/8.
//

#ifndef __OAR_FRAME_QUEUE_H_
#define __OAR_FRAME_QUEUE_H_
#include "oarplayer_type_def.h"
oar_frame_queue * oar_frame_queue_create(unsigned int size);
void oar_frame_queue_free(oar_frame_queue *queue);
int oar_frame_queue_put(oar_frame_queue *queue, OARFrame *frame);
OARFrame *  oar_frame_queue_get(oar_frame_queue *queue);
void freeFrame(OARFrame *frame);
//void xl_frame_queue_flush(xl_frame_queue *queue, xl_frame_pool *pool);
#endif //__OAR_FRAME_QUEUE_H_
