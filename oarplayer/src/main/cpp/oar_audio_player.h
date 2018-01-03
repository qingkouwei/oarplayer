
#ifndef OARPLAYER_OAR_AUDIO_PLAYER_H
#define OARPLAYER_OAR_AUDIO_PLAYER_H

#include "oarplayer_type_def.h"

oar_audio_player_context* oar_audio_engine_create();
void oar_audio_player_create(int rate, int channel, oarplayer* oar);

#endif //OARPLAYER_OAR_AUDIO_PLAYER_H
