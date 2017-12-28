//
// Created by gutou on 2017/4/20.
//

#ifndef __OAR_CLOCK_H
#define __OAR_CLOCK_H


#include <stdint.h>
#include "oarplayer_type_def.h"


uint64_t oar_clock_get_current_time();
oar_clock * oar_clock_create();
int64_t oar_clock_get(oar_clock * clock);
void oar_clock_set(oar_clock * clock, int64_t pts);
void oar_clock_free(oar_clock * clock);
void oar_clock_reset(oar_clock * clock);
#endif //__OAR_CLOCK_H
