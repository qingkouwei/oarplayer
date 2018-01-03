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
#include "oar_audio_player.h"

#include "oar_frame_queue.h"
#include "oar_clock.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <assert.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#define _JNILOG_TAG "audio_player"
#include "_android.h"
#include "oar_audio_mediacodec.h"

//engine interface
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

//output mix interface
static SLObjectItf outputMixObject = NULL;

//buffer queue player interface
static SLObjectItf bqPlayerObject =NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLVolumeItf bqPlayerVolume;

static int64_t get_delta_time(oar_audio_player_context *ctx){
    SLmillisecond sec;
    (*bqPlayerPlay)->GetPosition(bqPlayerPlay, &sec);
    if(sec > ctx->play_pos){
        return (int64_t)(sec-ctx->play_pos)*1000;
    }
    return 0;
}

static int get_audio_frame(oarplayer *oar){
//    LOGE("get_audio_frame....");
    oar_audio_player_context *ctx = oar->audio_player_ctx;
    if(oar->status == IDEL/* || oar->status == PAUSED || oar->status == BUFFER_EMPTY*/){
        LOGE("audio play state is not playing...");
        return -1;
    }
    LOGE("audio frame queue size:%d", oar->audio_frame_queue->count);
    oar->audio_frame = oar_frame_queue_get(oar->audio_frame_queue);
    // buffer empty ==> return -1
    // eos          ==> return -1
    if(oar->audio_frame == NULL){
        //如果没有音频流,就从这里发送结束信号
        if(oar->eof && !oar->metadata->has_audio || oar->just_audio){
            oar->send_message(oar, oar_message_stop);
        }
        LOGE("oar_frame_queue_get is null...");
        return -1;
    }
    ctx->frame_size = oar->audio_frame->size;
    int64_t  time_stamp = oar->audio_frame->pts;
    if(ctx->buffer_size < ctx->frame_size){
        ctx->buffer_size = ctx->frame_size;
        if(ctx->buffer == NULL){
            ctx->buffer = malloc((size_t)ctx->buffer_size);
        }else{
            ctx->buffer = realloc(ctx->buffer, (size_t)ctx->buffer_size);
        }
    }
    if(ctx->frame_size >0){
        memcpy(ctx->buffer, oar->audio_frame->data, (size_t)ctx->frame_size);
    }
//    LOGE("release buffer : %d", oar->audio_frame->HW_BUFFER_ID);
    free(oar->audio_frame->data);
    free(oar->audio_frame);
    oar->audio_frame = NULL;
    oar_clock_set(oar->audio_clock, time_stamp);
    return 0;
}

static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context){
//    LOGE("bqPlayerCallback...");
    oarplayer *oar = context;
    oar_audio_player_context *ctx = oar->audio_player_ctx;
    pthread_mutex_lock(ctx->lock);
    assert(bq==bqPlayerBufferQueue);
    if(-1 == get_audio_frame(oar)){
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
        pthread_mutex_unlock(ctx->lock);
        return;
    }
//    LOGE("buffer size : %d", ctx->frame_size);
    //for streaming playback, replace this test by logic to find and fill the next bufffer
    if(NULL != ctx->buffer && 0 != ctx->frame_size){
        SLresult result;
        //enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, ctx->buffer,
                                                 (SLuint32)ctx->frame_size);
        (*bqPlayerPlay)->GetPosition(bqPlayerPlay, &ctx->play_pos);
        (void)result;
    }
    pthread_mutex_unlock(ctx->lock);
}


void oar_audio_play(oarplayer *oar){
    SLresult result = 0;
    (*bqPlayerPlay)->GetPlayState(bqPlayerPlay, &result);
    if(result == SL_PLAYSTATE_PAUSED){
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
        bqPlayerCallback(bqPlayerBufferQueue, oar);
    }
}
void oar_audio_player_shutdown();
void oar_audio_player_release(oar_audio_player_context *ctx);
void oar_audio_player_create(int rate, int channel, oarplayer *oar);


oar_audio_player_context* oar_audio_engine_create(){
    SLresult result;
    //create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    //realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void) result;

    //get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    //create output mix, with environmental reverb specified as a non-required interface
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    //realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    //ignore unsuccessful result codes for environmental reverb, as it is optional for this example

    oar_audio_player_context *ctx = malloc(sizeof(oar_audio_player_context));
    ctx->play = oar_audio_play;
    ctx->shutdown = oar_audio_player_shutdown;
    ctx->release = oar_audio_player_release;
    ctx->player_create = oar_audio_player_create;
    ctx->get_delta_time = get_delta_time;
    ctx->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    ctx->buffer_size = 0;
    ctx->buffer =NULL;
    pthread_mutex_init(ctx->lock, NULL);
    return ctx;

}
void oar_audio_player_create(int rate, int channel, oarplayer* oar){

    LOGE("oar_audio_player_create: rate = %d, channel = %d", rate, channel);
    SLresult result;

    //configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    /**
     *  SLuint32 		formatType;
	 *  SLuint32 		numChannels;
	 *  SLuint32 		samplesPerSec;
	 *  SLuint32 		bitsPerSample;
	 *  SLuint32 		containerSize;
	 *  SLuint32 		channelMask;
	 *  SLuint32		endianness;
     */
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channel, SL_SAMPLINGRATE_44_1,
                                    SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
//    format_pcm.numChannels = (SLuint32)channel;
//    format_pcm.samplesPerSec = (SLuint32)(rate*1000);
    if(channel == 2){
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }else{
        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    //configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                2, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    //realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    //get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    //get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    //register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, oar);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    //get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
}

void oar_audio_player_shutdown(){
    if(bqPlayerPlay != NULL){
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}
void oar_audio_player_release(oar_audio_player_context *ctx){
    //destroy buffer queue audio player object, and invalidate all associated interfaces
    if(bqPlayerObject != NULL){
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerVolume = NULL;
    }

    //destroy output mix object, and invalidate all associated interfaces
    if(outputMixObject != NULL){
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }

    //destroy engine object,and ivalidate all associated interface
    if(engineObject != NULL){
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    if(ctx->lock != NULL){
        pthread_mutex_destroy(ctx->lock);
        free(ctx->lock);
    }
    if(ctx->buffer != NULL){
        free(ctx->buffer);
    }
    free(ctx);
}