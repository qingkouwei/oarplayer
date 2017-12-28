
#ifndef __OAR_GLSL_PROGRAM_H__
#define __OAR_GLSL_PROGRAM_H__

#include "oarplayer_type_def.h"

oar_glsl_program * oar_glsl_program_get(int pixel_format);
void oar_glsl_program_clear_all();

oar_model *createModel();
void freeModel(oar_model *model);
#endif //__OAR_GLSL_PROGRAM_H__
