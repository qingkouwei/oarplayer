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