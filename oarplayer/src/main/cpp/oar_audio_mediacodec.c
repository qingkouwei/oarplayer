
#include <unistd.h>
#include <malloc.h>
#include "oar_audio_mediacodec.h"
#define _JNILOG_TAG "audiomediacodec"
#include "_android.h"
#include "util.h"


oar_audio_mediacodec_context *oar_create_audio_mediacodec_context(
        oarplayer *oar) {
    oar_audio_mediacodec_context *ctx = (oar_audio_mediacodec_context *) malloc(sizeof(oar_audio_mediacodec_context));
    ctx->channel_count = oar->metadata->channels;
    ctx->sample_rate = oar->metadata->sample_rate;
    ctx->codec_id = oar->metadata->audio_codec;
    return ctx;
}

void oar_audio_mediacodec_start(oarplayer *oar){
    LOGE("oar_audio_mediacodec_start...");
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

void oar_audio_mediacodec_release_buffer(oarplayer *pd, int index) {
    JNIEnv *jniEnv = pd->audio_mediacodec_ctx->jniEnv;
    oar_java_class * jc = pd->jc;
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
//    LOGE("outbufidx:%d" , outbufidx);
    if (outbufidx >= 0) {
        *frame = malloc(sizeof(OARFrame) + size);
        jobject outputBuf = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwAudioDecodeBridge,
                                                        jc->audio_codec_getOutputBuffer,
                                                        outbufidx);
        uint8_t *buf = (*jniEnv)->GetDirectBufferAddress(jniEnv, outputBuf);
        if (buf != NULL){
            memcpy((*frame)->data, buf, size);
        }
//        LOGI("release outputbuffer index : %d", outbufidx);
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
                int pcm_encoding = get_int(fmtbuf + 8);
                (*jniEnv)->DeleteLocalRef(jniEnv, newFormat);
                output_ret = -2;
                break;
            }
            // AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED
            case -3:
//                LOGE("output_ret: %d", -3);
                break;
            // AMEDIACODEC_INFO_TRY_AGAIN_LATER
            case -1:
//                LOGE("output_ret: %d", -1);
                break;
            default:
//                LOGE("output_ret: %d", outbufidx);
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
        //LOGE("start get inputbuffer...");
        jobject inputBuffer = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwAudioDecodeBridge,
                                                                jc->audio_codec_getInputBuffer, id);
        uint8_t *buf = (*jniEnv)->GetDirectBufferAddress(jniEnv, inputBuffer);
        jlong size = (*jniEnv)->GetDirectBufferCapacity(jniEnv, inputBuffer);
        if (buf != NULL && size >= packet->size) {
            /*LOGI("data:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
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
