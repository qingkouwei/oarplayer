

#include <malloc.h>
#include <unistd.h>
#include "oar_player.h"
#include "oar_clock.h"
#include "oar_jni_reflect.h"
#include "oar_packet_queue.h"

#define _JNILOG_TAG "oar_player"

#include "_android.h"
#include "srs_readthread.h"
#include "oar_video_render.h"
#include "oar_player_video_hw_decode_thread.h"
#include "oar_video_mediacodec.h"
#include "oar_player_gl_thread.h"
#include "oar_frame_queue.h"
#include "oar_player_audio_hw_decode_thread.h"
#include "oar_audio_player.h"
#include "oar_audio_mediacodec.h"

static int stop(oarplayer *oar);

static void on_error(oarplayer *oar);

static void change_status(oarplayer *pd, PlayStatus status);

static void on_decoder_configuration(oarplayer *oar);

static void send_message(oarplayer *oar, int message) {
    int sig = message;
    write(oar->pipe_fd[1], &sig, sizeof(int));
}
static int message_callback(int fd, int events, void *data) {
    oarplayer *oar = data;
    int message;
    for (int i = 0; i < events; i++) {
        read(fd, &message, sizeof(int));
        LOGI("recieve message ==> %d", message);
        switch (message) {
            case oar_message_stop:
                stop(oar);
                break;
            case oar_message_buffer_empty:
                change_status(oar, BUFFER_EMPTY);
                break;
            case oar_message_buffer_full:
                change_status(oar, BUFFER_FULL);
                break;
            case oar_message_error:
                on_error(oar);
                break;
            case oar_message_decoder_configuration:
                on_decoder_configuration(data);
                break;
            default:
                break;
        }
    }
    return 1;
}
static void on_error_cb(oarplayer *pd, int error_code) {
    pd->error_code = error_code;
    pd->send_message(pd, oar_message_error);
}

static void buffer_empty_cb(void *data) {
    oarplayer *oar = data;
    if (oar->status != BUFFER_EMPTY && !oar->eof) {
        oar->send_message(oar, oar_message_buffer_empty);
    }
}

static void buffer_full_cb(void *data) {
    oarplayer *oar = data;
    if (oar->status == BUFFER_EMPTY) {
        oar->send_message(oar, oar_message_buffer_full);
    }
}

static void reset(oarplayer *oar) {
    if (oar == NULL) return;
    oar->eof = false;
    oar->av_track_flags = 0;
    oar->just_audio = false;
    oar->video_index = -1;
    oar->audio_index = -1;
    oar->url = NULL;
    oar->audio_frame = NULL;
    oar->video_frame = NULL;
    oar->timeout_start = 0;
    oar_clock_reset(oar->audio_clock);
    oar_clock_reset(oar->video_clock);
    oar->error_code = 0;
    oar->frame_rotation = OAR_ROTATION_0;
    oar->change_status(oar, IDEL);
    oar_video_render_ctx_reset(oar->video_render_ctx);
}

static inline void set_buffer_time(oarplayer *pd) {
    float buffer_time_length = pd->buffer_time_length;
    if (pd->metadata->has_audio) {
        oar_queue_set_duration(pd->audio_packet_queue,buffer_time_length);
        pd->audio_packet_queue->empty_cb = buffer_empty_cb;
        pd->audio_packet_queue->full_cb = buffer_full_cb;
        pd->audio_packet_queue->cb_data = pd;
    }
    if (pd->metadata->has_video) {
        oar_queue_set_duration(pd->video_packet_queue,buffer_time_length);
        pd->video_packet_queue->empty_cb = buffer_empty_cb;
        pd->video_packet_queue->full_cb = buffer_full_cb;
        pd->video_packet_queue->cb_data = pd;
    }
}

oarplayer *
oar_player_create(JNIEnv *env, jobject instance, int run_android_version, int best_samplerate){
    oarplayer *oar = (oarplayer *)malloc(sizeof(oarplayer));
    oar->jniEnv = env;
    (*env)->GetJavaVM(env, &oar->vm);
    oar->oarPlayer=(*oar->jniEnv)->NewGlobalRef(oar->jniEnv, instance);

    oar_jni_reflect_java_class(&oar->jc, oar->jniEnv);

    oar->run_android_version = run_android_version;
    oar->best_samplerate = best_samplerate;
    oar->buffer_size_max = default_buffer_size;
    oar->buffer_time_length = default_buffer_time;
    oar->force_sw_decode = false;
    oar->is_sw_decode = false;
    oar->read_timeout = default_read_timeout;


    oar->metadata = (oar_metadata_t*)malloc(sizeof(oar_metadata_t));
    oar->metadata->has_audio = 0;
    oar->metadata->has_video = 0;

    oar->audio_packet_queue = oar_queue_create();
    oar->video_packet_queue = oar_queue_create();
    oar->video_frame_queue = oar_frame_queue_create(20);
    oar->audio_frame_queue = oar_frame_queue_create(20);
    oar->audio_clock = oar_clock_create();
    oar->video_clock = oar_clock_create();
    oar->video_render_ctx = oar_video_render_ctx_create();
    oar->audio_player_ctx = oar_audio_engine_create();

    oar->main_looper = ALooper_forThread();
    pipe(oar->pipe_fd);
    if (1 !=
        ALooper_addFd(oar->main_looper, oar->pipe_fd[0], ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT,
                      message_callback, oar)) {
        LOGE("error. when add fd to main looper");
    }
    oar->change_status = change_status;
    oar->send_message = send_message;
    oar->on_error = on_error_cb;
    reset(oar);
    //TODO
    return oar;
}
void oar_player_set_buffer_time(oarplayer *oar, float buffer_time){
    oar->buffer_time_length = buffer_time;
    if (oar->status != IDEL) {
        set_buffer_time(oar);
    }
}
void oar_player_set_buffer_size(oarplayer *oar, int buffer_size){
    oar->buffer_size_max = buffer_size;
}

int oar_player_play(oarplayer *oar){
    oar->error_code = 0;
    pthread_create(&oar->read_stream_thread, NULL, read_thread, oar);

    return 0;
}
static inline void clean_queues(oarplayer *oar) {
    // clear pd->audio_frame audio_frame_queue  audio_packet_queue
    if ((oar->av_track_flags & OAR_HAS_AUDIO_FLAG) > 0) {
        //TODO
    }
    // clear pd->video_frame video_frame_queue video_frame_packet
    if ((oar->av_track_flags & OAR_HAS_VIDEO_FLAG) > 0) {
        //TODO
    }
}
static int stop(oarplayer *oar) {
    if(oar->url){
        free(oar->url);
    }
    if(oar->metadata){
        oar->metadata->has_video = 0;
        oar->metadata->has_audio = 0;
    }

    // remove buffer call back
    oar->audio_packet_queue->empty_cb = NULL;
    oar->audio_packet_queue->full_cb = NULL;
    oar->video_packet_queue->empty_cb = NULL;
    oar->video_packet_queue->full_cb = NULL;

    clean_queues(oar);
    // 停止各个thread
    void *thread_res;
    pthread_join(oar->read_stream_thread, &thread_res);
    if ((oar->av_track_flags & OAR_HAS_VIDEO_FLAG) > 0) {
        pthread_join(oar->video_decode_thread, &thread_res);
        if (oar->is_sw_decode) {
            //avcodec_free_context(&pd->video_codec_ctx);
        } else {
            oar_video_mediacodec_release_context(oar);
        }
        pthread_join(oar->gl_thread, &thread_res);
    }

    if ((oar->av_track_flags & OAR_HAS_AUDIO_FLAG) > 0) {
        pthread_join(oar->audio_decode_thread, &thread_res);
        //oar->audio_player_ctx->shutdown();
    }

    clean_queues(oar);
    reset(oar);
    LOGI("player stoped");
    return 0;
}
int oar_player_stop(oarplayer *oar){
    if (oar == NULL || oar->status == IDEL) return 0;
    oar->error_code = -1;
    return stop(oar);
}
int oar_player_release(oarplayer *oar){
    if (oar->status != IDEL) {
        oar_player_stop(oar);
    }
    while (oar->status != IDEL) {
        usleep(10000);
    }
    ALooper_removeFd(oar->main_looper, oar->pipe_fd[0]);
    close(oar->pipe_fd[1]);
    close(oar->pipe_fd[0]);
    oar_frame_queue_free(oar->audio_frame_queue);
    oar_frame_queue_free(oar->video_frame_queue);
    oar_packet_queue_free(oar->audio_packet_queue);
    oar_packet_queue_free(oar->video_packet_queue);
    oar_clock_free(oar->audio_clock);
    oar_clock_free(oar->video_clock);
    oar_video_render_ctx_release(oar->video_render_ctx);
    oar->audio_player_ctx->release(oar->audio_player_ctx);

    if(oar->metadata){
        if(oar->metadata->audio_pps){
            free(oar->metadata->audio_pps);
        }
        if(oar->metadata->video_extradata){
            free(oar->metadata->video_extradata);
        }
        free(oar->metadata);
        oar->metadata = NULL;
    }

    (*oar->jniEnv)->DeleteGlobalRef(oar->jniEnv, oar->oarPlayer);
    oar_jni_free(&oar->jc, oar->jniEnv);
    free(oar);
    return 0;
}
static void change_status(oarplayer *pd, PlayStatus status) {
    if (status == BUFFER_FULL) {
        //oar_player_resume(pd);
        pd->status = PLAYING;
    } else {
        pd->status = status;
    }
    (*pd->jniEnv)->CallVoidMethod(pd->jniEnv, pd->oarPlayer, pd->jc->player_onPlayStatusChanged,
                                  status);
}

static void on_error(oarplayer *pd) {
    (*pd->jniEnv)->CallVoidMethod(pd->jniEnv, pd->oarPlayer, pd->jc->player_onPlayError,
                                  pd->error_code);
}
static int hw_codec_init(oarplayer *oar) {
    oar->video_mediacodec_ctx = oar_create_video_mediacodec_context(oar);
    return 0;
}
static int audio_codec_init(oarplayer *oar) {

    oar->audio_mediacodec_ctx = oar_create_audio_mediacodec_context(oar);
    // Android openSL ES   can not support more than 2 channels.
    // flv acc sample_rate is constant 3(44100)
    oar->audio_player_ctx->player_create(oar->metadata->sample_rate, oar->metadata->channels, oar);
    return 0;
}
static void on_decoder_configuration(oarplayer *oar){
    LOGI("on_decoder_configuration :video bitrate = %d, fps = %d, width = %d, height = %d; audio bitrate = %d",
    oar->metadata->video_bitrate,oar->metadata->fps,oar->metadata->width,
         oar->metadata->height, oar->metadata->audio_bitrate);
    set_buffer_time(oar);
    int ret;
    /*if (oar->is_sw_decode) {
        ret = sw_codec_init(oar);
    } else {*/
        ret = hw_codec_init(oar);
//    }
    if (ret != 0) {
        LOGE("video decoder failed");
    }
    audio_codec_init(oar);

    if (oar->metadata->has_video) {
        /*if (oar->is_sw_decode) {
            pthread_create(&oar->video_decode_thread, NULL, video_decode_sw_thread, oar);
        } else {*/
            pthread_create(&oar->video_decode_thread, NULL, video_decode_hw_thread, oar);
//        }
        pthread_create(&oar->gl_thread, NULL, oar_player_gl_thread, oar);
    }
    if (oar->metadata->has_audio) {
        pthread_create(&oar->audio_decode_thread, NULL, audio_decode_thread, oar);
    }
    oar->change_status(oar, PLAYING);
}