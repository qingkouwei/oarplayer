
#ifndef OAR_VIDEO_RENDER_H
#define OAR_VIDEO_RENDER_H


#include "oarplayer_type_def.h"
void oar_video_render_ctx_reset(oar_video_render_context * ctx);
oar_video_render_context * oar_video_render_ctx_create();
void oar_video_render_ctx_release(oar_video_render_context * ctx);

#endif //OAR_VIDEO_RENDER_H
