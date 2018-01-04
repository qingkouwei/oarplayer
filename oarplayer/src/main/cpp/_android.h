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
#ifndef _ANDROID_H_
#define _ANDROID_H_

#include <jni.h>
#include <android/log.h>
#ifndef _JNILOG_TAG
#define  _JNILOG_TAG    __FILE__
#endif
#ifndef __LOG_LEVEL
#define __LOG_LEVEL ANDROID_LOG_DEBUG
#endif

#define isDebug 0

#define  _JNILOG(LEVEL, FMT, ...)  if (LEVEL >= __LOG_LEVEL) \
	__android_log_print(LEVEL,_JNILOG_TAG, "[%s():%d]" FMT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define  LOGD(...)  _JNILOG(ANDROID_LOG_DEBUG,__VA_ARGS__)
#define  LOGI(...)  _JNILOG(ANDROID_LOG_INFO,__VA_ARGS__)
#define  LOGW(...)  _JNILOG(ANDROID_LOG_WARN,__VA_ARGS__)
#define  LOGE(...)  _JNILOG(ANDROID_LOG_ERROR,__VA_ARGS__)

#define LOG_FUNC_FAIL_TRACE()               do {LOGE("%s: failed\n", __func__);} while (0)
#define LOG_FUNC_FAIL_TRACE1(x__)           do {LOGE("%s: failed: %s\n", __func__, x__);} while (0)
#define LOG_FUNC_FAIL_TRACE2(x1__, x2__)    do {LOGE("%s: failed: %s %s\n", __func__, x1__, x2__);} while (0)

#endif
