

#ifndef OARPLAYER_OAR_PLAYER_H
#define OARPLAYER_OAR_PLAYER_H

#include <jni.h>
#include "oarplayer_type_def.h"

oarplayer *
oar_player_create(JNIEnv *env, jobject instance, int run_android_version, int best_samplerate);
void oar_player_set_buffer_time(oarplayer *oar, float buffer_time);
void oar_player_set_buffer_size(oarplayer *oar, int buffer_size);
int oar_player_play(oarplayer *oar);
int oar_player_stop(oarplayer *oar);
int oar_player_release(oarplayer *oar);
#endif //OARPLAYER_OAR_PLAYER_H
