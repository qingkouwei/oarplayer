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

#ifndef __OAR_VIDEO_MEDIACODEC_H__
#define __OAR_VIDEO_MEDIACODEC_H__

#include <string.h>
#include <limits.h>
#include "oarplayer_type_def.h"
#include "endian.h"


void oar_video_mediacodec_release_buffer(oarplayer *oar, int index);
int oar_video_mediacodec_receive_frame(oarplayer *oar, OARFrame *frame);
int oar_video_mediacodec_send_packet(oarplayer *oar, OARPacket *packet);
void oar_video_mediacodec_flush(oarplayer *oar);
void oar_video_mediacodec_release_context(oarplayer *oar);
void oar_video_mediacodec_start(oarplayer *oar);
void oar_video_mediacodec_stop(oarplayer *oar);

#endif //__OAR_VIDEO_MEDIACODEC_H__
