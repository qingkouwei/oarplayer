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
#define _JNILOG_TAG "native_mediacodec"
#include "_android.h"
#include "oar_native_mediacodec.h"
#include <media/NdkMediaCodec.h>
#include <string.h>

#define VIDEO_CODEC_AVC 7
#define VIDEO_CODEC_H263 2
#define AUDIO_CODEC_AAC 10

int oar_native_mediacodec_send_packet(void * codec,
                                     int len,
                                     int type,
                                     int64_t dts,
                                     int64_t pts,
                                     int isKeyframe,
                                     uint8_t *data) {
    AMediaCodec *c = (AMediaCodec *)codec;
    if (data == NULL) { return -2; }
    uint32_t keyframe_flag = 0;
    int64_t time_stamp = pts;
    if (isKeyframe) {
        keyframe_flag |= 0x1;
    }
    ssize_t id = AMediaCodec_dequeueInputBuffer(c, 1000000);
    media_status_t media_status;
    size_t size;
    if (id >= 0) {
        uint8_t *buf = AMediaCodec_getInputBuffer(c, (size_t) id, &size);
        if (buf != NULL && size >= len) {
            memcpy(buf, data, (size_t)len);
            media_status = AMediaCodec_queueInputBuffer(c, (size_t) id, 0, (size_t) len,
                                                        (uint64_t) time_stamp,
                                                        keyframe_flag);
            if (media_status != AMEDIA_OK) {
//                LOGE("AMediaCodec_queueInputBuffer error. status ==> %d", media_status);
                return (int) media_status;
            }
        }
    }else if(id == AMEDIACODEC_INFO_TRY_AGAIN_LATER){
        return -1;
    }else{
//        LOGE("input buffer id < 0  value == %zd", id);
    }
    return 0;
}

void oar_native_mediacodec_release_buffer(void * codec, int bufferID, bool render) {
    AMediaCodec *c = (AMediaCodec *)codec;
    AMediaCodec_releaseOutputBuffer(c, (size_t) bufferID, render);
}

int oar_native_mediacodec_receive_frame(void * codec,
                                        void **frame,
                                        void *oar,
                                        int type,
                                       void *(frameGenerate)(void *,void **, void*, int, int64_t,ssize_t,int, int, int)) {
    AMediaCodec *c = (AMediaCodec *)codec;
    AMediaCodecBufferInfo info;
    int output_ret = 1;
    ssize_t outbufidx = AMediaCodec_dequeueOutputBuffer(c, &info, 0);
    if (outbufidx >= 0) {
        if(type == 0){//video
            frameGenerate(oar, frame, NULL,  0, info.presentationTimeUs, outbufidx, -1, -1, 2);
        }else{
            int size = 0;
            uint8_t *data = AMediaCodec_getOutputBuffer(c, outbufidx, &size);
//            LOGE("index : %d, size:%d, info.size : %d, offset: %d", outbufidx, size, info.size, info.offset);
            frameGenerate(oar, frame, data, info.size, info.presentationTimeUs, outbufidx, -1, -1, -1);
            AMediaCodec_releaseOutputBuffer(c,outbufidx, false);
        }
        output_ret = 0;
    } else {
        switch (outbufidx) {
            case AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED: {
                AMediaFormat *format = AMediaCodec_getOutputFormat(c);
                if(type == 0){
                    int pix_format = -1;
                    int width =0, height =0;
                    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &width);
                    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &height);
                    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &pix_format);
                    frameGenerate(oar, frame, NULL, 0, 0, AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED, width, height, pix_format);
                }else{
                    int sample_rate =0, channel_count =0;
                    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &sample_rate);
                    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &channel_count);
                    frameGenerate(oar,frame, NULL, 0, 0, AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED, sample_rate, channel_count, -1);
                }
                output_ret = -2;
                break;
            }
            case AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED:
                break;
            case AMEDIACODEC_INFO_TRY_AGAIN_LATER:
                break;
            default:
                break;
        }

    }
    return output_ret;
}

void oar_native_mediacodec_flush(void * codec) {
    AMediaCodec *c = (AMediaCodec *)codec;
    AMediaCodec_flush(c);
}

void *oar_create_native_mediacodec(int codec_id,
                                          int width, int height,
                                          int sample_rate, int channelCount,
                                          uint8_t *sps, int sps_size,
                                          uint8_t *pps, int pps_size,
                                           void *ctx,
                                          void (*formatCreated(void*, void*))) {
    /*LOGE("codecid:%d, width:%d, height:%d, samplerate:%d, channelCount:%d, sps_size:%d", codec_id,
    width, height, sample_rate, channelCount, sps_size);*/
    AMediaCodec *codec = NULL;
    AMediaFormat *format = AMediaFormat_new();
//    "video/x-vnd.on2.vp8" - VP8 video (i.e. video in .webm)
//    "video/x-vnd.on2.vp9" - VP9 video (i.e. video in .webm)
//    "video/avc" - H.264/AVC video
//    "video/hevc" - H.265/HEVC video
//    "video/mp4v-es" - MPEG4 video
//    "video/3gpp" - H.263 video
    switch (codec_id) {
        case VIDEO_CODEC_AVC:
            codec = AMediaCodec_createDecoderByType("video/avc");
            AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
            AMediaFormat_setBuffer(format, "csd-0", sps, sps_size);
            AMediaFormat_setBuffer(format, "csd-1", pps, pps_size);
            //    AMediaFormat_setInt32(ctx->format, "rotation-degrees", 90);
            AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, width);
            AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, height);
            break;

        case VIDEO_CODEC_H263:
            codec = AMediaCodec_createDecoderByType("video/3gpp");
            AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/3gpp");
            AMediaFormat_setBuffer(format, "csd-0", sps, sps_size);
            //    AMediaFormat_setInt32(ctx->format, "rotation-degrees", 90);
            AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, width);
            AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, height);
            break;
        case AUDIO_CODEC_AAC:
            codec = AMediaCodec_createDecoderByType("audio/mp4a-latm");
            AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "audio/mp4a-latm");
            AMediaFormat_setBuffer(format, "csd-0", sps, sps_size);
            AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, channelCount);
            AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, sample_rate);
            break;
        default:
            break;
    }

    formatCreated(ctx, format);
    return codec;
}

int oar_native_mediacodec_start(void * codec, void *format, void *window){
    AMediaCodec *c = (AMediaCodec *)codec;
    AMediaFormat *f = (AMediaFormat *)format;
    ANativeWindow *texture_window = NULL;
    if(window != NULL){
        texture_window = (ANativeWindow *)window;
    }
    media_status_t ret = AMediaCodec_configure(c, f, texture_window, NULL, 0);
    if (ret != AMEDIA_OK) {
        return ret;
    }
    ret = AMediaCodec_start(c);
    /*if (ret != AMEDIA_OK) {
        return ret;
    }*/
    return ret;
}

void oar_native_mediacodec_stop(void * codec){
    AMediaCodec *c = (AMediaCodec *)codec;
    AMediaCodec_stop(c);
}

void oar_native_mediacodec_release_context(void * codec, void *format){
    AMediaCodec *c = (AMediaCodec *)codec;
    AMediaFormat *f = (AMediaFormat *)format;
    AMediaCodec_delete(c);
    AMediaFormat_delete(f);
}