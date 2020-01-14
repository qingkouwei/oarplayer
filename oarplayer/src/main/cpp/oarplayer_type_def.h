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
#ifndef __OARPLAYER_OARPLAYER_TYPE_DEF_H__
#define __OARPLAYER_OARPLAYER_TYPE_DEF_H__

#include <jni.h>
#include <android/native_window_jni.h>
#include <sys/types.h>
#include <stdbool.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

#include <android/looper.h>

#include "oar_macro.h"

struct oarplayer;
typedef struct oar_java_class {
//    jclass OARPlayer_class;
    jmethodID player_onPlayStatusChanged;
    jmethodID player_onPlayError;

    jclass HwDecodeBridge;
    jmethodID codec_init;
    jmethodID codec_stop;
    jmethodID codec_flush;
    jmethodID codec_dequeueInputBuffer;
    jmethodID codec_queueInputBuffer;
    jmethodID codec_getInputBuffer;
    jmethodID codec_dequeueOutputBufferIndex;
    jmethodID codec_formatChange;
    __attribute__((unused))
    jmethodID codec_getOutputBuffer;
    jmethodID codec_releaseOutPutBuffer;
    jmethodID codec_release;

    jclass HwAudioDecodeBridge;
    jmethodID audio_codec_init;
    jmethodID audio_codec_stop;
    jmethodID audio_codec_flush;
    jmethodID audio_codec_dequeueInputBuffer;
    jmethodID audio_codec_queueInputBuffer;
    jmethodID audio_codec_getInputBuffer;
    jmethodID audio_codec_dequeueOutputBufferIndex;
    jmethodID audio_codec_formatChange;
    jmethodID audio_codec_getOutputBuffer;
    jmethodID audio_codec_releaseOutPutBuffer;
    jmethodID audio_codec_release;

    jclass SurfaceTextureBridge;
    jmethodID texture_getSurface;
    jmethodID texture_updateTexImage;
    jmethodID texture_getTransformMatrix;
    __attribute__((unused))
    jmethodID texture_release;

} oar_java_class;
typedef struct oar_dl_context{
    void *libHandler;
    int (*native_mediacodec_send_packet)(void * codec,
                                          int len,
                                          int type,
                                          int64_t dts,
                                          int64_t pts,
                                          int isKeyframe,
                                          uint8_t *data);
    void (*native_mediacodec_release_buffer)(void * codec, int bufferID, bool render);
    int (*native_mediacodec_receive_frame)(void * codec,
                                           void **frame,
                                           void * oar,
                                           int type,
                                            void *(frameGenerate)(void *, void**, void*, int, int64_t,ssize_t,int, int, int));
    void (*native_mediacodec_flush)(void * codec);
    void *(*create_native_mediacodec)(int codec_id,
                                               int width, int height,
                                               int sample_rate, int channelCount,
                                               uint8_t *sps, int sps_size,
                                               uint8_t *pps, int pps_size,
                                              void *ctx,
                                               void (*formatCreated(void*, void*)));
    int (*native_mediacodec_start)(void * codec, void *format, void *window);
    void (*native_mediacodec_stop)(void * codec);
    void (*native_mediacodec_release_context)(void * codec, void *format);
} oar_dl_context;
typedef enum{
    PIX_FMT_NONE = -1,
    PIX_FMT_YUV420P,
    PIX_FMT_NV12,
    PIX_FMT_EGL_EXT
} OARPixelFormat;

typedef struct {
    float* pp;
    float* tt;
    unsigned int * index;
    int ppLen, ttLen, indexLen;
} oar_mesh;

typedef struct OARFrame OARFrame;
#define NO_CMD 0
#define CMD_SET_WINDOW 1
typedef enum oar_draw_mode {
    wait_frame, fixed_frequency
} oar_draw_mode;
typedef struct oar_glsl_program {
    uint8_t has_init;

    void (*init)();

    GLuint program;
    GLint positon_location;
    GLint texcoord_location;
    GLint linesize_adjustment_location;
    GLint x_scale_location;
    GLint y_scale_location;
    GLint frame_rotation_location;
    GLint modelMatrixLoc;
    GLint viewMatrixLoc;
    GLint projectionMatrixLoc;
    GLint tex_y, tex_u, tex_v;
    GLint texture_matrix_location;
} oar_glsl_program;
typedef struct oar_model {
    OARPixelFormat pixel_format;
    GLuint vbos[3];
    size_t elementsCount;
    GLuint texture[4];
    oar_glsl_program * program;

    GLfloat modelMatrix[16];
    GLfloat view_matrix[16];
    GLfloat projectionMatrix[16];
    GLfloat head_matrix[16];
    GLfloat texture_matrix[16];
    float width_adjustment;
    int viewport_w, viewport_h;

    // only RECT Model use
    float x_scale, y_scale;
    int frame_rotation;

    void (*draw)(struct oar_model *model);

    void (*resize)(struct oar_model *model, int w, int h);

    void (*updateTexture)(struct oar_model *model, OARFrame *frame);

    void (*bind_texture)(struct oar_model *model);

    void (*update_frame)(struct oar_model *model, OARFrame *frame);

    // rx ry rz ∈ [-2π, 2π]
    void (*updateModelRotation)(struct oar_model *model, GLfloat rx, GLfloat ry, GLfloat rz,bool enable_tracker);

} oar_model;
typedef struct oar_video_render_context {
    JNIEnv *jniEnv;
    int width, height;
    EGLConfig config;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    struct ANativeWindow *window;
    struct ANativeWindow *texture_window;
    GLuint texture[4];
    oar_model *model;
    pthread_mutex_t *lock;
    oar_draw_mode draw_mode;

    uint8_t cmd;
    jfloat require_model_scale;
    float require_model_rotation[3];

    void (*set_window)(struct oar_video_render_context *ctx, struct ANativeWindow *window);
} oar_video_render_context;

typedef struct oar_clock {
    int64_t update_time;
    int64_t pts;
} oar_clock;

typedef enum {
    UNINIT = -1, IDEL = 0, PLAYING, PAUSED, BUFFER_EMPTY, BUFFER_FULL
} PlayStatus;

typedef enum {
    PktType_Video = 0,
    PktType_Audio = 1,
    PktType_VConf = 2,
    PktType_AConf = 3
}PktType_e;
typedef struct oar_metadata_t{
    int video_codec;
    int has_video;

    int width;
    int height;
    int fps;
    int video_bitrate;
    int rotate;
    char *video_extradata;
    int video_extradata_size;

    int audio_codec;
    int has_audio;

    int sample_rate;
    int sample_format;
    int channels;
    uint64_t channel_layout;
    int audio_bitrate;
    char* audio_pps;
    int audio_pps_size;

} oar_metadata_t;
typedef struct OARPacket {
    int size;
    PktType_e type;
    int64_t dts;
    int64_t pts;
    int isKeyframe;
    struct OARPacket *next;
    uint8_t data[0];
}OARPacket;
typedef struct oar_packet_queue {
    PktType_e media_type;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    OARPacket *cachedPackets;//队列首地址
    OARPacket *lastPacket;//队列最后一个元素

    int count;
    int total_bytes;
    uint64_t max_duration;

    void (*full_cb)(void *);

    void (*empty_cb)(void *);

    void *cb_data;
} oar_packet_queue;
typedef struct OARFrame {
    int size;
    PktType_e type;
    int64_t dts;
    int64_t pts;
    int format;
    int width;
    int height;
    int64_t pkt_pos;
    int sample_rate;
    struct OARFrame *next;
    uint8_t data[0];
}OARFrame;
typedef struct oar_frame_queue {
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    OARFrame *cachedFrames;
    OARFrame *lastFrame;
    int count;
    unsigned int size;
} oar_frame_queue;
typedef enum {
    VIDEO_CODEC_H263 = 2,
    VIDEO_CODEC_Screenvideo = 3,
    VIDEO_CODEC_On2VP6 = 4,
    VIDEO_CODEC_On2VP6AlphaChannel = 5,
    VIDEO_CODEC_Screenvideo2 = 6,
    VIDEO_CODEC_AVC = 7
}VideoCodecID;
typedef enum {
    AUDIO_CODEC_LPCMPE = 0,//Linear PCM, platform endian
    AUDIO_CODEC_ADPCM = 1, //ADPCM
    AUDIO_CODEC_MP3 = 2,//MP3
    AUDIO_CODEC_LPCMLE = 3, //Linear PCM, little endian
    AUDIO_CODEC_AAC = 10
}AudioCodecID;
typedef struct oar_video_mediacodec_context {
    JNIEnv *jniEnv;
    size_t nal_size;
    int width, height;
    OARPixelFormat pix_format;
    VideoCodecID codec_id;
    void *AFormat;
    void *ACodec;
    void (*oar_video_mediacodec_release_buffer)(struct oarplayer *oar, int index);
    int (*oar_video_mediacodec_receive_frame)(struct oarplayer *oar, OARFrame *frame);
    int (*oar_video_mediacodec_send_packet)(struct oarplayer *oar, OARPacket *packet);
    void (*oar_video_mediacodec_flush)(struct oarplayer *oar);
    void (*oar_video_mediacodec_release_context)(struct oarplayer *oar);
    void (*oar_video_mediacodec_start)(struct oarplayer *oar);
    void (*oar_video_mediacodec_stop)(struct oarplayer *oar);
} oar_video_mediacodec_context;
typedef struct oar_audio_mediacodec_context {
    JNIEnv *jniEnv;
    int channel_count;
    int sample_rate;
    AudioCodecID codec_id;
    void *AFormat;
    void *ACodec;
    void (*oar_audio_mediacodec_release_buffer)(struct oarplayer *oar, int index);
    int (*oar_audio_mediacodec_receive_frame)(struct oarplayer *oar, OARFrame **frame);
    int (*oar_audio_mediacodec_send_packet)(struct oarplayer *oar, OARPacket *packet);
    void (*oar_audio_mediacodec_flush)(struct oarplayer *oar);
    void (*oar_audio_mediacodec_release_context)(struct oarplayer *oar);
    void (*oar_audio_mediacodec_start)(struct oarplayer *oar);
    void (*oar_audio_mediacodec_stop)(struct oarplayer *oar);
} oar_audio_mediacodec_context;

typedef struct oar_audio_player_context {
    pthread_mutex_t *lock;
    unsigned int play_pos;
    int buffer_size;
    uint8_t * buffer;
    int frame_size;

    void (*play)(struct oarplayer *oar);

    void (*shutdown)();

    void (*release)(struct oar_audio_player_context * ctx);

    void (*player_create)(int rate, int channel, struct oarplayer *oar);

    int64_t (*get_delta_time)(struct oar_audio_player_context * ctx);
} oar_audio_player_context;

typedef struct oarplayer {
    JavaVM *vm;
    JNIEnv *jniEnv;
    int run_android_version;
    int best_samplerate;
    jobject *oarPlayer;
    oar_java_class *jc;

    //用户设置
    float buffer_time_length;

    char *url;

    // 播放器状态
    PlayStatus status;

    oar_metadata_t *metadata;
    oar_dl_context *dl_context;


    // 各个thread
    pthread_t read_stream_thread;
    pthread_t audio_decode_thread;
    pthread_t video_decode_thread;
    pthread_t gl_thread;

    // packet容器
    oar_packet_queue *video_packet_queue, *audio_packet_queue;
    // frame容器
    oar_frame_queue *audio_frame_queue;

    oar_frame_queue *video_frame_queue;

    // 音频
    oar_audio_player_context *audio_player_ctx;
    OARFrame *audio_frame;
    oar_audio_mediacodec_context *audio_mediacodec_ctx;

    // 软硬解公用
    oar_video_render_context *video_render_ctx;
    OARFrame *video_frame;
    int frame_rotation;

    // 硬解
    oar_video_mediacodec_context *video_mediacodec_ctx;

    // 音视频同步
    oar_clock *video_clock;
    oar_clock *audio_clock;


    // play background
    bool just_audio;

    // error code
    // -1 stop by user
    // -2 read stream time out
    // 1xx init
    // 2xx format and stream
    // 3xx audio decode
    // 4xx video decode sw
    // 5xx video decode hw
    // 6xx audio play
    // 7xx video play  openGL
    int error_code;

    // 统计
    //TODO

    // message
    ALooper *main_looper;
    int pipe_fd[2];

    void (*send_message)(struct oarplayer *oar, int message);

    void (*change_status)(struct oarplayer *oar, PlayStatus status);

    void (*on_error)(struct oarplayer * oar, int error_code);

} oarplayer;

#endif //__OARPLAYER_OARPLAYER_TYPE_DEF_H__
