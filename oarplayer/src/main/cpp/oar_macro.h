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
#ifndef __OAR_MACRO_H
#define __OAR_MACRO_H


#define NDK_MEDIACODEC_VERSION 21


#define NSEC_PER_SEC  1000000000L
#define NSEC_PER_MSEC 1000000L
#define USEC_PER_SEC  1000000L
#define NSEC_PER_USEC 1000L
#define USEC_PRE_MSEC 1000L
#define MSEC_PRE_SEC 1000L

#define oar_metadata_video_width "width"
#define oar_metadata_video_height "height"
#define oar_metadata_video_rate "videodatarate"
#define oar_metadata_video_framerate "framerate"
#define oar_metadata_video_codecid "videocodecid"
#define oar_metadata_audio_rate "audiodatarate"
#define oar_metadata_audio_samplerate "audiosamplerate"
#define oar_metadata_audio_samplesize "audiosamplesize"
#define oar_metadata_audio_codecid "audiocodecid"


#define oar_message_stop 1
#define oar_message_buffer_empty 2
#define oar_message_buffer_full 3
#define oar_message_decoder_configuration 4
#define oar_message_error 999

#define default_buffer_size 1024*1024*5
#define default_buffer_time 5.0f
#define default_read_timeout 3.0f

// 100 ms
#define NULL_LOOP_SLEEP_US 100000
// 10 ms
#define BUFFER_EMPTY_SLEEP_US 10000
// 30 fps
#define WAIT_FRAME_SLEEP_US 33333

//#define OAR_PIX_FMT_EGL_EXT 10000

#define OAR_ROTATION_0 0
#define OAR_ROTATION_90 1
#define OAR_ROTATION_180 2
#define OAR_ROTATION_270 3


//////// rename avframe fields
#define HW_BUFFER_ID pkt_pos
// 0 : 0
// 1 : 90
// 2 : 180
// 3 : 270
#define FRAME_ROTATION sample_rate




// error code
#define OAR_ERROR_FORMAT_VIDEO_CODEC 2001
#define OAR_ERROR_FORMAT_AUDIO_CODEC 2002
#define OAR_ERROR_FORMAT_VIDEO_CONFIG 2003
#define OAR_ERROR_FORMAT_AUDIO_CONFIG 2004

#define OAR_ERROR_AUDIO_DECODE_SEND_PACKET 3001
#define OAR_ERROR_AUDIO_DECODE_CODEC_NOT_OPENED 3002
#define OAR_ERROR_AUDIO_DECODE_RECIVE_FRAME 3003

#define OAR_ERROR_VIDEO_SW_DECODE_SEND_PACKET 4101
#define OAR_ERROR_VIDEO_SW_DECODE_CODEC_NOT_OPENED 4102
#define OAR_ERROR_VIDEO_SW_DECODE_RECIVE_FRAME 4103

#define OAR_ERROR_VIDEO_HW_MEDIACODEC_RECEIVE_FRAME 501

#endif //__OAR_MACRO_H
