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
//
// Created by 申俊伟 on 2018/1/4.
//

#ifndef OARPLAYER_OAR_AUDIO_MEDIACODEC_NDK_H
#define OARPLAYER_OAR_AUDIO_MEDIACODEC_NDK_H

#include "oarplayer_type_def.h"

void oar_create_audio_mediacodec_ndk(
        oarplayer *oar);
void oar_audio_mediacodec_release_buffer_ndk(oarplayer *oar, int index);
int oar_audio_mediacodec_receive_frame_ndk(oarplayer *oar, OARFrame **frame);
int oar_audio_mediacodec_send_packet_ndk(oarplayer *oar, OARPacket *packet);
void oar_audio_mediacodec_flush_ndk(oarplayer *oar);
void oar_audio_mediacodec_release_context_ndk(oarplayer *oar);
void oar_audio_mediacodec_start_ndk(oarplayer *oar);
void oar_audio_mediacodec_stop_ndk(oarplayer *oar);
#endif //OARPLAYER_OAR_AUDIO_MEDIACODEC_NDK_H