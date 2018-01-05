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
//
// Created by qingkouwei on 2018/1/4.
//

#ifndef OARPLAYER_OAR_NATIVE_VIDEO_MEDIACODEC_H
#define OARPLAYER_OAR_NATIVE_VIDEO_MEDIACODEC_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

int oar_native_mediacodec_send_packet(void * codec,
                                      int len,
                                      int type,
                                      int64_t dts,
                                      int64_t pts,
                                      int isKeyframe,
                                      uint8_t *data);
void oar_native_mediacodec_release_buffer(void * codec, int bufferID, bool render);
int oar_native_mediacodec_receive_frame(void * codec,
                                        void** frame,
                                        void *oar,
                                        int type,
                                        void *(frameGenerate)(void*,void**, void*, int, int64_t,ssize_t,int, int, int));
void oar_native_mediacodec_flush(void * codec);
void *oar_create_native_mediacodec(int codec_id,
                                           int width, int height,
                                           int sample_rate, int channelCount,
                                           uint8_t *sps, int sps_size,
                                           uint8_t *pps, int pps_size,
                                           void *ctx,
                                           void (*formatCreated(void*, void*)));
int oar_native_mediacodec_start(void * codec, void *format, void *window);
void oar_native_mediacodec_stop(void * codec);
void oar_native_mediacodec_release_context(void * codec, void *format);
#endif //OARPLAYER_OAR_NATIVE_VIDEO_MEDIACODEC_H
