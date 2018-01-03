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
#include <pthread.h>
#include <malloc.h>
#include "oar_video_render.h"

void oar_video_render_set_window(oar_video_render_context *ctx, struct ANativeWindow *window) {
    pthread_mutex_lock(ctx->lock);
    ctx->window = window;
    ctx->cmd |= CMD_SET_WINDOW;
    pthread_mutex_unlock(ctx->lock);
}


void oar_video_render_ctx_reset(oar_video_render_context * ctx){
    ctx->surface = EGL_NO_SURFACE;
    ctx->context = EGL_NO_CONTEXT;
    ctx->display = EGL_NO_DISPLAY;
    ctx->model = NULL;
    ctx->require_model_scale = 1;
    ctx->cmd = NO_CMD;
    ctx->draw_mode = wait_frame;
    ctx->require_model_rotation[0] = 0;
    ctx->require_model_rotation[1] = 0;
    ctx->require_model_rotation[2] = 0;
    ctx->width = ctx->height = 1;
}


oar_video_render_context *oar_video_render_ctx_create() {
    oar_video_render_context *ctx = malloc(sizeof(oar_video_render_context));
    ctx->lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(ctx->lock, NULL);
    ctx->set_window = oar_video_render_set_window;
    ctx->window = NULL;
    ctx->texture_window = NULL;
    oar_video_render_ctx_reset(ctx);
    return ctx;
}

void oar_video_render_ctx_release(oar_video_render_context *ctx) {
    pthread_mutex_destroy(ctx->lock);
    free(ctx->lock);
    free(ctx);
}