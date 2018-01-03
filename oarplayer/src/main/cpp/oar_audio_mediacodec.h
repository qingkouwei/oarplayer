
#ifndef __OAR_AUDIO_MEDIACODEC_H__
#define __OAR_AUDIO_MEDIACODEC_H__

#include <string.h>
#include <limits.h>
#include "oarplayer_type_def.h"
#include "endian.h"

oar_audio_mediacodec_context * oar_create_audio_mediacodec_context(
        oarplayer *oar);
void oar_audio_mediacodec_release_buffer(oarplayer *oar, int index);
int oar_audio_mediacodec_receive_frame(oarplayer *oar, OARFrame **frame);
int oar_audio_mediacodec_send_packet(oarplayer *oar, OARPacket *packet);
void oar_audio_mediacodec_flush(oarplayer *oar);
void oar_audio_mediacodec_release_context(oarplayer *oar);
void oar_audio_mediacodec_start(oarplayer *oar);
void oar_audio_mediacodec_stop(oarplayer *oar);

#endif //__OAR_AUDIO_MEDIACODEC_H__
