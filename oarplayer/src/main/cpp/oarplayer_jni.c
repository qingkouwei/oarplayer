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
#define _JNILOG_TAG "oarplayer_jni"
#include "_android.h"

#include <jni.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "jni_utils.h"
#include "oarplayer_type_def.h"
#include "oar_player.h"


#define JNI_CLASS_OARPLAYER     "com/wodekouwei/srsrtmpplayer/OARPlayer"
#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

static oarplayer *oar;
static int oar_run_android_version;
static int oar_best_samplerate;

typedef struct player_fields_t {
    pthread_mutex_t mutex;
    jclass clazz;
} player_fields_t;
static player_fields_t g_clazz;

static void
SrsPlayer_native_init(JNIEnv *env,jobject thiz,int run_android_version, int best_samplerate)
{
    oar_run_android_version = run_android_version;
    oar_best_samplerate = best_samplerate;
    oar = oar_player_create(env,thiz,oar_run_android_version,oar_best_samplerate);
    oar->jniEnv = env;
    (*env)->GetJavaVM(env, &oar->vm);
    LOGI("native init...");


}
static void
SrsPlayer_setDataSourceAndHeaders(
        JNIEnv *env, jobject thiz, jstring path,
        jobjectArray keys, jobjectArray values){
    const char *c_path = NULL;
    c_path = (*env)->GetStringUTFChars(env, path, NULL );
    int len = strlen(c_path);
    oar->url = malloc(sizeof(char)*len);
    strcpy(oar->url, c_path);
    (*env)->ReleaseStringUTFChars(env, path, c_path);
}
static void
SrsPlayer_setVideoSurface(JNIEnv *env, jobject thiz, jobject jsurface)
{
    if (oar != NULL) {
        if (oar->video_render_ctx->window != NULL) {
            ANativeWindow_release(oar->video_render_ctx->window);
        }
        ANativeWindow *sur = ANativeWindow_fromSurface(env, jsurface);
        oar->video_render_ctx->set_window(oar->video_render_ctx, sur);
    }
}
static void
SrsPlayer_prepareAsync(JNIEnv *env, jobject thiz)
{

}
static void
SrsPlayer_start(JNIEnv *env, jobject thiz)
{
    oar_player_play(oar);
}
static void
SrsPlayer_stop(JNIEnv *env, jobject thiz)
{
    oar_player_stop(oar);
}
static void
SrsPlayer_release(JNIEnv *env, jobject thiz){
    LOGI("release...");
    oar_player_release(oar);
    oar = NULL;
}
static float
SrsPlayer_getCurrentTime() {
    if (oar) {
        if (oar->metadata->has_audio) {
            return (float) oar->audio_clock->pts / 1000000;
        } else if (oar->metadata->has_video) {
            return (float) oar->video_clock->pts / 1000000;
        }
    }
    return 0.0f;
}
static void
SrsPlayer_setPlayBackground(jboolean playBackground){
    LOGI("setPlayBackground...");
    oar_player_set_play_background(oar, playBackground);
}
static void
SrsPlayer_setBufferTime(jfloat bufferTime){
    oar_player_set_buffer_time(oar, bufferTime);
}
static void
SrsPlayer_onPause(){
    LOGI("pause...");
    if (oar && oar->status == PLAYING) {
        oar->change_status(oar, PAUSED);
    }
}
static void
SrsPlayer_onResume(){
    if(oar && oar->status == PAUSED){
        oar_player_resume(oar);
    }
}
static JNINativeMethod g_methods[] = {
        {
                "_setDataSource",
                                        "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
                                                                                              (void *) SrsPlayer_setDataSourceAndHeaders
        },
        { "_setVideoSurface",       "(Landroid/view/Surface;)V", (void *) SrsPlayer_setVideoSurface },
        { "_prepareAsync",          "()V",      (void *) SrsPlayer_prepareAsync },
        { "_setBufferTime",         "(F)V",     (void *)SrsPlayer_setBufferTime},
        { "_start",                 "()V",      (void *) SrsPlayer_start },
        { "_setPlayBackground",     "(Z)V",      (void *) SrsPlayer_setPlayBackground },
        { "_getCurrentTime",        "()F",      (void *) SrsPlayer_getCurrentTime },
        { "_onPause",               "()V",      (void * ) SrsPlayer_onPause },
        { "_onResume",               "()V",      (void * ) SrsPlayer_onResume },
        { "_stop",                  "()V",      (void *) SrsPlayer_stop },
        { "_release",               "()V",      (void *) SrsPlayer_release },
        { "native_init",            "(II)V",      (void *) SrsPlayer_native_init },
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);

    pthread_mutex_init(&g_clazz.mutex, NULL );

    // FindClass returns LocalReference
    OAR_FIND_JAVA_CLASS(env, g_clazz.clazz, JNI_CLASS_OARPLAYER);
    (*env)->RegisterNatives(env, g_clazz.clazz, g_methods, NELEM(g_methods) );



    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    LOGE("JNI_OnUnload....");
    pthread_mutex_destroy(&g_clazz.mutex);
}

