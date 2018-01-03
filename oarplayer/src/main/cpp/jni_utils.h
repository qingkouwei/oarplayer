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
#ifndef __JNI_UTILS_H__
#define __JNI_UTILS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include <stdbool.h>

int getIntFieldValue(JNIEnv* env, jobject obj, const char *fieldName, int def);
jlong getLongFieldValue(JNIEnv* env, jobject obj, const char *fieldName, jlong def);

jboolean getBooleanFieldValue(JNIEnv* env, jobject obj, const char *fieldName, jlong def);

int setLongFieldValue(JNIEnv* env, jobject obj,
		const char *fieldName, jlong value);

char *getStringFieldValue(JNIEnv* env, jobject obj, const char *fieldName);

jstring str2JString(JNIEnv* env, const char* str);

jclass getClassRef(JNIEnv* env, const char *clsName);

int getStaticIntFieldValue(JNIEnv* env, const char *clsName, const char *fieldName, int def);

char *getStaticStringFieldValue(JNIEnv* env, const char *clsName, const char *fieldName);

jobject getStaticObjectFieldValue(JNIEnv* env, const char *clsName,
								  const char *fieldName,
								  const char *fieldSigName);

JNIEnv* getJNIEnv();
void detachCurrentThread();

int ExceptionCheck__throwAny(JNIEnv *env);
int ExceptionCheck__catchAll(JNIEnv *env);
int ThrowExceptionOfClass(JNIEnv* env, jclass clazz, const char* msg);
int ThrowException(JNIEnv* env, const char* class_sign, const char* msg);
int ThrowIllegalStateException(JNIEnv *env, const char* msg);
jclass NewGlobalRef__catchAll(JNIEnv *env, jobject obj);
void DeleteLocalRef(JNIEnv *env, jobject obj);
void DeleteLocalRef__p(JNIEnv *env, jobject *obj);
void DeleteGlobalRef(JNIEnv *env, jobject obj);

void DeleteGlobalRef__p(JNIEnv *env, jobject *obj);
void ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *c_str);
void ReleaseStringUTFChars__p(JNIEnv *env, jstring str, const char **c_str);
jclass FindClass__catchAll(JNIEnv *env, const char *class_sign);
jclass FindClass__asGlobalRef__catchAll(JNIEnv *env, const char *class_sign);
jmethodID GetMethodID__catchAll(JNIEnv *env, jclass clazz, const char *method_name, const char *method_sign);
jmethodID GetStaticMethodID__catchAll(JNIEnv *env, jclass clazz, const char *method_name, const char *method_sign);
jfieldID GetFieldID__catchAll(JNIEnv *env, jclass clazz, const char *field_name, const char *field_sign);

jfieldID GetStaticFieldID__catchAll(JNIEnv *env, jclass clazz, const char *field_name, const char *field_sign);

jbyteArray NewByteArray__catchAll(JNIEnv *env, jsize capacity);

jbyteArray NewByteArray__asGlobalRef__catchAll(JNIEnv *env, jsize capacity);

int      JNI_ThrowException(JNIEnv *env, const char *exception, const char* msg);
int      JNI_ThrowIllegalStateException(JNIEnv *env, const char* msg);
bool JNI_ExceptionCheck__catchAll(JNIEnv *env);
int JNI_ThrowExceptionOfClass(JNIEnv* env, jclass clazz, const char* msg);

#define OAR_FIND_JAVA_CLASS(env__, var__, classsign__) \
    do { \
        jclass clazz = (*env__)->FindClass(env__, classsign__); \
        if (JNI_ExceptionCheck__catchAll(env) || !(clazz)) { \
            LOGE("FindClass failed: %s", classsign__); \
            return -1; \
        } \
        var__ = (*env__)->NewGlobalRef(env__, clazz); \
        if (JNI_ExceptionCheck__catchAll(env) || !(var__)) { \
            LOGE("FindClass::NewGlobalRef failed: %s", classsign__); \
            (*env__)->DeleteLocalRef(env__, clazz); \
            return -1; \
        } \
        (*env__)->DeleteLocalRef(env__, clazz); \
    } while(0);

#define JNI_CHECK_GOTO(condition__, env__, exception__, msg__, label__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                JNI_ThrowException(env__, exception__, msg__); \
            } \
            goto label__; \
        } \
    }while(0)

#define JNI_CHECK_RET_VOID(condition__, env__, exception__, msg__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                JNI_ThrowException(env__, exception__, msg__); \
            } \
            return; \
        } \
    }while(0)

#define JNI_CHECK_RET(condition__, env__, exception__, msg__, ret__) \
    do { \
        if (!(condition__)) { \
            if (exception__) { \
                JNI_ThrowException(env__, exception__, msg__); \
            } \
            return ret__; \
        } \
    }while(0)

#ifdef __cplusplus
}
#endif

#endif
