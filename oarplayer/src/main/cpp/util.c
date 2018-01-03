

#include "util.h"
int get_int(uint8_t *buf) {
    return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
}

int64_t get_long(uint8_t *buf) {
    return (((int64_t) buf[0]) << 56)
           + (((int64_t) buf[1]) << 48)
           + (((int64_t) buf[2]) << 40)
           + (((int64_t) buf[3]) << 32)
           + (((int64_t) buf[4]) << 24)
           + (((int64_t) buf[5]) << 16)
           + (((int64_t) buf[6]) << 8)
           + ((int64_t) buf[7]);
}