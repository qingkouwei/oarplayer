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

#define _JNILOG_TAG "srs_player"
#include "_android.h"
#include <malloc.h>
#include <string.h>
#include "srs_readthread.h"
#include "srs_librtmp.h"
#include "oarplayer_type_def.h"
#include "oar_packet_queue.h"

#define SRS_READ_TIMEOUT                    500
#define SRS_WRITE_TIMEOUT                   500
#define ERROR_SOCKET_READ                   1007
#define ERROR_SOCKET_TIMEOUT                1011

#define isDebug 1
#define _LOGD if(isDebug) LOGE

static int getSamplerate(int index){
    switch(index){
        case 0:
            return 96000;
        case 1:
            return 88200;
        case 2:
            return 64000;;
        case 3:
            return 48000;
        case 4:
            return 44100;;
        case 5:
            return 32000;
        case 6:
            return 24000;
        case 7:
            return 22050;
        case 8:
            return 16000;
        case 9:
            return 12000;
        case 10:
            return 11025;
        case 11:
            return 8000;
        case 12:
            return 7350;
        default:
            return -1;
    }
}

void *read_thread(void *data) {
    _LOGD("read thread start...");
    _LOGD("version: %d.%d.%d\n", srs_version_major(), srs_version_minor(), srs_version_revision());
    oarplayer *oar = data;
    srs_rtmp_t rtmp = srs_rtmp_create(oar->url);
    if(rtmp == NULL){
        LOGE("create srs rtmp failed.");
    }
    _LOGD("srs_rtmp_create success, url is : %s", oar->url);
    int ret = srs_rtmp_set_timeout(rtmp, SRS_READ_TIMEOUT, SRS_WRITE_TIMEOUT);
    if(ret != 0){
        LOGE("srs_rtmp_set_timeout failed:%d",ret);
    }
    ret = srs_rtmp_handshake(rtmp);
    if(ret != 0){
        LOGE("simple handshake failed:%d",ret);
        goto rtmp_destroy;
    }
    _LOGD("simple handshake success.");
    if(srs_rtmp_connect_app(rtmp) != 0){
        LOGE("connect vhost/app failed.");
        goto rtmp_destroy;
    }
    _LOGD("connect vhost/app success.");
    if(srs_rtmp_play_stream(rtmp) != 0){
        LOGE("play stream failed.");
        goto rtmp_destroy;
    }
    _LOGD("play stream success.");
    _LOGD("error_code:%d", oar->error_code);
    while(oar->error_code == 0){
        int size;
        char type;
        char* data;
        u_int32_t timestamp;
        int ret = srs_rtmp_read_packet(rtmp, &type, &timestamp, &data, &size);
        if ( ret != 0) {
            LOGE("read packet error, ret : %d", ret);
            if(ret == ERROR_SOCKET_TIMEOUT){
                //read timeout
            }else if(ret == ERROR_SOCKET_READ){
                //read error
            }
        }
        if(type == SRS_RTMP_TYPE_AUDIO){
            char audio_fromat_type = srs_utils_flv_audio_sound_format(data,size);//编码类型
            char audio_rate = srs_utils_flv_audio_sound_rate(data,size);//采样率
            char sampe_size = srs_utils_flv_audio_sound_size(data,size);//采样大小
            char audio_type = srs_utils_flv_audio_sound_type(data,size);//声道
            char audio_packet_type= srs_utils_flv_audio_aac_packet_type(data,size);
            if(audio_fromat_type == AUDIO_CODEC_AAC){
                if( audio_packet_type == 0){
                    _LOGD("audio pps:%d %d %d %d, sample_size = %d, audio_rate = %d", data[0], data[1], data[2], data[3], sampe_size,audio_rate);
                    uint8_t profile = data[2]>>3;
                    uint8_t sample_index = ((data[2] & 0x07) << 1) | ((data[3] & 0x80)>>7);
                    uint8_t channel_count = (data[3] & 0x78) >> 3;
                    _LOGD("profile:%d, channel_count:%d, sample_index:%d", profile, channel_count, sample_index);
                    oar->metadata->audio_codec = audio_fromat_type;
                    oar->metadata->sample_format = sampe_size;
                    int sample_rate = getSamplerate(sample_index);
                    if(sample_rate == -1){
                        oar->on_error(oar, OAR_ERROR_FORMAT_AUDIO_CONFIG);
                        free(data);
                        goto rtmp_destroy;
                    }
                    if(profile == 5 || profile == 29){
                        oar->metadata->sample_rate = sample_rate * 2;
                    }else{
                        oar->metadata->sample_rate = sample_rate;
                    }
                    oar->metadata->channels = channel_count > 2?2 : channel_count;//Android openSL ES   can not support more than 2 channels.
                    oar->metadata->audio_pps_size = 2;
                    if(oar->metadata->audio_pps){
                        free(oar->metadata->audio_pps);
                        oar->metadata->audio_pps = NULL;
                    }
                    oar->metadata->audio_pps = malloc(sizeof(char)*2);
                    memcpy(oar->metadata->audio_pps, data+2, 2);
                    oar->metadata->has_audio = 1;
                    if(oar->metadata->has_video){
                        oar->send_message(oar, oar_message_decoder_configuration);
                    }
                }else{
                    /*_LOGD("type: %s(%d); timestamp: %u; size: %d, data:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                     srs_human_flv_tag_type2string(type),type, timestamp,size,
                     data[0],data[1],data[2],data[3],
                     data[4],data[5],data[6],data[7],
                     data[8],data[9],data[10],data[11],
                     data[12],data[13],data[14],data[15]);*/
//                    _LOGD("audio packet size:%d", oar->audio_packet_queue->count);
                    oar_packet_queue_put(oar->audio_packet_queue, size-2,PktType_Audio, timestamp*USEC_PRE_MSEC, timestamp*USEC_PRE_MSEC, 0, data+2);
                }
            }else{
                oar->on_error(oar, OAR_ERROR_FORMAT_AUDIO_CODEC);
            }
            //_LOGD("audio format:%d, rate:%d, sample size:%d, type:%d, pakcet type:%d",
                 //audio_fromat_type, audio_rate, sampe_size,audio_type, audio_packet_type);
        }else if(type == SRS_RTMP_TYPE_VIDEO){
            char video_codec_id = srs_utils_flv_video_codec_id(data, size);
            char isKeyFrame = srs_utils_flv_video_frame_type(data,size);
            char video_packet_type = srs_utils_flv_video_avc_packet_type(data,size);
            if(video_codec_id == VIDEO_CODEC_AVC){
                if(video_packet_type == 0){
                    oar->metadata->video_codec = video_codec_id;
                    /*int sps_size = (*(data + 11)<<8) | *(data+12);
                    oar->metadata->video_sps = malloc(sizeof(char) * sps_size);
                    memcpy(oar->metadata->video_sps, data+13, sps_size);
                    oar->metadata->video_sps_size = sps_size;

                    int pps_size = (*(data+12+sps_size + 2)<<8) | *(data+12+sps_size + 3);
                    oar->metadata->video_pps = malloc(sizeof(char) * pps_size);
                    memcpy(oar->metadata->video_pps, data+12+sps_size + 4, pps_size);
                    oar->metadata->video_pps_size = pps_size;*/
                    if(oar->metadata->video_extradata != NULL){
                        free(oar->metadata->video_extradata);
                        oar->metadata->video_extradata = NULL;
                    }
                    oar->metadata->video_extradata = malloc(sizeof(char) * (size-5));
                    memcpy(oar->metadata->video_extradata, data+5, size-5);
                    oar->metadata->video_extradata_size = size -5;

                    oar->metadata->has_video = 1;
                    if(oar->metadata->has_audio){
                        oar->send_message(oar, oar_message_decoder_configuration);
                    }
                    //_LOGD("total size : %d, SPS size :%d, pps size : %d", size,0, 0);

                    //23 0 0 0 0
                    // 1(分隔符)
                    // 77 64 31(sps[1],sps[2],sps[3]
                    // 255 225
                    // 0 27 (sps size)
                    // 107 77 64 31 232 128 40 2 221 127 181 1 1 1 64 0 0 250 64 0 58 152 3 198 12 68 128 (sps)
                    // 1(分隔符)
                    // 0 4 104 235 239 32 (pps size)
                    /*_LOGE("video sps:%d, %d %d %d %d, %d, %d %d %d,%0x %0x, sps size = %d %d ,%d %d %d %d %d %d %d, pps size = %d %d %d",
                         data[0], data[1],data[2],data[3],data[4],
                         data[5],
                         data[6],data[7],data[8],
                         data[9],data[10],
                         data[11],data[12],
                         data[13],data[14],data[15],data[16],data[17],data[18],data[19],
                         data[13+data[12]],data[14+data[12]], data[15+data[12]]);*/


                }else{
                    /*_LOGI("type: %s(%d); timestamp: %u; size: %d, data:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                         srs_human_flv_tag_type2string(type),type, timestamp,size,
                         data[0],data[1],data[2],data[3],
                         data[4],data[5],data[6],data[7],
                         data[8],data[9],data[10],data[11],
                         data[12],data[13],data[14],data[15]);*/
                    oar_packet_queue_put(oar->video_packet_queue, size-5,PktType_Video, timestamp*USEC_PRE_MSEC, timestamp*USEC_PRE_MSEC, isKeyFrame, data+5);
                }
//            _LOGD("video codec_id:%d, isKeyFrame:%d, packet_type:%d", video_codec_id, isKeyFrame, video_packet_type);
            }else{
                oar->on_error(oar, OAR_ERROR_FORMAT_VIDEO_CODEC);
            }

        }else if(type == SRS_RTMP_TYPE_SCRIPT){
            if (srs_rtmp_is_onMetaData(type, data, size)) {
                int nparsed = 0;
                while (nparsed < size) {
                    int nb_parsed_this = 0;
                    srs_amf0_t amf0 = srs_amf0_parse(data + nparsed, size - nparsed, &nb_parsed_this);
                    if (amf0 == NULL) {
                        break;
                    }
                    if(srs_amf0_is_string(amf0)){
//                        _LOGD("string...");
                    }else if(srs_amf0_is_object(amf0)){
//                        _LOGD("object...");
                        /*char* amf0_str = NULL;
                        srs_human_amf0_print(amf0, &amf0_str, NULL);
                        _LOGD("nb_parsed_this:%d, str:%s", nb_parsed_this, amf0_str);*/
                        int count = srs_amf0_object_property_count(amf0);
//                        _LOGD("count : %d", count);
                       int i = 0;
                        for(i=0;i<count;i++){
                            const char * key = srs_amf0_object_property_name_at(amf0,i);
                            srs_amf0_t value  = srs_amf0_object_property_value_at(amf0, i);
                            if(strcmp(oar_metadata_video_framerate, key) == 0){
                                oar->metadata->fps =  srs_amf0_to_number(value);
                            }else if(strcmp(oar_metadata_video_rate, key)==0){
                                oar->metadata->video_bitrate =  srs_amf0_to_number(value);
                            }else if(strcmp(oar_metadata_video_width, key)==0){
                                oar->metadata->width =  srs_amf0_to_number(value);
                            }else if(strcmp(oar_metadata_video_height, key)==0){
                                oar->metadata->height =  srs_amf0_to_number(value);
                            }else if(strcmp(oar_metadata_audio_rate, key)==0){
                                oar->metadata->audio_bitrate =  srs_amf0_to_number(value);
                            }
//                            _LOGD("index is :%d,key is %s", i, key);

                        }
                    }
                    nparsed += nb_parsed_this;
                    srs_amf0_free(amf0);
                }

            }
        }

        //_LOGD("type: %s(%d); timestamp: %u; size: %d, data:%d %d %d %d", srs_human_flv_tag_type2string(type),type, timestamp,size,data[0],data[1],data[2],data[3]);

        free(data);
    }
    _LOGD("thread ==> %s exit", __func__);
    srs_rtmp_destroy(rtmp);
    return NULL;
rtmp_destroy:
    LOGE("goto rtmp destroy...");
    srs_rtmp_destroy(rtmp);
    return NULL;
}
