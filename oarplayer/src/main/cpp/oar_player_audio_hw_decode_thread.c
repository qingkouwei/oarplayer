
#include <unistd.h>
#include <sys/prctl.h>
#include <malloc.h>
#include "oar_player_audio_hw_decode_thread.h"
#include "oar_packet_queue.h"
#include "oar_frame_queue.h"
#define _JNILOG_TAG "audio_decode_thread"
#include "_android.h"
#include "oar_audio_mediacodec.h"

void * audio_decode_thread(void * data){
    prctl(PR_SET_NAME, __func__);
    oarplayer * oar = (oarplayer *)data;
    (*(oar->vm))->AttachCurrentThread(oar->vm, &oar->audio_mediacodec_ctx->jniEnv, NULL);
    oar_audio_mediacodec_start(oar);
    int ret;
    OARPacket * packet = NULL;
    OARFrame *frame;
    while (oar->error_code == 0) {
        if(oar->status == PAUSED){
            usleep(NULL_LOOP_SLEEP_US);
        }

        ret = oar_audio_mediacodec_receive_frame(oar, &frame);
//        LOGE("audio ret:%d",ret);
        if (ret == 0) {
            oar_frame_queue_put(oar->audio_frame_queue, frame);
            // 触发音频播放
            if (oar->metadata->has_audio){
                oar->audio_player_ctx->play(oar);
            }
        } else if (ret == 1) {
//            LOGI("start read audio...");
            packet = oar_packet_queue_get(oar->audio_packet_queue);
            // buffer empty ==> wait  10ms
            // eof          ==> break
            if(packet == NULL){
                if(oar->eof){
                    break;
                }else{
//                    LOGE("audio deocdec sleep...");
                    usleep(BUFFER_EMPTY_SLEEP_US);
                    continue;
                }
            }
            ret = oar_audio_mediacodec_send_packet(oar, packet);
            if(ret == 0){
                freePacket(packet);
                packet = NULL;
            }else{
                if(oar->status == PAUSED){
                    usleep(NULL_LOOP_SLEEP_US);
                }
            }
        } else if (ret == -2) {
//            oar->on_error(oar, OAR_ERROR_AUDIO_DECODE_CODEC_NOT_OPENED);
//            break;
        } else {
            oar->on_error(oar, OAR_ERROR_AUDIO_DECODE_RECIVE_FRAME);
            break;
        }
    }
    oar_audio_mediacodec_stop(oar);
    (*oar->vm)->DetachCurrentThread(oar->vm);
    LOGI("thread ==> %s exit", __func__);
    return NULL;
}