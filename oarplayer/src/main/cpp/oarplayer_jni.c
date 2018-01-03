#include <jni.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define _JNILOG_TAG "oarplayer_jni"
#include "_android.h"
#include "jni_utils.h"

#include "oarplayer_type_def.h"
#include "oar_player.h"


/**
 * Error:(35133, 43) error:
 * invalid suffix on literal; C++11 requires a space between literal and identifier [-Wreserved-user-defined-literal]
 */

#define JNI_CLASS_OARPLAYER     "com/wodekouwei/srsrtmpplayer/OARPlayer"

static oarplayer *oar;
static int oar_run_android_version;
static int oar_best_samplerate;

typedef struct player_fields_t {
    pthread_mutex_t mutex;
    jclass clazz;
} player_fields_t;
static player_fields_t g_clazz;

static void
SrsPlayer_native_init(JNIEnv *env,int run_android_version, int best_samplerate)
{
    oar_run_android_version = run_android_version;
    oar_best_samplerate = best_samplerate;
    LOGI("native init...");

}
static void
SrsPlayer_setDataSourceAndHeaders(
        JNIEnv *env, jobject thiz, jstring path,
        jobjectArray keys, jobjectArray values){
    oar = oar_player_create(env,thiz,oar_run_android_version,oar_best_samplerate);
    oar->jniEnv = env;
    (*env)->GetJavaVM(env, &oar->vm);
    LOGE("vm = %p, tid = %d", oar->vm, gettid());
    const char *c_path = NULL;
    c_path = (*env)->GetStringUTFChars(env, path, NULL );
    //LOGE("path : %s", c_path);
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
}
static JNINativeMethod g_methods[] = {
        {
                "_setDataSource",
                                        "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
                                                                                              (void *) SrsPlayer_setDataSourceAndHeaders
        },
        { "_setVideoSurface",       "(Landroid/view/Surface;)V", (void *) SrsPlayer_setVideoSurface },
        { "_prepareAsync",          "()V",      (void *) SrsPlayer_prepareAsync },
        { "_start",                 "()V",      (void *) SrsPlayer_start },
        { "_stop",                  "()V",      (void *) SrsPlayer_stop },
        { "native_init",            "(II)V",      (void *) SrsPlayer_native_init },
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;

    //g_jvm = vm;
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

    pthread_mutex_destroy(&g_clazz.mutex);
}

