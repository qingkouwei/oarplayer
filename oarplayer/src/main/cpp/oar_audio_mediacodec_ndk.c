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
#define _JNILOG_TAG "oar_audio_mediacodec_ndk"
#include "_android.h"
#include <malloc.h>
#include <string.h>
#include "oar_audio_mediacodec_ndk.h"
#define isDebug 1
#define _LOGD if(isDebug) LOGI
static void formatCreate(void *ctx, void *format){
    oar_audio_mediacodec_context *context = (oar_audio_mediacodec_context*)ctx;
    context->AFormat = format;
}

void oar_create_audio_mediacodec_ndk(
        oarplayer *oar){
    oar->audio_mediacodec_ctx->ACodec = oar->dl_context->create_native_mediacodec(oar->metadata->audio_codec,
                                                      0, 0,
                                                      oar->metadata->sample_rate,
                                                      oar->metadata->channels,
                                                      oar->metadata->audio_pps, oar->metadata->audio_pps_size,
                                                      NULL, 0,
                                                      oar->audio_mediacodec_ctx,
                                                      formatCreate);
}

void oar_audio_mediacodec_release_buffer_ndk(oarplayer *oar, int index){
    oar->dl_context->native_mediacodec_release_buffer(oar->audio_mediacodec_ctx->ACodec, index, false);
}
static void frameGenerate(void* player, void **frame, void *data, int size, int64_t pts,ssize_t index,int arg1, int arg2, int pix_format){
    oarplayer *oar = (oarplayer*)player;
    if (index >= 0) {
        OARFrame *f = (OARFrame *)malloc(sizeof(OARFrame) + size);
        if (data != NULL){
            memcpy(f->data, data, size);
        }
        f->type = PktType_Audio;
        f->size = size;
        f->pts = pts;
        f->HW_BUFFER_ID = index;
        f->next=NULL;
        *frame = f;
    }else{
        switch (index) {
            // AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED
            case -2: {
                oar->audio_mediacodec_ctx->sample_rate = arg1;
                oar->audio_mediacodec_ctx->channel_count = arg2;
                break;
            }
                // AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED
            case -3:
                break;
                // AMEDIACODEC_INFO_TRY_AGAIN_LATER
            case -1:
                break;
            default:
                break;
        }
    }
}
int oar_audio_mediacodec_receive_frame_ndk(oarplayer *oar, OARFrame **frame){
    return oar->dl_context->native_mediacodec_receive_frame(oar->audio_mediacodec_ctx->ACodec,
                                                            frame, oar, 1, frameGenerate);
}
int oar_audio_mediacodec_send_packet_ndk(oarplayer *oar, OARPacket *packet){
    if (packet == NULL) { return -2; }
    return oar->dl_context->native_mediacodec_send_packet(oar->audio_mediacodec_ctx->ACodec,
                                                          packet->size,
                                                          packet->type,
                                                          packet->pts,
                                                          packet->pts,
                                                          packet->isKeyframe,
                                                          packet->data);
}
void oar_audio_mediacodec_flush_ndk(oarplayer *oar){
    oar->dl_context->native_mediacodec_flush(oar->audio_mediacodec_ctx->ACodec);
}
void oar_audio_mediacodec_release_context_ndk(oarplayer *oar){
    oar->dl_context->native_mediacodec_release_context(oar->audio_mediacodec_ctx->ACodec,oar->audio_mediacodec_ctx->AFormat);
}
void oar_audio_mediacodec_start_ndk(oarplayer *oar){
    oar->dl_context->native_mediacodec_start(oar->audio_mediacodec_ctx->ACodec, oar->audio_mediacodec_ctx->AFormat, NULL);
}
void oar_audio_mediacodec_stop_ndk(oarplayer *oar){
    oar->dl_context->native_mediacodec_stop(oar->audio_mediacodec_ctx->ACodec);
}
