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

#include <unistd.h>
#include <malloc.h>
#include "oar_video_mediacodec.h"
#include "oarplayer_type_def.h"
#define _JNILOG_TAG "videomediacodec"
#include "_android.h"
#include "util.h"

/*#if __ANDROID_API__ >= NDK_MEDIACODEC_VERSION

int oar_video_mediacodec_send_packet(oarplayer * oar, OARPacket *packet) {
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    if (packet == NULL) { return -2; }
    uint32_t keyframe_flag = 0;
//    av_packet_split_side_data(packet);
    int64_t time_stamp = packet->pts;
    if (packet->isKeyframe) {
        keyframe_flag |= 0x1;
    }
    ssize_t id = AMediaCodec_dequeueInputBuffer(ctx->codec, 1000000);
    media_status_t media_status;
    size_t size;
    if (id >= 0) {
        uint8_t *buf = AMediaCodec_getInputBuffer(ctx->codec, (size_t) id, &size);
        if (buf != NULL && size >= packet->size) {
            memcpy(buf, packet->data, (size_t) packet->size);
            media_status = AMediaCodec_queueInputBuffer(ctx->codec, (size_t) id, 0, (size_t) packet->size,
                                                        (uint64_t) time_stamp,
                                                        keyframe_flag);
            if (media_status != AMEDIA_OK) {
                LOGE("AMediaCodec_queueInputBuffer error. status ==> %d", media_status);
                return (int) media_status;
            }
        }
    }else if(id == AMEDIACODEC_INFO_TRY_AGAIN_LATER){
        return -1;
    }else{
        LOGE("input buffer id < 0  value == %zd", id);
    }
    return 0;
}

void oar_video_mediacodec_release_buffer(oarplayer * oar, OARFrame *frame) {
    AMediaCodec_releaseOutputBuffer(oar->video_mediacodec_ctx->codec, (size_t) frame->HW_BUFFER_ID, true);
}

int oar_video_mediacodec_receive_frame(oarplayer * oar, OARFrame *frame) {
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    AMediaCodecBufferInfo info;
    int output_ret = 1;
    ssize_t outbufidx = AMediaCodec_dequeueOutputBuffer(ctx->codec, &info, 0);
    if (outbufidx >= 0) {
            frame->pts = info.presentationTimeUs;
            frame->format = OAR_PIX_FMT_EGL_EXT;
            frame->width = oar->metadata->width;
            frame->height = oar->metadata->height;
            frame->HW_BUFFER_ID = outbufidx;
            output_ret = 0;
    } else {
        switch (outbufidx) {
            case AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED: {
                int pix_format = -1;
                AMediaFormat *format = AMediaCodec_getOutputFormat(ctx->codec);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &ctx->width);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &ctx->height);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &pix_format);
                //todo 仅支持了两种格式
                switch (pix_format) {
                    case 19:
                        ctx->pix_format = PIX_FMT_YUV420P;
                        break;
                    case 21:
                        ctx->pix_format = PIX_FMT_NV12;
                        break;
                    default:
                        break;
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

void oar_video_mediacodec_flush(oarplayer * oar) {
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    AMediaCodec_flush(ctx->codec);
}

oar_video_mediacodec_context *oar_create_video_mediacodec_context(oarplayer * oar) {
    oar_video_mediacodec_context *ctx = (oar_video_mediacodec_context *) malloc(sizeof(oar_video_mediacodec_context));
    ctx->width = oar->metadata->width;
    ctx->height = oar->metadata->height;
    ctx->codec_id = oar->metadata->video_codec;
    ctx->nal_size = 0;
    ctx->format = AMediaFormat_new();
    ctx->pix_format = PIX_FMT_NONE;
//    "video/x-vnd.on2.vp8" - VP8 video (i.e. video in .webm)
//    "video/x-vnd.on2.vp9" - VP9 video (i.e. video in .webm)
//    "video/avc" - H.264/AVC video
//    "video/hevc" - H.265/HEVC video
//    "video/mp4v-es" - MPEG4 video
//    "video/3gpp" - H.263 video
    switch (ctx->codec_id) {
        case VIDEO_CODEC_AVC:
            ctx->codec = AMediaCodec_createDecoderByType("video/avc");
            AMediaFormat_setString(ctx->format, AMEDIAFORMAT_KEY_MIME, "video/avc");
            AMediaFormat_setBuffer(ctx->format, "csd-0", oar->metadata->video_sps, oar->metadata->video_sps_size);
            AMediaFormat_setBuffer(ctx->format, "csd-1", oar->metadata->video_pps, oar->metadata->video_pps_size);
            break;

        case VIDEO_CODEC_H263:
            ctx->codec = AMediaCodec_createDecoderByType("video/3gpp");
            AMediaFormat_setString(ctx->format, AMEDIAFORMAT_KEY_MIME, "video/3gpp");
            AMediaFormat_setBuffer(ctx->format, "csd-0", oar->metadata->video_sps, oar->metadata->video_sps_size);//TODO 获取H263 sps
            break;
        default:
            break;
    }
//    AMediaFormat_setInt32(ctx->format, "rotation-degrees", 90);
    AMediaFormat_setInt32(ctx->format, AMEDIAFORMAT_KEY_WIDTH, ctx->width);
    AMediaFormat_setInt32(ctx->format, AMEDIAFORMAT_KEY_HEIGHT, ctx->height);
//    media_status_t ret = AMediaCodec_configure(ctx->codec, ctx->format, NULL, NULL, 0);

    return ctx;
}

void oar_video_mediacodec_start(oarplayer * oar){
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    while(oar->video_render_ctx->texture_window == NULL){
        usleep(10000);
    }
    media_status_t ret = AMediaCodec_configure(ctx->codec, ctx->format, oar->video_render_ctx->texture_window, NULL, 0);
    if (ret != AMEDIA_OK) {
        LOGE("open mediacodec failed \n");
    }
    ret = AMediaCodec_start(ctx->codec);
    if (ret != AMEDIA_OK) {
        LOGE("open mediacodec failed \n");
    }
}

void oar_video_mediacodec_stop(oarplayer * oar){
    AMediaCodec_stop(oar->video_mediacodec_ctx->codec);
}

void oar_video_mediacodec_release_context(oarplayer * oar){
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    AMediaCodec_delete(ctx->codec);
    AMediaFormat_delete(ctx->format);
    free(ctx);
    oar->video_mediacodec_ctx = NULL;
}
#else*/

oar_video_mediacodec_context *oar_create_video_mediacodec_context(
        oarplayer *oar) {
    oar_video_mediacodec_context *ctx = (oar_video_mediacodec_context *) malloc(sizeof(oar_video_mediacodec_context));
    ctx->width = oar->metadata->width;
    ctx->height = oar->metadata->height;
    ctx->codec_id = oar->metadata->video_codec;
    ctx->nal_size = 0;
    ctx->pix_format = PIX_FMT_NONE;
    return ctx;
}

void oar_video_mediacodec_start(oarplayer *oar){
    LOGE("oar_video_mediacodec_start...");
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    JNIEnv *jniEnv = ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    jobject codecName = NULL, csd_0 = NULL, csd_1 = NULL;
    while(oar->video_render_ctx->texture_window == NULL){
        usleep(10000);
    }
    switch (ctx->codec_id) {
        case VIDEO_CODEC_AVC:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "video/avc");
            if (oar->metadata->video_extradata) {
                size_t sps_size, pps_size;
                uint8_t *sps_buf;
                uint8_t *pps_buf;
                sps_buf = (uint8_t *) malloc((size_t) oar->metadata->video_extradata_size + 20);
                pps_buf = (uint8_t *) malloc((size_t) oar->metadata->video_extradata_size + 20);
                if (0 != convert_sps_pps2(oar->metadata->video_extradata, (size_t) oar->metadata->video_extradata_size,
                                          sps_buf, &sps_size, pps_buf, &pps_size, &ctx->nal_size)) {
                    LOGE("%s:convert_sps_pps: failed\n", __func__);
                }
                LOGE("extradata_size = %d, sps_size = %d, pps_size = %d, nal_size = %d",
                     (size_t) oar->metadata->video_extradata_size, sps_size, pps_size, ctx->nal_size);

                csd_0 = (*jniEnv)->NewDirectByteBuffer(jniEnv, sps_buf, sps_size);
                csd_1 = (*jniEnv)->NewDirectByteBuffer(jniEnv, pps_buf, pps_size);
                (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init,
                                                codecName, ctx->width, ctx->height, csd_0, csd_1);
                (*jniEnv)->DeleteLocalRef(jniEnv, csd_0);
                (*jniEnv)->DeleteLocalRef(jniEnv, csd_1);
            } else {
                (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init,
                                                codecName, ctx->width, ctx->height, NULL, NULL);
            }
            break;

        case VIDEO_CODEC_H263:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "video/3gpp");
            //TODO 解析h263格式
            /*csd_0 = (*jniEnv)->NewDirectByteBuffer(jniEnv, oar->metadata->video_sps, oar->metadata->video_sps_size);
            (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init, codecName,
                                            ctx->width, ctx->height, csd_0, NULL);
            (*jniEnv)->DeleteLocalRef(jniEnv, csd_0);*/
            break;
        default:
            break;
    }
    if (codecName != NULL) {
        (*jniEnv)->DeleteLocalRef(jniEnv, codecName);
    }
}

void oar_video_mediacodec_release_buffer(oarplayer *pd, OARFrame *frame) {
    JNIEnv *jniEnv = pd->video_render_ctx->jniEnv;
    oar_java_class * jc = pd->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_releaseOutPutBuffer,
                                    (int)frame->HW_BUFFER_ID);
}

int oar_video_mediacodec_receive_frame(oarplayer *oar, OARFrame *frame) {
    JNIEnv *jniEnv = oar->video_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    int output_ret = 1;
    jobject deqret = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwDecodeBridge,
                                                       jc->codec_dequeueOutputBufferIndex,
                                                       (jlong) 0);
    uint8_t *retbuf = (*jniEnv)->GetDirectBufferAddress(jniEnv, deqret);
    int outbufidx = get_int(retbuf);
    int64_t pts = get_long(retbuf + 8);
    (*jniEnv)->DeleteLocalRef(jniEnv, deqret);
    LOGE("oar_video_mediacodec_receive_frame outbufidx:%d" , outbufidx);
    if (outbufidx >= 0) {
        frame->type = PktType_Video;
        frame->pts = pts;
        frame->format = PIX_FMT_EGL_EXT;
        frame->width = oar->metadata->width;
        frame->height = oar->metadata->height;
        frame->HW_BUFFER_ID = outbufidx;
        frame->next=NULL;
        output_ret = 0;
    } else {
        switch (outbufidx) {
            // AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED
            case -2: {
                jobject newFormat = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwDecodeBridge,
                                                                      jc->codec_formatChange);
                uint8_t *fmtbuf = (*jniEnv)->GetDirectBufferAddress(jniEnv, newFormat);
                ctx->width = get_int(fmtbuf);
                ctx->height = get_int(fmtbuf + 4);
                int pix_format = get_int(fmtbuf + 8);
                (*jniEnv)->DeleteLocalRef(jniEnv, newFormat);

                //todo 仅支持了两种格式
                switch (pix_format) {
                    case 19:
                        ctx->pix_format = PIX_FMT_YUV420P;
                        break;
                    case 21:
                        ctx->pix_format = PIX_FMT_NV12;
                        break;
                    default:
                        break;
                }
                output_ret = -2;
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
    return output_ret;
}

int oar_video_mediacodec_send_packet(oarplayer *oar, OARPacket *packet) {
    JNIEnv *jniEnv = oar->video_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    if (packet == NULL) { return -2; }
    int keyframe_flag = 0;
    int64_t time_stamp = packet->pts;

    if (ctx->codec_id == VIDEO_CODEC_AVC) {
        H264ConvertState convert_state = {0, 0};
        convert_h264_to_annexb(packet->data, packet->size, ctx->nal_size, &convert_state);
    }
    if (packet->isKeyframe) {
        keyframe_flag |= 0x1;
    }
    int id = (*jniEnv)->CallStaticIntMethod(jniEnv, jc->HwDecodeBridge,
                                            jc->codec_dequeueInputBuffer, (jlong) 1000000);
    if (id >= 0) {
        jobject inputBuffer = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwDecodeBridge,
                                                                jc->codec_getInputBuffer, id);
        uint8_t *buf = (*jniEnv)->GetDirectBufferAddress(jniEnv, inputBuffer);
        jlong size = (*jniEnv)->GetDirectBufferCapacity(jniEnv, inputBuffer);
        if (buf != NULL && size >= packet->size) {
            memcpy(buf, packet->data, (size_t) packet->size);
            (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge,
                                            jc->codec_queueInputBuffer,
                                            (jint) id, (jint) packet->size,
                                            (jlong) time_stamp, (jint) keyframe_flag);
        }
        (*jniEnv)->DeleteLocalRef(jniEnv, inputBuffer);
    } else if (id == -1) {
        LOGE("dequeue inputbuffer is -1");
        return -1;
    } else {
        LOGE("input buffer id < 0  value == %zd", id);
    }
    return 0;
}

void oar_video_mediacodec_flush(oarplayer *oar) {
    JNIEnv *jniEnv = oar->video_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_flush);
}

void oar_video_mediacodec_release_context(oarplayer *oar) {
    JNIEnv *jniEnv = oar->jniEnv;
    oar_java_class * jc = oar->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_release);
    oar_video_mediacodec_context *ctx = oar->video_mediacodec_ctx;
    free(ctx);
    oar->video_mediacodec_ctx = NULL;
}

void oar_video_mediacodec_stop(oarplayer *oar) {
    JNIEnv *jniEnv = oar->video_mediacodec_ctx->jniEnv;
    oar_java_class * jc = oar->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_stop);
}

//#endif