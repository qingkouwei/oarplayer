
#include <unistd.h>
#include <sys/prctl.h>
#include <malloc.h>
#include "oar_player_video_hw_decode_thread.h"
#include "oar_video_mediacodec.h"
#include "oar_packet_queue.h"
#include "oar_frame_queue.h"
#define _JNILOG_TAG "video_decode_thread"
#include "_android.h"

static inline int drop_video_packet(oarplayer * oar){
    OARPacket * packet = oar_packet_queue_get(oar->video_packet_queue);
    if(packet != NULL){
        int64_t time_stamp = packet->pts;
        int64_t diff = time_stamp - oar->audio_clock->pts;
        if(diff > 0){
            usleep((useconds_t) diff);
        }
        freePacket(packet);
        oar_video_mediacodec_flush(oar);

    }else{
        if(oar->eof){
            return -1;
        }
        usleep(NULL_LOOP_SLEEP_US);
    }
    return 0;
}

void* video_decode_hw_thread(void * data){
    prctl(PR_SET_NAME, __func__);
    oarplayer * oar = (oarplayer *)data;
    LOGE("vm = %p, thread id = %d ,api = %d", oar->vm,gettid(),__ANDROID_API__);
    (*(oar->vm))->AttachCurrentThread(oar->vm, &oar->video_mediacodec_ctx->jniEnv, NULL);
    oar_video_mediacodec_start(oar);
    int ret;
    OARPacket * packet = NULL;
    OARFrame * frame = (OARFrame*)malloc(sizeof(OARFrame));//TODO
    while (oar->error_code == 0) {
        if(oar->just_audio){
            // 如果只播放音频  按照音视频同步的速度丢包
            if( -1 == drop_video_packet(oar)){
                break;
            }
        }else{
            ret = oar_video_mediacodec_receive_frame(oar, frame);
            LOGE("video ret:%d", ret);
            if (ret == 0) {
                frame->FRAME_ROTATION = oar->frame_rotation;
                oar_frame_queue_put(oar->video_frame_queue, frame);
                frame = malloc(sizeof(OARFrame));
            }else if(ret == 1) {
                if(packet == NULL){
                    LOGE("start get video packet...");
                    packet = oar_packet_queue_get(oar->video_packet_queue);
                    LOGE("end get video packet...");
                }
                // buffer empty ==> wait  10ms
                // eof          ==> break
                if(packet == NULL){
                    LOGE("video packet is null...");
                    if(oar->eof){
                        break;
                    }else{
                        usleep(BUFFER_EMPTY_SLEEP_US);
                        continue;
                    }
                }
                if(0 == oar_video_mediacodec_send_packet(oar, packet)){
                    LOGE("send packet success...");
                    freePacket(packet);
                    packet = NULL;
                }else{
                    // some device AMediacodec input buffer ids count < frame_queue->size
                    // when pause   frame_queue not full
                    // thread will not block in  "xl_frame_queue_put" function
                    LOGE("send packet failed:11111");
                    if(oar->status == PAUSED){
                        LOGE("send packet failed:2222222");
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
    oar_video_mediacodec_stop(oar);
    (*oar->vm)->DetachCurrentThread(oar->vm);
    LOGI("thread ==> %s exit", __func__);
    return NULL;
}