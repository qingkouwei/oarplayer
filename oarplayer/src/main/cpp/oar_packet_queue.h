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
#endif //__OAR_PACKET_QUEUE_H
