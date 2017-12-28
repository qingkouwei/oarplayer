

#include <malloc.h>
#include "oar_jni_reflect.h"

void oar_jni_reflect_java_class(oar_java_class ** p_jc, JNIEnv *jniEnv) {
    oar_java_class * jc = malloc(sizeof(oar_java_class));
    jclass xlPlayerClass = (*jniEnv)->FindClass(jniEnv, "com/wodekouwei/srsrtmpplayer/SrsPlayer");
    jc->player_onPlayStatusChanged = (*jniEnv)->GetMethodID(jniEnv, xlPlayerClass,
                                                     "onPlayStatusChanged", "(I)V");
    jc->player_onPlayError = (*jniEnv)->GetMethodID(jniEnv, xlPlayerClass,
                                                     "onPlayError", "(I)V");
//    jc->XLPlayer_class = (*jniEnv)->NewGlobalRef(jniEnv, xlPlayerClass);
    (*jniEnv)->DeleteLocalRef(jniEnv, xlPlayerClass);

    jclass java_HwDecodeBridge = (*jniEnv)->FindClass(jniEnv, "com/wodekouwei/srsrtmpplayer/proxy/HwDecodeWrapper");
    jc->codec_init = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "init", "(Ljava/lang/String;IILjava/nio/ByteBuffer;Ljava/nio/ByteBuffer;)V");
    jc->codec_stop = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "stop", "()V");
    jc->codec_flush = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "flush",  "()V");
    jc->codec_dequeueInputBuffer = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "dequeueInputBuffer", "(J)I");
    jc->codec_queueInputBuffer = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "queueInputBuffer", "(IIJI)V");
    jc->codec_getInputBuffer = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "getInputBuffer", "(I)Ljava/nio/ByteBuffer;");
    jc->codec_getOutputBuffer = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "getOutputBuffer", "(I)Ljava/nio/ByteBuffer;");
    jc->codec_releaseOutPutBuffer = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "releaseOutPutBuffer",  "(I)V");
    jc->codec_release = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "release", "()V");
    jc->codec_formatChange = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "formatChange", "()Ljava/nio/ByteBuffer;");
    jc->codec_dequeueOutputBufferIndex = (*jniEnv)->GetStaticMethodID(jniEnv, java_HwDecodeBridge, "dequeueOutputBufferIndex", "(J)Ljava/nio/ByteBuffer;");
    jc->HwDecodeBridge = (*jniEnv)->NewGlobalRef(jniEnv, java_HwDecodeBridge);
    (*jniEnv)->DeleteLocalRef(jniEnv, java_HwDecodeBridge);

    jclass java_SurfaceTextureBridge = (*jniEnv)->FindClass(jniEnv, "com/wodekouwei/srsrtmpplayer/proxy/SurfaceTextureWrapper");
    jc->texture_getSurface = (*jniEnv)->GetStaticMethodID(jniEnv, java_SurfaceTextureBridge, "getSurface", "(I)Landroid/view/Surface;");
    jc->texture_updateTexImage = (*jniEnv)->GetStaticMethodID(jniEnv, java_SurfaceTextureBridge, "updateTexImage", "()V");
    jc->texture_getTransformMatrix = (*jniEnv)->GetStaticMethodID(jniEnv, java_SurfaceTextureBridge, "getTransformMatrix",  "()[F");
    jc->texture_release = (*jniEnv)->GetStaticMethodID(jniEnv, java_SurfaceTextureBridge, "release", "()V");
    jc->SurfaceTextureBridge = (*jniEnv)->NewGlobalRef(jniEnv, java_SurfaceTextureBridge);
    (*jniEnv)->DeleteLocalRef(jniEnv, java_SurfaceTextureBridge);

    *p_jc = jc;
}

void oar_jni_free(oar_java_class **p_jc, JNIEnv *jniEnv){
    oar_java_class * jc = *p_jc;
    (*jniEnv)->DeleteGlobalRef(jniEnv, jc->HwDecodeBridge);
    (*jniEnv)->DeleteGlobalRef(jniEnv, jc->SurfaceTextureBridge);
    free(jc);
    *p_jc = NULL;
}