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
// Created by qingkouwei on 2018/1/5.
//

#ifndef OARPLAYER_OAR_VIDEO_MEDIACODEC_CTX_H
#define OARPLAYER_OAR_VIDEO_MEDIACODEC_CTX_H

#include "oarplayer_type_def.h"

typedef struct H264ConvertState {
    uint32_t nal_len;
    uint32_t nal_pos;
} H264ConvertState;

oar_video_mediacodec_context * oar_create_video_mediacodec_context(
        oarplayer *oar);


void convert_h264_to_annexb( uint8_t *p_buf, size_t i_len,
                             size_t i_nal_size,
                             H264ConvertState *state );
int convert_sps_pps2(const uint8_t *p_buf, size_t i_buf_size,
                     uint8_t * out_sps_buf, size_t * out_sps_buf_size,
                     uint8_t * out_pps_buf, size_t * out_pps_buf_size,
                     size_t *p_nal_size
);
int convert_hevc_nal_units(const uint8_t *p_buf,size_t i_buf_size,
                           uint8_t *p_out_buf,size_t i_out_buf_size,
                           size_t *p_sps_pps_size,size_t *p_nal_size);

#endif //OARPLAYER_OAR_VIDEO_MEDIACODEC_CTX_H
