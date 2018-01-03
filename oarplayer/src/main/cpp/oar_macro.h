
#ifndef __OAR_MACRO_H
#define __OAR_MACRO_H


#define NDK_MEDIACODEC_VERSION 21


#define OAR_HAS_AUDIO_FLAG 0x1
#define OAR_HAS_VIDEO_FLAG 0x2

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
#define OAR_ERROR_AUDIO_DECODE_SEND_PACKET 3001
#define OAR_ERROR_AUDIO_DECODE_CODEC_NOT_OPENED 3002
#define OAR_ERROR_AUDIO_DECODE_RECIVE_FRAME 3003

#define OAR_ERROR_VIDEO_SW_DECODE_SEND_PACKET 4101
#define OAR_ERROR_VIDEO_SW_DECODE_CODEC_NOT_OPENED 4102
#define OAR_ERROR_VIDEO_SW_DECODE_RECIVE_FRAME 4103

#define OAR_ERROR_VIDEO_HW_MEDIACODEC_RECEIVE_FRAME 501

#endif //__OAR_MACRO_H