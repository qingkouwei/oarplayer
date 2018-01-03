

#ifndef __OAR_VIDEO_MEDIACODEC_H__
#define __OAR_VIDEO_MEDIACODEC_H__

#include <string.h>
#include <limits.h>
#include "oarplayer_type_def.h"
#include "endian.h"

/* Inspired by libavcodec/hevc.c */
static int convert_hevc_nal_units(const uint8_t *p_buf,size_t i_buf_size,
                           uint8_t *p_out_buf,size_t i_out_buf_size,
                           size_t *p_sps_pps_size,size_t *p_nal_size)
{
    int i, num_arrays;
    const uint8_t *p_end = p_buf + i_buf_size;
    uint32_t i_sps_pps_size = 0;

    if( i_buf_size <= 3 || ( !p_buf[0] && !p_buf[1] && p_buf[2] <= 1 ) )
        return -1;

    if( p_end - p_buf < 23 )
    {
        //LOGE( "Input Metadata too small" );
        return -1;
    }

    p_buf += 21;

    if( p_nal_size )
        *p_nal_size = (size_t) ((*p_buf & 0x03) + 1);
    p_buf++;

    num_arrays = *p_buf++;

    for( i = 0; i < num_arrays; i++ )
    {
        int type, cnt, j;

        if( p_end - p_buf < 3 )
        {
            //LOGE( "Input Metadata too small" );
            return -1;
        }
        type = *(p_buf++) & 0x3f;
        (void)(type);

        cnt = p_buf[0] << 8 | p_buf[1];
        p_buf += 2;

        for( j = 0; j < cnt; j++ )
        {
            int i_nal_size;

            if( p_end - p_buf < 2 )
            {
                //LOGE( "Input Metadata too small" );
                return -1;
            }

            i_nal_size = p_buf[0] << 8 | p_buf[1];
            p_buf += 2;

            if( i_nal_size < 0 || p_end - p_buf < i_nal_size )
            {
                //LOGE( "NAL unit size does not match Input Metadata size" );
                return -1;
            }

            if( i_sps_pps_size + 4 + i_nal_size > i_out_buf_size )
            {
                //LOGE( "Output buffer too small" );
                return -1;
            }

            p_out_buf[i_sps_pps_size++] = 0;
            p_out_buf[i_sps_pps_size++] = 0;
            p_out_buf[i_sps_pps_size++] = 0;
            p_out_buf[i_sps_pps_size++] = 1;

            memcpy(p_out_buf + i_sps_pps_size, p_buf, (size_t) i_nal_size);
            p_buf += i_nal_size;

            i_sps_pps_size += i_nal_size;
        }
    }

    *p_sps_pps_size = i_sps_pps_size;

    return 0;
}
static int convert_sps_pps2(const uint8_t *p_buf, size_t i_buf_size,
                            uint8_t * out_sps_buf, size_t * out_sps_buf_size,
                            uint8_t * out_pps_buf, size_t * out_pps_buf_size,
//                            uint8_t *p_out_buf, size_t i_out_buf_size,
//                            size_t *p_sps_pps_size,
                            size_t *p_nal_size
) {
    // int i_profile;
    uint32_t i_data_size = (uint32_t) i_buf_size, i_nal_size ;
    unsigned int i_loop_end;

    /* */
    if (i_data_size < 7) {
        //LOGE("Input Metadata too small");
        return -1;
    }

    /* Read infos in first 6 bytes */
    // i_profile    = (p_buf[1] << 16) | (p_buf[2] << 8) | p_buf[3];
    if (p_nal_size)
        *p_nal_size = (size_t) ((p_buf[4] & 0x03) + 1);
    p_buf += 5;
    i_data_size -= 5;

    for (unsigned int j = 0; j < 2; j++) {
        /* First time is SPS, Second is PPS */
        if (i_data_size < 1) {
            /*LOGE("PPS too small after processing SPS/PPS %u",
                 i_data_size);*/
            return -1;
        }
        i_loop_end = (unsigned int) (p_buf[0] & (j == 0 ? 0x1f : 0xff));
        p_buf++;
        i_data_size--;

        for (unsigned int i = 0; i < i_loop_end; i++) {
            if (i_data_size < 2) {
                //LOGE("SPS is too small %u", i_data_size);
                return -1;
            }

            i_nal_size = (p_buf[0] << 8) | p_buf[1];
            p_buf += 2;
            i_data_size -= 2;

            if (i_data_size < i_nal_size) {
                //LOGE("SPS size does not match NAL specified size %u",
                 //    i_data_size);
                return -1;
            }
//            if (i_sps_pps_size + 4 + i_nal_size > i_out_buf_size) {
//                LOGE("Output SPS/PPS buffer too small");
//                return -1;
//            }
//
//            p_out_buf[i_sps_pps_size++] = 0;
//            p_out_buf[i_sps_pps_size++] = 0;
//            p_out_buf[i_sps_pps_size++] = 0;
//            p_out_buf[i_sps_pps_size++] = 1;
//
//            memcpy(p_out_buf + i_sps_pps_size, p_buf, i_nal_size);
//            i_sps_pps_size += i_nal_size;
            if(j == 0){
                out_sps_buf[0] = 0;
                out_sps_buf[1] = 0;
                out_sps_buf[2] = 0;
                out_sps_buf[3] = 1;
                memcpy(out_sps_buf + 4, p_buf, i_nal_size);
                * out_sps_buf_size = i_nal_size + 4;
            }else{
                out_pps_buf[0] = 0;
                out_pps_buf[1] = 0;
                out_pps_buf[2] = 0;
                out_pps_buf[3] = 1;
                memcpy(out_pps_buf + 4, p_buf, i_nal_size);
                * out_pps_buf_size = i_nal_size + 4;
            }

            p_buf += i_nal_size;
            i_data_size -= i_nal_size;
        }
    }

//    *p_sps_pps_size = i_sps_pps_size;

    return 0;
}
typedef struct H264ConvertState {
    uint32_t nal_len;
    uint32_t nal_pos;
} H264ConvertState;

static void convert_h264_to_annexb( uint8_t *p_buf, size_t i_len,
                                    size_t i_nal_size,
                                    H264ConvertState *state )
{
    if( i_nal_size < 3 || i_nal_size > 4 )
        return;

    /* This only works for NAL sizes 3-4 */
    while( i_len > 0 )
    {
        if( state->nal_pos < i_nal_size ) {
            unsigned int i;
            for( i = 0; state->nal_pos < i_nal_size && i < i_len; i++, state->nal_pos++ ) {
                state->nal_len = (state->nal_len << 8) | p_buf[i];
                p_buf[i] = 0;
            }
            if( state->nal_pos < i_nal_size )
                return;
            p_buf[i - 1] = 1;
            p_buf += i;
            i_len -= i;
        }
        if( state->nal_len > INT_MAX )
            return;
        if( state->nal_len > i_len )
        {
            state->nal_len -= i_len;
            return;
        }
        else
        {
            p_buf += state->nal_len;
            i_len -= state->nal_len;
            state->nal_len = 0;
            state->nal_pos = 0;
        }
    }
}

oar_video_mediacodec_context * oar_create_video_mediacodec_context(
        oarplayer *oar);
void oar_video_mediacodec_release_buffer(oarplayer *oar, OARFrame *frame);
int oar_video_mediacodec_receive_frame(oarplayer *oar, OARFrame *frame);
int oar_video_mediacodec_send_packet(oarplayer *oar, OARPacket *packet);
void oar_video_mediacodec_flush(oarplayer *oar);
void oar_video_mediacodec_release_context(oarplayer *oar);
void oar_video_mediacodec_start(oarplayer *oar);
void oar_video_mediacodec_stop(oarplayer *oar);

#endif //__OAR_VIDEO_MEDIACODEC_H__
