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
