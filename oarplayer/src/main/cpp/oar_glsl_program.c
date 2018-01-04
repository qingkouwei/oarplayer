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
#define _JNILOG_TAG "glsl_program"
#include "_android.h"
#include <malloc.h>
#include <string.h>
#include "oar_glsl_program.h"
#include "oar_texture.h"

#define STR(s) #s



static int video_width = 0, video_height = 0;

static inline void resize_video(oar_model *model){
    int screen_width = model->viewport_w, screen_height = model->viewport_h;
    if(screen_width == 0
       || screen_height == 0
       || video_width == 0
       || video_height == 0
            ){
        model->x_scale = 1.0f;
        model->y_scale = 1.0f;
    }
    float screen_rate = (float)screen_width / (float)screen_height;
    float video_rate = (float)video_width / (float)video_height;
    if(model->frame_rotation == OAR_ROTATION_90 || model->frame_rotation == OAR_ROTATION_270){
        screen_rate = (float)screen_height / (float)screen_width;
    }
    if(screen_rate > video_rate){
        model->x_scale = video_rate / screen_rate;
        model->y_scale = 1.0f;
    }else{
        model->x_scale = 1.0f;
        model->y_scale = screen_rate / video_rate;
    }
}

void model_rect_resize(oar_model *model, int w, int h) {
    model->viewport_w = w;
    model->viewport_h = h;
    resize_video(model);
}

static void draw_rect(oar_model *model){
    oar_glsl_program * program = model->program;
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, model->viewport_w, model->viewport_h);
    model->bind_texture(model);
    glUniform1f(program->linesize_adjustment_location, model->width_adjustment);
    glUniform1f(program->x_scale_location, model->x_scale);
    glUniform1f(program->y_scale_location, model->y_scale);
    glUniform1i(program->frame_rotation_location, model->frame_rotation);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vbos[2]);
    glDrawElements(GL_TRIANGLES, (GLsizei) model->elementsCount, GL_UNSIGNED_INT, 0);
}

void update_frame_rect(oar_model *model, OARFrame * frame){
    //LOGI("model format = %d, frame format = %d", model->pixel_format, frame->format);
    if(model->pixel_format != frame->format){
        model->pixel_format = (OARPixelFormat)frame->format;
        model->program = oar_glsl_program_get(model->pixel_format);
        glUseProgram(model->program->program);
        switch(frame->format){
            case PIX_FMT_YUV420P:
                model->bind_texture = bind_texture_yuv420p;
                model->updateTexture = update_texture_yuv420p;
                break;
            case PIX_FMT_NV12:
                model->bind_texture = bind_texture_nv12;
                model->updateTexture = update_texture_nv12;
                break;
            case PIX_FMT_EGL_EXT:
                model->bind_texture = bind_texture_oes;
                model->updateTexture = update_texture_oes;
                break;
            default:
                LOGE("not support this pix_format ==> %d", frame->format);
                return;
        }
        glBindBuffer(GL_ARRAY_BUFFER, model->vbos[0]);
        glVertexAttribPointer((GLuint) model->program->positon_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray((GLuint) model->program->positon_location);
        glBindBuffer(GL_ARRAY_BUFFER, model->vbos[1]);
        glVertexAttribPointer((GLuint) model->program->texcoord_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray((GLuint) model->program->texcoord_location);
    }
    model->updateTexture(model, frame);
    // some video linesize > width
    model->width_adjustment = 1/*(float)frame->width / (float)frame->linesize[0]*/;
    if(frame->width != video_width
       || frame->height != video_height
       || frame->FRAME_ROTATION != model->frame_rotation
            ){
        video_width = frame->width;
        video_height = frame->height;
        model->frame_rotation = frame->FRAME_ROTATION;
        resize_video(model);
    }
}
static oar_mesh *get_rect_mesh() {
    oar_mesh *rect_mesh;
    rect_mesh = (oar_mesh *) malloc(sizeof(oar_mesh));
    memset(rect_mesh, 0, sizeof(oar_mesh));

    rect_mesh->ppLen = 12;
    rect_mesh->ttLen = 8;
    rect_mesh->indexLen = 6;
    rect_mesh->pp = (float *) malloc((size_t) (sizeof(float) * rect_mesh->ppLen));
    rect_mesh->tt = (float *) malloc((size_t) (sizeof(float) * rect_mesh->ttLen));
    rect_mesh->index = (unsigned int *) malloc(sizeof(unsigned int) * rect_mesh->indexLen);
    float pa[12] = {-1, 1, -0.1f, 1, 1, -0.1f, 1, -1, -0.1f, -1, -1, -0.1f};
    float ta[8] = {0, 1, 1, 1, 1, 0, 0, 0};
    unsigned int ia[6] = {0, 1, 2, 0, 2, 3};
    for (int i = 0; i < 12; ++i) {
        *rect_mesh->pp++ = pa[i];
    }
    for (int i = 0; i < 8; ++i) {
        *rect_mesh->tt++ = ta[i];
    }
    for (int i = 0; i < 6; ++i) {
        *rect_mesh->index++ = ia[i];
    }
    rect_mesh->pp -= rect_mesh->ppLen;
    rect_mesh->tt -= rect_mesh->ttLen;
    rect_mesh->index -= rect_mesh->indexLen;
    return rect_mesh;
}
void free_mesh(oar_mesh *p) {
    if (p != NULL) {
        if (p->index != NULL) {
            free(p->index);
        }
        if (p->pp != NULL) {
            free(p->pp);
        }
        if (p->tt != NULL) {
            free(p->tt);
        }
        free(p);
    }
}
oar_model *createModel(){
    oar_model *model = (oar_model *) malloc(sizeof(oar_model));
    model->program = NULL;
    model->draw = NULL;
    model->updateTexture = NULL;
    model->resize = model_rect_resize;
    model->update_frame = update_frame_rect;
    model->draw = draw_rect;
    model->pixel_format = PIX_FMT_NONE;
    model->width_adjustment = 1;
    model->x_scale = 1.0f;
    model->y_scale = 1.0f;
    model->frame_rotation = OAR_ROTATION_0;

    model->updateModelRotation = NULL;

    oar_mesh *rect = get_rect_mesh();
    glGenBuffers(3, model->vbos);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * rect->ppLen, rect->pp, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * rect->ttLen, rect->tt, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vbos[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * rect->indexLen, rect->index,
                 GL_STATIC_DRAW);
    model->elementsCount = (size_t) rect->indexLen;
    free_mesh(rect);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    return model;
}
void freeModel(oar_model *model){
    if(model != NULL){
        glDeleteBuffers(3, model->vbos);
        free(model);
    }
}

static void init_rect_nv12();
static void init_rect_yuv_420p();
static void init_rect_oes();


static const char * vs_rect = STR(
    attribute vec3 position;
    attribute vec2 texcoord;
    uniform float width_adjustment;
    uniform float x_scale;
    uniform float y_scale;
    uniform int frame_rotation;
    varying vec2 tx;

    const mat2 rotation90 = mat2(
            0.0, -1.0,
            1.0, 0.0
    );
    const mat2 rotation180 = mat2(
            -1.0, 0.0,
            0.0, -1.0
    );
    const mat2 rotation270 = mat2(
            0.0, 1.0,
            -1.0, 0.0
    );
    void main(){
        tx = vec2(texcoord.x * width_adjustment, texcoord.y);
        vec2 xy = vec2(position.x * x_scale, position.y * y_scale);
        if(frame_rotation == 1){
            xy = rotation90 * xy;
        }else if(frame_rotation == 2){
            xy = rotation180 * xy;
        }else if(frame_rotation == 3){
            xy = rotation270 * xy;
        }
        gl_Position = vec4(xy, position.z, 1.0);
    }
);

static const char * fs_egl_ext = "#extension GL_OES_EGL_image_external : require\n"
"precision mediump float;\n"
"uniform mat4 tx_matrix;\n"
"uniform samplerExternalOES tex_y;\n"
"varying vec2 tx;\n"
"void main(){\n"
"    vec2 tx_transformed = (tx_matrix * vec4(tx, 0, 1.0)).xy;\n"
"    gl_FragColor = texture2D(tex_y, tx_transformed);\n"
"}\n";

static const char * fs_nv12 = STR(
    precision mediump float;
    varying vec2 tx;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    void main(void)
    {
        vec2 tx_flip_y = vec2(tx.x, 1.0 - tx.y);
        float y = texture2D(tex_y, tx_flip_y).r;
        vec4 uv = texture2D(tex_u, tx_flip_y);
        float u = uv.r - 0.5;
        float v = uv.a - 0.5;
        float r = y +             1.402 * v;
        float g = y - 0.344 * u - 0.714 * v;
        float b = y + 1.772 * u;
        gl_FragColor = vec4(r, g, b, 1.0);
    }
);

static const char * fs_yuv_420p = STR(
    precision mediump float;
    varying vec2 tx;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;
    void main(void)
    {
        vec2 tx_flip_y = vec2(tx.x, 1.0 - tx.y);
        float y = texture2D(tex_y, tx_flip_y).r;
        float u = texture2D(tex_u, tx_flip_y).r - 0.5;
        float v = texture2D(tex_v, tx_flip_y).r - 0.5;
        float r = y +             1.402 * v;
        float g = y - 0.344 * u - 0.714 * v;
        float b = y + 1.772 * u;
        gl_FragColor = vec4(r, g, b, 1.0);
    }
);


static oar_glsl_program rect_nv12 = {
        .has_init = 0,
        .init = init_rect_nv12
};
static oar_glsl_program rect_yuv_420p = {
        .has_init = 0,
        .init = init_rect_yuv_420p
};
static oar_glsl_program rect_oes = {
        .has_init = 0,
        .init = init_rect_oes
};


static GLuint loadShader(GLenum shaderType, const char * shaderSrc){
    GLuint shader;
    GLint compiled;
    shader = glCreateShader(shaderType);
    if(shader == 0) return 0;
    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled){
        GLint infoLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 0){
            char * infoLog = (char *)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            LOGE("compile shader error ==>\n%s\n\n%s\n", shaderSrc, infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}


static GLuint loadProgram(const char * vsSrc, const char * fsSrc){
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fsSrc);
    if(vertexShader == 0 || fragmentShader == 0) return 0;
    GLint linked;
    GLuint pro = glCreateProgram();
    if(pro == 0){
        LOGE("create program error!");
    }
    glAttachShader(pro, vertexShader);
    glAttachShader(pro, fragmentShader);
    glLinkProgram(pro);
    glGetProgramiv(pro, GL_LINK_STATUS, &linked);
    if(!linked){
        GLint infoLen;
        glGetProgramiv(pro, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 0){
            char * infoLog = (char *)malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(pro, infoLen, NULL, infoLog);
            LOGE("link program error ==>\n%s\n", infoLog);
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(pro);
        return 0;
    }
    return pro;
}

static void init_rect_nv12(){
    GLuint pro = loadProgram(vs_rect, fs_nv12);
    rect_nv12.program = pro;
    rect_nv12.positon_location = glGetAttribLocation(pro, "position");
    rect_nv12.texcoord_location = glGetAttribLocation(pro, "texcoord");
    rect_nv12.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    rect_nv12.x_scale_location = glGetUniformLocation(pro, "x_scale");
    rect_nv12.y_scale_location = glGetUniformLocation(pro, "y_scale");
    rect_nv12.frame_rotation_location = glGetUniformLocation(pro, "frame_rotation");
    rect_nv12.tex_y = glGetUniformLocation(pro, "tex_y");
    rect_nv12.tex_u = glGetUniformLocation(pro, "tex_u");
}

static void init_rect_yuv_420p(){
    GLuint pro = loadProgram(vs_rect, fs_yuv_420p);
    rect_yuv_420p.program = pro;
    rect_yuv_420p.positon_location = glGetAttribLocation(pro, "position");
    rect_yuv_420p.texcoord_location = glGetAttribLocation(pro, "texcoord");
    rect_yuv_420p.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    rect_yuv_420p.x_scale_location = glGetUniformLocation(pro, "x_scale");
    rect_yuv_420p.y_scale_location = glGetUniformLocation(pro, "y_scale");
    rect_yuv_420p.frame_rotation_location = glGetUniformLocation(pro, "frame_rotation");
    rect_yuv_420p.tex_y = glGetUniformLocation(pro, "tex_y");
    rect_yuv_420p.tex_u = glGetUniformLocation(pro, "tex_u");
    rect_yuv_420p.tex_v = glGetUniformLocation(pro, "tex_v");
}

static void init_rect_oes(){
    GLuint pro = loadProgram(vs_rect, fs_egl_ext);
    rect_oes.program = pro;
    rect_oes.positon_location = glGetAttribLocation(pro, "position");
    rect_oes.texcoord_location = glGetAttribLocation(pro, "texcoord");
    rect_oes.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    rect_oes.x_scale_location = glGetUniformLocation(pro, "x_scale");
    rect_oes.y_scale_location = glGetUniformLocation(pro, "y_scale");
    rect_oes.frame_rotation_location = glGetUniformLocation(pro, "frame_rotation");
    rect_oes.tex_y = glGetUniformLocation(pro, "tex_y");
    rect_oes.texture_matrix_location = glGetUniformLocation(pro, "tx_matrix");
}



oar_glsl_program * oar_glsl_program_get(int pixel_format){
    oar_glsl_program * pro = NULL;
    switch(pixel_format){
        case PIX_FMT_NV12:
            pro = &rect_nv12;
            break;
        case PIX_FMT_YUV420P:
            pro = &rect_yuv_420p;
            break;
        case PIX_FMT_EGL_EXT:
            pro = &rect_oes;
            break;
        default:
            break;
    }

    if(pro != NULL && pro->has_init == 0){
        pro->init();
        pro->has_init = 1;
    }
    return pro;
}


void oar_glsl_program_clear_all(){
    glDeleteProgram(rect_nv12.program);
    rect_nv12.has_init = 0;

    glDeleteProgram(rect_yuv_420p.program);
    rect_yuv_420p.has_init = 0;

    glDeleteProgram(rect_oes.program);
    rect_oes.has_init = 0;
}