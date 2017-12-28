

#ifndef __OAR_TEXTURE_H__
#define __OAR_TEXTURE_H__
#include "oarplayer_type_def.h"


void initTexture(oarplayer * oar);
void oar_texture_delete(oarplayer * oar);
void update_texture_yuv420p(oar_model *model, OARFrame *frame);
void update_texture_nv12(oar_model *model, OARFrame *frame);
void bind_texture_yuv420p(oar_model *model);
void bind_texture_nv12(oar_model *model);
void bind_texture_oes(oar_model *model);
void update_texture_oes(oar_model *model, OARFrame *frame);

#endif //__OAR_TEXTURE_H__
