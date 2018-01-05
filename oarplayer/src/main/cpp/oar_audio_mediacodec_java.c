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
#define _JNILOG_TAG "audiomediacodec"
#include "_android.h"
#include <unistd.h>
#include <malloc.h>
#include "oar_audio_mediacodec_java.h"
#include "util.h"

#define isDebug 0
#define _LOGD if(isDebug) LOGI

void oar_audio_mediacodec_start(oarplayer *oar){
    _LOGD("oar_audio_mediacodec_start...");
    oar_audio_mediacodec_context *ctx = oar->audio_mediacodec_ctx;
    JNIEnv *jniEnv = ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    jobject codecName = NULL, csd_0 = NULL;

    switch (ctx->codec_id) {
        case AUDIO_CODEC_AAC:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "audio/mp4a-latm");
            if (oar->metadata->audio_pps) {
                csd_0 = (*jniEnv)->NewDirectByteBuffer(jniEnv, oar->metadata->audio_pps, oar->metadata->audio_pps_size);
                (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwAudioDecodeBridge, jc->audio_codec_init,
                                                codecName, ctx->sample_rate, ctx->channel_count, csd_0);
                (*jniEnv)->DeleteLocalRef(jniEnv, csd_0);
            } else {
                (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwAudioDecodeBridge, jc->audio_codec_init,
                                                codecName, ctx->sample_rate, ctx->channel_count, NULL);
            }
            break;
        default:
            break;
    }
    if (codecName != NULL) {
        (*jniEnv)->DeleteLocalRef(jniEnv, codecName);
    }
}

void oar_audio_mediacodec_release_buffer(oarplayer *oar, int index) {
    JNIEnv *jniEnv = oar->audio_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwAudioDecodeBridge, jc->audio_codec_releaseOutPutBuffer,
                                    index);
}

int oar_audio_mediacodec_receive_frame(oarplayer *oar, OARFrame **frame) {
    JNIEnv *jniEnv = oar->audio_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    oar_audio_mediacodec_context *ctx = oar->audio_mediacodec_ctx;
    int output_ret = 1;
    jobject deqret = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwAudioDecodeBridge,
                                                       jc->audio_codec_dequeueOutputBufferIndex,
                                                       (jlong) 0);
    uint8_t *retbuf = (*jniEnv)->GetDirectBufferAddress(jniEnv, deqret);
    int outbufidx = get_int(retbuf);
    int64_t pts = get_long(retbuf + 8);
    int size = get_int(retbuf + 16);
    (*jniEnv)->DeleteLocalRef(jniEnv, deqret);
    _LOGD("outbufidx:%d" , outbufidx);
    if (outbufidx >= 0) {
        *frame = malloc(sizeof(OARFrame) + size);
        jobject outputBuf = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwAudioDecodeBridge,
                                                        jc->audio_codec_getOutputBuffer,
                                                        outbufidx);
        uint8_t *buf = (*jniEnv)->GetDirectBufferAddress(jniEnv, outputBuf);
        if (buf != NULL){
            memcpy((*frame)->data, buf, size);
        }
        _LOGD("release outputbuffer index : %d", outbufidx);
        (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwAudioDecodeBridge,
                                       jc->audio_codec_releaseOutPutBuffer,
                                        outbufidx);
        (*jniEnv)->DeleteLocalRef(jniEnv, outputBuf);
        (*frame)->type = PktType_Audio;
        (*frame)->size = size;
        (*frame)->pts = pts;
        (*frame)->HW_BUFFER_ID = outbufidx;
        (*frame)->next=NULL;
        output_ret = 0;
    } else {
        switch (outbufidx) {
            // AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED
            case -2: {
                jobject newFormat = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwAudioDecodeBridge,
                                                                      jc->audio_codec_formatChange);
                uint8_t *fmtbuf = (*jniEnv)->GetDirectBufferAddress(jniEnv, newFormat);
                ctx->sample_rate = get_int(fmtbuf);
                ctx->channel_count = get_int(fmtbuf + 4);
                (*jniEnv)->DeleteLocalRef(jniEnv, newFormat);
                output_ret = -2;
                break;
            }
            // AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED
            case -3:
                _LOGD("output_ret: %d", -3);
                break;
            // AMEDIACODEC_INFO_TRY_AGAIN_LATER
            case -1:
                _LOGD("output_ret: %d", -1);
                break;
            default:
               _LOGD("output_ret: %d", outbufidx);
                break;
        }

    }
    return output_ret;
}

int oar_audio_mediacodec_send_packet(oarplayer *oar, OARPacket *packet) {
    JNIEnv *jniEnv = oar->audio_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    oar_audio_mediacodec_context *ctx = oar->audio_mediacodec_ctx;
    if (packet == NULL) { return -2; }
    int64_t time_stamp = packet->pts;

    int id = (*jniEnv)->CallStaticIntMethod(jniEnv, jc->HwAudioDecodeBridge,
                                            jc->audio_codec_dequeueInputBuffer, (jlong) 1000000);
    if (id >= 0) {
        _LOGD("start get inputbuffer...");
        jobject inputBuffer = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwAudioDecodeBridge,
                                                                jc->audio_codec_getInputBuffer, id);
        uint8_t *buf = (*jniEnv)->GetDirectBufferAddress(jniEnv, inputBuffer);
        jlong size = (*jniEnv)->GetDirectBufferCapacity(jniEnv, inputBuffer);
        if (buf != NULL && size >= packet->size) {
            /*_LOGD("data:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                 packet->data[0],packet->data[1],packet->data[2],packet->data[3],
                 packet->data[4],packet->data[5],packet->data[6],packet->data[7],
                 packet->data[8],packet->data[9],packet->data[10],packet->data[11],
                 packet->data[12],packet->data[13],packet->data[14],packet->data[15]);*/
            memcpy(buf, packet->data, (size_t) packet->size);
            (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwAudioDecodeBridge,
                                            jc->audio_codec_queueInputBuffer,
                                            (jint) id, (jint) packet->size,
                                            (jlong) time_stamp, (jint) 0);
        }
        (*jniEnv)->DeleteLocalRef(jniEnv, inputBuffer);
    } else if (id == -1) {
        return -1;
    } else {
        LOGE("input buffer id < 0  value == %zd", id);
    }
    return 0;
}

void oar_audio_mediacodec_flush(oarplayer *oar) {
    JNIEnv *jniEnv = oar->audio_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwAudioDecodeBridge, jc->audio_codec_flush);
}

void oar_audio_mediacodec_release_context(oarplayer *oar) {
    JNIEnv *jniEnv = oar->jniEnv;
    oar_java_class * jc = oar->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwAudioDecodeBridge, jc->audio_codec_release);
    oar_audio_mediacodec_context *ctx = oar->audio_mediacodec_ctx;
    free(ctx);
    oar->audio_mediacodec_ctx = NULL;
}

void oar_audio_mediacodec_stop(oarplayer *oar) {
    JNIEnv *jniEnv = oar->audio_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwAudioDecodeBridge, jc->audio_codec_stop);
}
