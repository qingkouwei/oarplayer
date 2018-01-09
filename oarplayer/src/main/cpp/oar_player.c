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
#define _JNILOG_TAG "oar_player"
#include "_android.h"
#include <malloc.h>
#include <unistd.h>
#include <dlfcn.h>
#include "oar_player.h"
#include "oar_clock.h"
#include "oar_jni_reflect.h"
#include "oar_packet_queue.h"
#include "srs_readthread.h"
#include "oar_video_render.h"
#include "oar_player_video_hw_decode_thread.h"
#include "oar_video_mediacodec_java.h"
#include "oar_player_gl_thread.h"
#include "oar_frame_queue.h"
#include "oar_player_audio_hw_decode_thread.h"
#include "oar_audio_player.h"
#include "oar_audio_mediacodec_java.h"
#include "oar_audio_mediacodec_ndk.h"
#include "oar_video_mediacodec_ctx.h"
#include "oar_audio_mediacodec_ctx.h"
#include "oar_video_mediacodec_ndk.h"

#define isDebug 1
#define _LOGD if(isDebug) LOGI

static int stop(oarplayer *oar);

static void on_error(oarplayer *oar);

static void change_status(oarplayer *oar, PlayStatus status);

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
static void on_error_cb(oarplayer *oar, int error_code) {
    oar->error_code = error_code;
    oar->send_message(oar, oar_message_error);
}

static void buffer_empty_cb(void *data) {
    oarplayer *oar = data;
    if (oar->status != BUFFER_EMPTY) {
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
    oar->just_audio = false;
    oar->url = NULL;
    oar->audio_frame = NULL;
    oar->video_frame = NULL;
    oar_clock_reset(oar->audio_clock);
    oar_clock_reset(oar->video_clock);
    oar->error_code = 0;
    oar->frame_rotation = OAR_ROTATION_0;
    oar->change_status(oar, IDEL);
    oar_video_render_ctx_reset(oar->video_render_ctx);
}

static inline void set_buffer_time(oarplayer *oar) {
    float buffer_time_length = oar->buffer_time_length;
    if (oar->metadata->has_audio) {
        oar_queue_set_duration(oar->audio_packet_queue,buffer_time_length);
        oar->audio_packet_queue->empty_cb = buffer_empty_cb;
        oar->audio_packet_queue->full_cb = buffer_full_cb;
        oar->audio_packet_queue->cb_data = oar;
    }
    if (oar->metadata->has_video) {
        oar_queue_set_duration(oar->video_packet_queue,buffer_time_length);
        oar->video_packet_queue->empty_cb = buffer_empty_cb;
        oar->video_packet_queue->full_cb = buffer_full_cb;
        oar->video_packet_queue->cb_data = oar;
    }
}

static oar_dl_context *create_dl_context()
{
    oar_dl_context *dl_context = (oar_dl_context *)malloc(sizeof(oar_dl_context));
    dl_context->libHandler = dlopen("./libmediacodec-lib.so", RTLD_NOW);
    if (!dl_context->libHandler) {
        LOGE("!!!!!!!! failed to load library: %s");
        return NULL;
    }

    dl_context->create_native_mediacodec = dlsym(dl_context->libHandler, "oar_create_native_mediacodec");
    dl_context->native_mediacodec_flush = dlsym(dl_context->libHandler, "oar_native_mediacodec_flush");
    dl_context->native_mediacodec_receive_frame = dlsym(dl_context->libHandler, "oar_native_mediacodec_receive_frame");
    dl_context->native_mediacodec_release_buffer = dlsym(dl_context->libHandler, "oar_native_mediacodec_release_buffer");
    dl_context->native_mediacodec_release_context = dlsym(dl_context->libHandler, "oar_native_mediacodec_release_context");
    dl_context->native_mediacodec_send_packet = dlsym(dl_context->libHandler, "oar_native_mediacodec_send_packet");
    dl_context->native_mediacodec_start = dlsym(dl_context->libHandler, "oar_native_mediacodec_start");
    dl_context->native_mediacodec_stop = dlsym(dl_context->libHandler, "oar_native_mediacodec_stop");
    return dl_context;
}

static void release_dl_context(oarplayer *oar)
{
    if(oar->dl_context ==NULL){
        return;
    }
    oar->dl_context->create_native_mediacodec = NULL;
    oar->dl_context->native_mediacodec_flush = NULL;
    oar->dl_context->native_mediacodec_receive_frame = NULL;
    oar->dl_context->native_mediacodec_release_buffer = NULL;
    oar->dl_context->native_mediacodec_release_context = NULL;
    oar->dl_context->native_mediacodec_send_packet = NULL;
    oar->dl_context->native_mediacodec_start = NULL;
    oar->dl_context->native_mediacodec_stop = NULL;
    if (oar->dl_context->libHandler != NULL) {
        dlclose (oar->dl_context->libHandler);
    }
    free(oar->dl_context);
    oar->dl_context= NULL;
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
    oar->buffer_time_length = default_buffer_time;


    oar->metadata = (oar_metadata_t*)malloc(sizeof(oar_metadata_t));
    oar->metadata->has_audio = 0;
    oar->metadata->has_video = 0;
    oar->metadata->video_extradata = NULL;
    oar->metadata->audio_pps = NULL;

    if(oar->run_android_version >= NDK_MEDIACODEC_VERSION){
        _LOGD("create dl context");
        oar->dl_context = create_dl_context();
    }else{
        oar->dl_context = NULL;
    }

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
void oar_player_set_play_background(oarplayer *oar, bool play_background) {
    if (oar->just_audio && oar->metadata->has_video) {
        oar->video_mediacodec_ctx->oar_video_mediacodec_flush(oar);
    }
    oar->just_audio = play_background;
}

int oar_player_play(oarplayer *oar){
    oar->error_code = 0;
    pthread_create(&oar->read_stream_thread, NULL, read_thread, oar);

    return 0;
}
static inline void clean_queues(oarplayer *oar) {
    // clear oar->audio_frame audio_frame_queue  audio_packet_queue
    if (oar->metadata->has_audio) {
        if(oar->audio_frame != NULL){
            free(oar->audio_frame);
        }
        while (1) {
            oar->audio_frame = oar_frame_queue_get(oar->audio_frame_queue);
            if (oar->audio_frame == NULL) {
                break;
            }
            free(oar->audio_frame);
        }
        OARPacket *packet = NULL;
        while (1) {
            packet = oar_packet_queue_get(oar->audio_packet_queue);
            if (packet == NULL) {
                break;
            }
            free(packet);
        }
    }
    // clear oar->video_frame video_frame_queue video_frame_packet
    if (oar->metadata->has_video) {
        if(oar->video_frame != NULL){
            free(oar->video_frame);
        }
        while (1) {
            oar->video_frame = oar_frame_queue_get(oar->video_frame_queue);
            if (oar->video_frame == NULL) {
                break;
            }
            free(oar->video_frame);
        }
        OARPacket *packet = NULL;
        while (1) {
            packet = oar_packet_queue_get(oar->video_packet_queue);
            if (packet == NULL) {
                break;
            }
            free(packet);
        }
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
    if (oar->metadata->has_video) {
        pthread_join(oar->video_decode_thread, &thread_res);
        oar->video_mediacodec_ctx->oar_video_mediacodec_release_context(oar);
        pthread_join(oar->gl_thread, &thread_res);
    }

    if (oar->metadata->has_audio) {
        pthread_join(oar->audio_decode_thread, &thread_res);
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_release_context(oar->audio_mediacodec_ctx->ACodec);
        oar->audio_player_ctx->shutdown();
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
    if(oar->dl_context ){
        release_dl_context(oar);
    }
    (*oar->jniEnv)->DeleteGlobalRef(oar->jniEnv, oar->oarPlayer);
    oar_jni_free(&oar->jc, oar->jniEnv);
    free(oar);
    return 0;
}
static int oar_player_resume(oarplayer *oar) {
    oar->change_status(oar, PLAYING);
    if (oar->metadata->has_audio) {
        oar->audio_player_ctx->play(oar);
    }
    return 0;
}
static void change_status(oarplayer *oar, PlayStatus status) {
    if (status == BUFFER_FULL) {
        oar_player_resume(oar);
    } else {
        oar->status = status;
    }
    (*oar->jniEnv)->CallVoidMethod(oar->jniEnv, oar->oarPlayer, oar->jc->player_onPlayStatusChanged,
                                  status);
}

static void on_error(oarplayer *oar) {
    (*oar->jniEnv)->CallVoidMethod(oar->jniEnv, oar->oarPlayer, oar->jc->player_onPlayError,
                                   oar->error_code);
}
static int hw_codec_init(oarplayer *oar) {
    oar->video_mediacodec_ctx = oar_create_video_mediacodec_context(oar);
    if(oar->dl_context){
        oar->video_mediacodec_ctx->oar_video_mediacodec_receive_frame = oar_video_mediacodec_receive_frame_ndk;
        oar->video_mediacodec_ctx->oar_video_mediacodec_release_buffer = oar_video_mediacodec_release_buffer_ndk;
        oar->video_mediacodec_ctx->oar_video_mediacodec_send_packet = oar_video_mediacodec_send_packet_ndk;
        oar->video_mediacodec_ctx->oar_video_mediacodec_flush = oar_video_mediacodec_flush_ndk;
        oar->video_mediacodec_ctx->oar_video_mediacodec_release_context = oar_video_mediacodec_release_context_ndk;
        oar->video_mediacodec_ctx->oar_video_mediacodec_start = oar_video_mediacodec_start_ndk;
        oar->video_mediacodec_ctx->oar_video_mediacodec_stop = oar_video_mediacodec_stop_ndk;
        oar_create_video_mediacodec_ndk(oar);
    }else{
        oar->video_mediacodec_ctx->oar_video_mediacodec_receive_frame = oar_video_mediacodec_receive_frame;
        oar->video_mediacodec_ctx->oar_video_mediacodec_release_buffer = oar_video_mediacodec_release_buffer;
        oar->video_mediacodec_ctx->oar_video_mediacodec_send_packet = oar_video_mediacodec_send_packet;
        oar->video_mediacodec_ctx->oar_video_mediacodec_flush = oar_video_mediacodec_flush;
        oar->video_mediacodec_ctx->oar_video_mediacodec_release_context = oar_video_mediacodec_release_context;
        oar->video_mediacodec_ctx->oar_video_mediacodec_start = oar_video_mediacodec_start;
        oar->video_mediacodec_ctx->oar_video_mediacodec_stop = oar_video_mediacodec_stop;
    }
    return 0;
}
static int audio_codec_init(oarplayer *oar) {

    oar->audio_mediacodec_ctx = oar_create_audio_mediacodec_context(oar);
    if(oar->dl_context){
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_receive_frame = oar_audio_mediacodec_receive_frame_ndk;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_release_buffer = oar_audio_mediacodec_release_buffer_ndk;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_send_packet = oar_audio_mediacodec_send_packet_ndk;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_flush = oar_audio_mediacodec_flush_ndk;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_release_context = oar_audio_mediacodec_release_context_ndk;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_start = oar_audio_mediacodec_start_ndk;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_stop = oar_audio_mediacodec_stop_ndk;
        oar_create_audio_mediacodec_ndk(oar);
    }else{
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_receive_frame = oar_audio_mediacodec_receive_frame;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_release_buffer = oar_audio_mediacodec_release_buffer;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_send_packet = oar_audio_mediacodec_send_packet;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_flush = oar_audio_mediacodec_flush;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_release_context = oar_audio_mediacodec_release_context;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_start = oar_audio_mediacodec_start;
        oar->audio_mediacodec_ctx->oar_audio_mediacodec_stop = oar_audio_mediacodec_stop;
    }
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
    int ret = hw_codec_init(oar);
    if (ret != 0) {
        LOGE("video decoder failed");
    }
    audio_codec_init(oar);

    if (oar->metadata->has_video) {
        pthread_create(&oar->video_decode_thread, NULL, video_decode_hw_thread, oar);
        pthread_create(&oar->gl_thread, NULL, oar_player_gl_thread, oar);
    }
    if (oar->metadata->has_audio) {
        pthread_create(&oar->audio_decode_thread, NULL, audio_decode_thread, oar);
    }
    oar->change_status(oar, PLAYING);
}