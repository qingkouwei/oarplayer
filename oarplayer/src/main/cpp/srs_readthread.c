//
// Created by 申俊伟 on 2017/12/15.
//

#include <malloc.h>
#include "srs_readthread.h"
#define _JNILOG_TAG "srs_player"
#include "_android.h"
#include "srs_librtmp.h"
#include "oarplayer_type_def.h"
#include "oar_packet_queue.h"

void *read_thread(void *data) {
    LOGI("read thread start...");
    LOGI("version: %d.%d.%d\n", srs_version_major(), srs_version_minor(), srs_version_revision());
    oarplayer *oar = data;
    srs_rtmp_t rtmp = srs_rtmp_create(oar->url);
    if(rtmp == NULL){
        LOGE("create srs rtmp failed.");
    }
    LOGI("srs_rtmp_create success, url is : %s", oar->url);
    int ret = srs_rtmp_handshake(rtmp);
    if(ret != 0){
        LOGE("simple handshake failed:%d",ret);
        goto rtmp_destroy;
    }
    LOGI("simple handshake success.");
    if(srs_rtmp_connect_app(rtmp) != 0){
        LOGE("connect vhost/app failed.");
        goto rtmp_destroy;
    }
    LOGI("connect vhost/app success.");
    if(srs_rtmp_play_stream(rtmp) != 0){
        LOGE("play stream failed.");
        goto rtmp_destroy;
    }
    LOGI("play stream success.");
    LOGI("error_code:%d", oar->error_code);
    while(oar->error_code == 0){
        int size;
        char type;
        char* data;
        u_int32_t timestamp;//, pts;
        //LOGE("start read...");
        int ret = srs_rtmp_read_packet(rtmp, &type, &timestamp, &data, &size);
        if ( ret != 0) {
            LOGE("read packet ret : %d", ret);
            //goto rtmp_destroy;
        }
        /*if (srs_utils_parse_timestamp(timestamp, type, data, size, &pts) != 0) {
            LOGE("parse timestampe failed...");
            goto rtmp_destroy;
        }*/
        if(type == SRS_RTMP_TYPE_AUDIO){
            char audio_fromat_type = srs_utils_flv_audio_sound_format(data,size);//编码类型
            char audio_rate = srs_utils_flv_audio_sound_rate(data,size);//采样率
            char sampe_size = srs_utils_flv_audio_sound_size(data,size);//采样大小
            char audio_type = srs_utils_flv_audio_sound_type(data,size);//声道
            char audio_packet_type= srs_utils_flv_audio_aac_packet_type(data,size);
            if(audio_fromat_type == 10 && audio_packet_type == 0){
                LOGI("audio pps:%d %d %d %d", data[0], data[1], data[2], data[3]);
                oar->metadata->audio_codec = audio_fromat_type;
                oar->metadata->sample_format = sampe_size;
                oar->metadata->sample_rate = audio_rate;
                oar->metadata->channels = audio_type;
                //oar->metadata->audio_bitrate =;
                oar->metadata->audio_pps_size = size;
                oar->metadata->audio_pps = malloc(sizeof(char)*size);
                memcpy(oar->metadata->audio_pps, data, size);
                oar->metadata->has_audio = 1;
                if(oar->metadata->has_video){
                    oar->send_message(oar, oar_message_decoder_configuration);
                }
            }else{

            }
            //LOGI("audio format:%d, rate:%d, sample size:%d, type:%d, pakcet type:%d",
                 //audio_fromat_type, audio_rate, sampe_size,audio_type, audio_packet_type);
        }else if(type == SRS_RTMP_TYPE_VIDEO){
            char video_codec_id = srs_utils_flv_video_codec_id(data, size);
            char isKeyFrame = srs_utils_flv_video_frame_type(data,size);
            char video_packet_type = srs_utils_flv_video_avc_packet_type(data,size);

            if(video_codec_id == 7 && video_packet_type == 0){
                oar->metadata->video_codec = video_codec_id;
                /*int sps_size = (*(data + 11)<<8) | *(data+12);
                oar->metadata->video_sps = malloc(sizeof(char) * sps_size);
                memcpy(oar->metadata->video_sps, data+13, sps_size);
                oar->metadata->video_sps_size = sps_size;

                int pps_size = (*(data+12+sps_size + 2)<<8) | *(data+12+sps_size + 3);
                oar->metadata->video_pps = malloc(sizeof(char) * pps_size);
                memcpy(oar->metadata->video_pps, data+12+sps_size + 4, pps_size);
                oar->metadata->video_pps_size = pps_size;*/
                oar->metadata->video_extradata = malloc(sizeof(char) * (size-5));
                memcpy(oar->metadata->video_extradata, data+5, size-5);
                oar->metadata->video_extradata_size = size -5;

                oar->metadata->has_video = 1;
                if(oar->metadata->has_audio){
                    oar->send_message(oar, oar_message_decoder_configuration);
                }
                LOGI("total size : %d, SPS size :%d, pps size : %d", size,0, 0);

                //23 0 0 0 0
                // 1(分隔符)
                // 77 64 31(sps[1],sps[2],sps[3]
                // 255 225
                // 0 27 (sps size)
                // 107 77 64 31 232 128 40 2 221 127 181 1 1 1 64 0 0 250 64 0 58 152 3 198 12 68 128 (sps)
                // 1(分隔符)
                // 0 4 104 235 239 32 (pps size)
                /*LOGE("video sps:%d, %d %d %d %d, %d, %d %d %d,%0x %0x, sps size = %d %d ,%d %d %d %d %d %d %d, pps size = %d %d %d",
                     data[0], data[1],data[2],data[3],data[4],
                     data[5],
                     data[6],data[7],data[8],
                     data[9],data[10],
                     data[11],data[12],
                     data[13],data[14],data[15],data[16],data[17],data[18],data[19],
                     data[13+data[12]],data[14+data[12]], data[15+data[12]]);*/


            }else{
                /*LOGI("type: %s(%d); timestamp: %u; size: %d, data:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                     srs_human_flv_tag_type2string(type),type, timestamp,size,
                     data[0],data[1],data[2],data[3],
                     data[4],data[5],data[6],data[7],
                     data[8],data[9],data[10],data[11],
                     data[12],data[13],data[14],data[15]);*/
                char *video = (char*)malloc(sizeof(char) *(size - 5));
                memcpy(video, data+5, size - 5);
                oar_packet_queue_put(oar->video_packet_queue, size-5,PktType_Video, timestamp, timestamp, isKeyFrame, video);
                free(video);
            }
//            LOGI("video codec_id:%d, isKeyFrame:%d, packet_type:%d", video_codec_id, isKeyFrame, video_packet_type);

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
//                        LOGI("string...");
                    }else if(srs_amf0_is_object(amf0)){
//                        LOGI("object...");
                        /*char* amf0_str = NULL;
                        srs_human_amf0_print(amf0, &amf0_str, NULL);
                        LOGI("nb_parsed_this:%d, str:%s", nb_parsed_this, amf0_str);*/
                        int count = srs_amf0_object_property_count(amf0);
//                        LOGI("count : %d", count);
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
                            //LOGI("index is :%d,key is %s", i, key);
                            if(key){
                                free((void*)key);
                                key = NULL;
                            }

                        }
                    }
                    nparsed += nb_parsed_this;
                    srs_amf0_free(amf0);
                }

            }
        }

//        LOGI("type: %s(%d); timestamp: %u; size: %d, data:%d %d %d %d", srs_human_flv_tag_type2string(type),type, timestamp,size,data[0],data[1],data[2],data[3]);

        srs_rtmp_free_packet(data);
    }
    return NULL;
rtmp_destroy:
    LOGE("goto rtmp destroy...");
    srs_rtmp_destroy(rtmp);
    return NULL;
}
