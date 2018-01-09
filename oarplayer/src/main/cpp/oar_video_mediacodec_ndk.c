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
// Created by qingkouwei on 2018/1/5.
//

#define _JNILOG_TAG "oar_video_mediacodec_ndk"
#include "_android.h"
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "oar_video_mediacodec_ndk.h"
#include "oar_video_mediacodec_ctx.h"

static void formatCreate(void *ctx, void *format){
    oar_video_mediacodec_context *context = (oar_video_mediacodec_context*)ctx;
    context->AFormat = format;
}

void oar_create_video_mediacodec_ndk(
        oarplayer *oar){
    size_t sps_size, pps_size;
    uint8_t *sps_buf;
    uint8_t *pps_buf;
    sps_buf = (uint8_t *) malloc((size_t) oar->metadata->video_extradata_size + 20);
    pps_buf = (uint8_t *) malloc((size_t) oar->metadata->video_extradata_size + 20);
    if (0 != convert_sps_pps2(oar->metadata->video_extradata, (size_t) oar->metadata->video_extradata_size,
                              sps_buf, &sps_size, pps_buf, &pps_size, &oar->video_mediacodec_ctx->nal_size)) {
        LOGE("%s:convert_sps_pps: failed\n", __func__);
    }
    oar->video_mediacodec_ctx->ACodec = oar->dl_context->create_native_mediacodec(oar->metadata->video_codec,
                                                                                  oar->metadata->width, oar->metadata->height,
                                                                                  -1,
                                                                                  -1,
                                                                                  sps_buf, sps_size,
                                                                                  pps_buf, pps_size,
                                                                                  oar->video_mediacodec_ctx,
                                                                                  formatCreate);
    free(sps_buf);
    free(pps_buf);
}

void oar_video_mediacodec_release_buffer_ndk(oarplayer *oar, int index){
    oar->dl_context->native_mediacodec_release_buffer(oar->video_mediacodec_ctx->ACodec, index, true);
}
static void frameGenerate(void* player, void **frame, void *data, int size, int64_t pts,ssize_t index,int arg1, int arg2, int pix_format){
    oarplayer *oar = (oarplayer*)player;
    if (index >= 0) {
        OARFrame *f = (OARFrame *)(*frame);
        f->type = PktType_Video;
        f->pts = pts;
        f->format = PIX_FMT_EGL_EXT;
        f->width = oar->metadata->width;
        f->height = oar->metadata->height;
        f->HW_BUFFER_ID = index;
        f->next=NULL;
    }else{
        switch (index) {
            // AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED
            case -2: {
                oar->video_mediacodec_ctx->width = arg1;
                oar->video_mediacodec_ctx->height = arg2;
                oar->video_mediacodec_ctx->pix_format = pix_format;
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
int oar_video_mediacodec_receive_frame_ndk(oarplayer *oar, OARFrame *frame){
    return oar->dl_context->native_mediacodec_receive_frame(oar->video_mediacodec_ctx->ACodec,
                                                            &frame, oar, 0, frameGenerate);
}
int oar_video_mediacodec_send_packet_ndk(oarplayer *oar, OARPacket *packet){
    if (packet == NULL) { return -2; }
    if (oar->video_mediacodec_ctx->codec_id == VIDEO_CODEC_AVC) {
        H264ConvertState convert_state = {0, 0};
        convert_h264_to_annexb(packet->data, packet->size, oar->video_mediacodec_ctx->nal_size, &convert_state);
    }
    return oar->dl_context->native_mediacodec_send_packet(oar->video_mediacodec_ctx->ACodec,
                                                          packet->size,
                                                          packet->type,
                                                          packet->pts,
                                                          packet->pts,
                                                          packet->isKeyframe,
                                                          packet->data);
}
void oar_video_mediacodec_flush_ndk(oarplayer *oar){
    oar->dl_context->native_mediacodec_flush(oar->video_mediacodec_ctx->ACodec);
}
void oar_video_mediacodec_release_context_ndk(oarplayer *oar){
    oar->dl_context->native_mediacodec_release_context(oar->video_mediacodec_ctx->ACodec,oar->video_mediacodec_ctx->AFormat);
}
void oar_video_mediacodec_start_ndk(oarplayer *oar){
    while(oar->video_render_ctx->texture_window == NULL){
        usleep(10000);
    }
    oar->dl_context->native_mediacodec_start(oar->video_mediacodec_ctx->ACodec,
                                             oar->video_mediacodec_ctx->AFormat,
                                             oar->video_render_ctx->texture_window);
}
void oar_video_mediacodec_stop_ndk(oarplayer *oar){
    oar->dl_context->native_mediacodec_stop(oar->video_mediacodec_ctx->ACodec);
}

