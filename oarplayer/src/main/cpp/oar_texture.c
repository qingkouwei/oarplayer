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
#include "oar_texture.h"

void initTexture(oarplayer * oar) {
    oar_video_render_context * ctx = oar->video_render_ctx;
    glGenTextures(1, &ctx->texture[3]);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, ctx->texture[3]);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void oar_texture_delete(oarplayer * oar){
    oar_video_render_context * ctx = oar->video_render_ctx;
    glDeleteTextures(1, &ctx->texture[3]);
}

void update_texture_yuv420p(oar_model *model, OARFrame *frame){
    /*glBindTexture(GL_TEXTURE_2D, model->texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->linesize[0], frame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[0]);
    glBindTexture(GL_TEXTURE_2D, model->texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->linesize[1], frame->height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[1]);
    glBindTexture(GL_TEXTURE_2D, model->texture[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->linesize[2], frame->height / 2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[2]);*/
}

void bind_texture_yuv420p(oar_model *model){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model->texture[0]);
    glUniform1i(model->program->tex_y, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, model->texture[1]);
    glUniform1i(model->program->tex_u, 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, model->texture[2]);
    glUniform1i(model->program->tex_v, 2);
}

void update_texture_nv12(oar_model *model, OARFrame *frame){
    /*glBindTexture(GL_TEXTURE_2D, model->texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame->linesize[0], frame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->data[0]);
    glBindTexture(GL_TEXTURE_2D, model->texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, frame->linesize[1] / 2, frame->height / 2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, frame->data[1]);*/
}

void bind_texture_nv12(oar_model *model){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model->texture[0]);
    glUniform1i(model->program->tex_y, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, model->texture[1]);
    glUniform1i(model->program->tex_u, 1);
}

void bind_texture_oes(oar_model *model){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, model->texture[3]);
    glUniform1i(model->program->tex_y, 0);
    glUniformMatrix4fv(model->program->texture_matrix_location, 1, GL_FALSE, model->texture_matrix);
}

void update_texture_oes(__attribute__((unused)) oar_model *model, __attribute__((unused)) OARFrame *frame){

}