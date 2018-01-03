
#include <unistd.h>
#include "oar_player_gl_thread.h"
#include "oarplayer_type_def.h"
#include "oar_player.h"
#include "oar_frame_queue.h"
#include "oar_glsl_program.h"
#include "oar_texture.h"
#include <sys/prctl.h>
#include <string.h>
#include <pthread.h>

#define _JNILOG_TAG "oar_player_gl_thread"
#include "_android.h"
#include "oar_video_mediacodec.h"
#include "oar_clock.h"

static void init_egl(oarplayer * oar){
    oar_video_render_context *ctx = oar->video_render_ctx;
    const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES2_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE,
                              8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 0,
                              EGL_NONE};
    EGLint numConfigs;
    ctx->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint majorVersion, minorVersion;
    eglInitialize(ctx->display, &majorVersion, &minorVersion);
    eglChooseConfig(ctx->display, attribs, &ctx->config, 1, &numConfigs);
    ctx->surface = eglCreateWindowSurface(ctx->display, ctx->config, ctx->window, NULL);
    EGLint attrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    ctx->context = eglCreateContext(ctx->display, ctx->config, NULL, attrs);
    EGLint err = eglGetError();
    if (err != EGL_SUCCESS) {
        LOGE("egl error");
    }
    if (eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context) == EGL_FALSE) {
        LOGE("------EGL-FALSE");
    }
    eglQuerySurface(ctx->display, ctx->surface, EGL_WIDTH, &ctx->width);
    eglQuerySurface(ctx->display, ctx->surface, EGL_HEIGHT, &ctx->height);
    initTexture(oar);
    if(!oar->is_sw_decode) {
        oar_java_class * jc = oar->jc;
        JNIEnv * jniEnv = oar->video_render_ctx->jniEnv;
        jobject surface_texture = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->SurfaceTextureBridge, jc->texture_getSurface, ctx->texture[3]);
        ctx->texture_window = ANativeWindow_fromSurface(jniEnv, surface_texture);
    }

}

static void release_egl(oarplayer * oar) {
    oar_video_render_context *ctx = oar->video_render_ctx;
    if (ctx->display == EGL_NO_DISPLAY) return;
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(ctx->display, ctx->surface);
    oar_texture_delete(oar);
    oar_glsl_program_clear_all();
    if(ctx->texture_window != NULL){
        ANativeWindow_release(ctx->texture_window);
        ctx->texture_window = NULL;
    }
    if (ctx->model != NULL) {
        freeModel(ctx->model);
        ctx->model = NULL;
    }
    eglMakeCurrent(ctx->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (ctx->context != EGL_NO_CONTEXT) {
        eglDestroyContext(ctx->display, ctx->context);
    }
    if (ctx->surface != EGL_NO_SURFACE) {
        eglDestroySurface(ctx->display, ctx->surface);
    }
    eglTerminate(ctx->display);
    ctx->display = EGL_NO_DISPLAY;
    ctx->context = EGL_NO_CONTEXT;
    ctx->surface = EGL_NO_SURFACE;
}

static void set_window(oarplayer * oar) {
    oar_video_render_context *ctx = oar->video_render_ctx;
    if(ctx->display == EGL_NO_DISPLAY){
        init_egl(oar);
    }else {
        glClear(GL_COLOR_BUFFER_BIT);
        eglSwapBuffers(ctx->display, ctx->surface);
        eglDestroySurface(ctx->display, ctx->surface);

        ctx->surface = eglCreateWindowSurface(ctx->display, ctx->config, ctx->window, NULL);
        EGLint err = eglGetError();
        if (err != EGL_SUCCESS) {
            LOGE("egl error");
        }
        if (eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context) == EGL_FALSE) {
            LOGE("------EGL-FALSE");
        }
        eglQuerySurface(ctx->display, ctx->surface, EGL_WIDTH, &ctx->width);
        eglQuerySurface(ctx->display, ctx->surface, EGL_HEIGHT, &ctx->height);
    }
}

void change_model(oar_video_render_context *ctx) {
    if (ctx->model != NULL) {
        freeModel(ctx->model);
    }
    ctx->model = createModel();
    memcpy(ctx->model->texture, ctx->texture, sizeof(GLuint) * 4);
    ctx->model->resize(ctx->model, ctx->width, ctx->height);
    ctx->draw_mode = wait_frame;
}

static inline void oar_player_release_video_frame(oarplayer *oar, OARFrame *frame) {
    if (!oar->is_sw_decode) {
        oar_video_mediacodec_release_buffer(oar, frame);
    }
    //TODO release frame
    oar->video_frame = NULL;
}
static inline void draw_now(oar_video_render_context *ctx) {
    oar_model *model = ctx->model;
    pthread_mutex_lock(ctx->lock);
    // The initialization is done
    if (model->pixel_format != PIX_FMT_NONE) {
        model->draw(model);
    }
    eglSwapBuffers(ctx->display, ctx->surface);
    pthread_mutex_unlock(ctx->lock);
}

/**
 *
 * @param oar
 * @param frame
 * @return  0   draw
 *         -1   sleep 33ms  continue
 *         -2   break
 */
static inline int draw_video_frame(oarplayer *oar) {
    // 上一次可能没有画， 这种情况就不需要取新的了
    if (oar->video_frame == NULL) {
        oar->video_frame = oar_frame_queue_get(oar->video_frame_queue);
    }
    // buffer empty  ==> sleep 10ms , return 0
    // eos           ==> return -2
    if (oar->video_frame == NULL) {
        LOGE("video_frame is null...");
        if (oar->eof) {
            return -2;
        } else {
            usleep(BUFFER_EMPTY_SLEEP_US);
            return 0;
        }
    }
    int64_t time_stamp = oar->video_frame->pts;


    int64_t diff = 0;
    //TODO 音视频同步
    if(oar->metadata->has_audio){
        diff = time_stamp - (oar->audio_clock->pts + oar->audio_player_ctx->get_delta_time(oar->audio_player_ctx));
    }else{
        diff = time_stamp - oar_clock_get(oar->video_clock);
    }
    LOGI("time_stamp:%lld, clock:%lld, diff:%lld",time_stamp , oar_clock_get(oar->video_clock), diff);
    oar_model *model = oar->video_render_ctx->model;


    // diff >= 33ms if draw_mode == wait_frame return -1
    //              if draw_mode == fixed_frequency draw previous frame ,return 0
    // diff > 0 && diff < 33ms  sleep(diff) draw return 0
    // diff <= 0  draw return 0
    if (diff >= WAIT_FRAME_SLEEP_US) {
        if (oar->video_render_ctx->draw_mode == wait_frame) {
            return -1;
        } else {
            draw_now(oar->video_render_ctx);
            return 0;
        }
    } else {
        // if diff > WAIT_FRAME_SLEEP_US   then  use previous frame
        // else  use current frame   and  release frame
//        LOGI("start draw...");
        pthread_mutex_lock(oar->video_render_ctx->lock);
        model->update_frame(model, oar->video_frame);
        pthread_mutex_unlock(oar->video_render_ctx->lock);
        oar_player_release_video_frame(oar, oar->video_frame);
        if(!oar->is_sw_decode){
            JNIEnv * jniEnv = oar->video_render_ctx->jniEnv;
            (*jniEnv)->CallStaticVoidMethod(jniEnv, oar->jc->SurfaceTextureBridge, oar->jc->texture_updateTexImage);
            jfloatArray texture_matrix_array = (*jniEnv)->CallStaticObjectMethod(jniEnv, oar->jc->SurfaceTextureBridge, oar->jc->texture_getTransformMatrix);
            (*jniEnv)->GetFloatArrayRegion(jniEnv, texture_matrix_array, 0, 16, model->texture_matrix);
            (*jniEnv)->DeleteLocalRef(jniEnv, texture_matrix_array);
        }

        if (diff > 0) usleep((useconds_t) diff);
        draw_now(oar->video_render_ctx);
        oar_clock_set(oar->video_clock, time_stamp);
        return 0;
    }
}

void *oar_player_gl_thread(void *data) {
    prctl(PR_SET_NAME, __func__);
    oarplayer *oar = (oarplayer *) data;
    oar_video_render_context *ctx = oar->video_render_ctx;
    (*oar->vm)->AttachCurrentThread(oar->vm, &ctx->jniEnv, NULL);
    int ret;
    while (oar->error_code == 0) {
        // 处理egl
        pthread_mutex_lock(ctx->lock);
        if (ctx->cmd != NO_CMD) {
            if ((ctx->cmd & CMD_SET_WINDOW) != 0) {
                set_window(oar);
                change_model(ctx);
            }
            ctx->cmd = NO_CMD;
        }
        pthread_mutex_unlock(ctx->lock);
        // 处理pd->status
        if (oar->status == PAUSED /*|| oar->status == BUFFER_EMPTY*/) {
            LOGE("gl thread sleep...");
            usleep(NULL_LOOP_SLEEP_US);
        } else if (oar->status == PLAYING|| oar->status == BUFFER_EMPTY) {
            LOGE("drawframe....");
            ret = draw_video_frame(oar);
            if (ret == 0) {
                continue;
            } else if (ret == -1) {
                usleep(WAIT_FRAME_SLEEP_US);
                continue;
            } else if (ret == -2) {
                // 如果有视频   就在这发结束信号
                oar->send_message(oar, oar_message_stop);
                break;
            }
        } else if (oar->status == IDEL) {
            usleep(WAIT_FRAME_SLEEP_US);
        } else {
            LOGE("error state  ==> %d", oar->status);
            break;
        }
    }
    release_egl(oar);
    (*oar->vm)->DetachCurrentThread(oar->vm);
    LOGI("thread ==> %s exit", __func__);
    return NULL;
}