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
#define _JNILOG_TAG "video_decode_thread"
#include "_android.h"

#include <unistd.h>
#include <sys/prctl.h>
#include <malloc.h>
#include "oar_player_video_hw_decode_thread.h"
#include "oar_video_mediacodec_java.h"
#include "oar_packet_queue.h"
#include "oar_frame_queue.h"

#define _LOGD if(isDebug) LOGD

static inline int drop_video_packet(oarplayer * oar){
    OARPacket * packet = oar_packet_queue_get(oar->video_packet_queue);
    if(packet != NULL){
        int64_t time_stamp = packet->pts;
        int64_t diff = time_stamp - oar->audio_clock->pts;
        if(diff > 0){
            usleep((useconds_t) diff);
        }
        freePacket(packet);
        oar->video_mediacodec_ctx->oar_video_mediacodec_flush(oar);

    }else{
        usleep(NULL_LOOP_SLEEP_US);
    }
    return 0;
}

void* video_decode_hw_thread(void * data){
    prctl(PR_SET_NAME, __func__);
    oarplayer * oar = (oarplayer *)data;
    _LOGD("vm = %p, thread id = %d ,api = %d", oar->vm,gettid(),__ANDROID_API__);
    (*(oar->vm))->AttachCurrentThread(oar->vm, &oar->video_mediacodec_ctx->jniEnv, NULL);
    oar->video_mediacodec_ctx->oar_video_mediacodec_start(oar);
    int ret;
    OARPacket * packet = NULL;
    OARFrame * frame = (OARFrame*)malloc(sizeof(OARFrame));
    while (oar->error_code == 0) {
        if(oar->just_audio){
            // 如果只播放音频  按照音视频同步的速度丢包
            drop_video_packet(oar);
        }else{
            ret = oar->video_mediacodec_ctx->oar_video_mediacodec_receive_frame(oar, frame);
            _LOGD("video ret:%d", ret);
            if (ret == 0) {
                frame->FRAME_ROTATION = oar->frame_rotation;
                oar_frame_queue_put(oar->video_frame_queue, frame);
                frame = malloc(sizeof(OARFrame));
            }else if(ret == 1) {
                if(packet == NULL){
                    _LOGD("start get video packet...");
                    packet = oar_packet_queue_get(oar->video_packet_queue);
                    _LOGD("end get video packet...");
                }
                // buffer empty ==> wait  10ms
                // eof          ==> break
                if(packet == NULL){
                    _LOGD("video packet is null...");
                    usleep(BUFFER_EMPTY_SLEEP_US);
                    continue;
                }
                if(0 == oar->video_mediacodec_ctx->oar_video_mediacodec_send_packet(oar, packet)){
                    freePacket(packet);
                    packet = NULL;
                }else{
                    // some device AMediacodec input buffer ids count < frame_queue->size
                    // when pause   frame_queue not full
                    // thread will not block in  "xl_frame_queue_put" function
                    if(oar->status == PAUSED){
                        usleep(NULL_LOOP_SLEEP_US);
                    }
                }

            }else if(ret == -2) {
                //frame = xl_frame_pool_get_frame(pd->video_frame_pool);
            }else {
                oar->on_error(oar, OAR_ERROR_VIDEO_HW_MEDIACODEC_RECEIVE_FRAME);
                break;
            }
        }
    }
    drop_video_packet(oar);//Avoid srs read thread waiting cannot stopped.
    oar->video_mediacodec_ctx->oar_video_mediacodec_stop(oar);
    (*oar->vm)->DetachCurrentThread(oar->vm);
    LOGI("thread ==> %s exit", __func__);
    return NULL;
}