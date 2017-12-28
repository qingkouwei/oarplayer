/*
 * android_log.h
 *
 *  Created on: 2014-6-30
 *      Author: ken
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
#define  _JNILOG(LEVEL, FMT, ...)  if (LEVEL >= __LOG_LEVEL) \
	__android_log_print(LEVEL,_JNILOG_TAG, "[%s():%d]" FMT, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define  LOGD(...)  _JNILOG(ANDROID_LOG_DEBUG,__VA_ARGS__)
#define  LOGI(...)  _JNILOG(ANDROID_LOG_INFO,__VA_ARGS__)
#define  LOGW(...)  _JNILOG(ANDROID_LOG_WARN,__VA_ARGS__)
#define  LOGE(...)  _JNILOG(ANDROID_LOG_ERROR,__VA_ARGS__)

#define LOG_FUNC_FAIL_TRACE()               do {LOGE("%s: failed\n", __func__);} while (0)
#define LOG_FUNC_FAIL_TRACE1(x__)           do {LOGE("%s: failed: %s\n", __func__, x__);} while (0)
#define LOG_FUNC_FAIL_TRACE2(x1__, x2__)    do {LOGE("%s: failed: %s %s\n", __func__, x1__, x2__);} while (0)

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

#endif
