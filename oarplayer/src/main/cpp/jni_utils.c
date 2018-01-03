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
#include <stdlib.h>
#include <string.h>
#include "jni_utils.h"
#define _JNILOG_TAG		"jni_utils"
#include "_android.h"

//char* to jstring
jstring str2JString(JNIEnv* env, const char* str)
{
	jclass strClass = (*env)->FindClass(env, "java/lang/String");
	jmethodID ctorID = (*env)->GetMethodID(env, strClass, "<init>",
			"([BLjava/lang/String;)V");
	jbyteArray bytes = (*env)->NewByteArray(env, strlen(str));
	(*env)->SetByteArrayRegion(env, bytes, 0, strlen(str), (jbyte*) str);
	jstring encoding = (*env)->NewStringUTF(env, "utf-8");
	jstring jstr = (jstring) (*env)->NewObject(env, strClass, ctorID, bytes, encoding);
	jstring ret = (*env)->NewGlobalRef(env, jstr);

	(*env)->DeleteLocalRef(env, jstr);
	(*env)->DeleteLocalRef(env, bytes);
	(*env)->DeleteLocalRef(env, encoding);
	(*env)->DeleteLocalRef(env, strClass);

	return ret;
}

int getIntFieldValue(JNIEnv* env, jobject obj, const char *fieldName, int def)
{
	jclass cls = NULL;
	jfieldID fid;

	cls =  (*env)->GetObjectClass(env, obj);
	fid =  (*env)->GetFieldID(env, cls , fieldName, "I");
	if (!fid) {
		(*env)->DeleteLocalRef(env, cls);
		return def;
	}
	int ret = (int)(*env)->GetIntField(env, obj, fid);
	(*env)->DeleteLocalRef(env, cls);
	return ret;
}

jlong getLongFieldValue(JNIEnv* env, jobject obj, const char *fieldName, jlong def)
{
	jclass cls = NULL;
	jfieldID fid;

	cls =  (*env)->GetObjectClass(env, obj);
	fid =  (*env)->GetFieldID(env, cls , fieldName, "J");
	if (!fid) {
		//LOGE("can not find long field, name: %s", fieldName);
		(*env)->DeleteLocalRef(env, cls);
		return def;
	}
	jlong ret = (*env)->GetLongField(env, obj, fid);
	(*env)->DeleteLocalRef(env, cls);
	return ret;
}

jboolean getBooleanFieldValue(JNIEnv* env, jobject obj, const char *fieldName, jlong def)
{
	jclass cls = NULL;
	jfieldID fid;

	cls =  (*env)->GetObjectClass(env, obj);
	fid =  (*env)->GetFieldID(env, cls , fieldName, "Z");
	if (!fid) {
		(*env)->DeleteLocalRef(env, cls);
		return def;
	}
	jboolean ret = (*env)->GetBooleanField(env, obj, fid);
	(*env)->DeleteLocalRef(env, cls);
	return ret;
}

int setLongFieldValue(JNIEnv* env, jobject obj,
		const char *fieldName, jlong value)
{
	jclass cls = NULL;
	jfieldID fid;

	cls =  (*env)->GetObjectClass(env, obj);
	fid =  (*env)->GetFieldID(env, cls , fieldName, "J");
	if (!fid) {
		LOGE("failed in GetFieldID, fid: %p", fid);
		(*env)->DeleteLocalRef(env, cls);
		return -1;
	}
	(*env)->SetLongField(env, obj, fid, value);
	(*env)->DeleteLocalRef(env, cls);
	return 0;
}

char *getStringFieldValue(JNIEnv* env, jobject obj, const char *fieldName)
{
	jclass cls = NULL;
	jfieldID fid;
	jboolean isCopy = 0;
	cls = (*env)->GetObjectClass(env, obj);
	fid = (*env)->GetFieldID(env, cls , fieldName, "Ljava/lang/String;");
	if (!fid) {
		LOGE("failed to get field, name=%s", fieldName);
		(*env)->DeleteLocalRef(env, cls);
		return NULL;
	}
	jstring jstr = (jstring)(*env)->GetObjectField(env, obj, fid);
	if (!jstr) {
		(*env)->DeleteLocalRef(env, cls);
		return NULL;
	}
	char *retStr = NULL;
	const char *data = (*env)->GetStringUTFChars(env, jstr, &isCopy);
	if (data) {
		retStr = (char *)malloc(strlen(data) + 1);
		if (retStr) {
			strcpy(retStr, data);
		}
	}

	if (data) {
		(*env)->ReleaseStringUTFChars(env, jstr, data);
	}
	(*env)->DeleteLocalRef(env, jstr);
	(*env)->DeleteLocalRef(env, cls);
	return retStr;
}

jclass getClassRef(JNIEnv* env, const char *clsName)
{
	jclass clz = (*env)->FindClass(env, clsName);
	if (!clz) {
		LOGE("failed to find the class: %s", clsName);
		return NULL;
	}
	clz = (*env)->NewGlobalRef(env, clz);
	if (!clz) {
		LOGE("failed to new ref of the class: %s", clsName);
		return NULL;
	}

	return clz;
}

int getStaticIntFieldValue(JNIEnv* env, const char *clsName, const char *fieldName, int def)
{
	jclass clz = getClassRef(env, clsName);
	if (!clz) {
		return def;
	}

	jfieldID fid;
	fid =  (*env)->GetStaticFieldID(env, clz , fieldName, "I");
	if (!fid) {
		LOGE("failed to get static field(%s) from class(%s).", fieldName, clsName);
		return def;
	}
	return (int)(*env)->GetStaticIntField(env, clz, fid);
}

char *getStaticStringFieldValue(JNIEnv* env, const char *clsName, const char *fieldName)
{
	jboolean isCopy = 0;
	jfieldID fid = 0;
	jclass cls = getClassRef(env, clsName);
	if (!cls) {
		return NULL;
	}

	fid = (*env)->GetStaticFieldID(env, cls , fieldName, "Ljava/lang/String;");
	if (!fid) {
		LOGE("failed to get field, name=%s", fieldName);
		return NULL;
	}
	jstring jstr = (jstring)(*env)->GetStaticObjectField(env, cls, fid);
	if (!jstr) {
		return NULL;
	}
	char *retStr = NULL;
	const char *data = (*env)->GetStringUTFChars(env, jstr, &isCopy);
	if (data) {
		retStr = (char *)malloc(strlen(data) + 1);
		if (retStr) {
			strcpy(retStr, data);
		}
	}

	if (data) {
		(*env)->ReleaseStringUTFChars(env, jstr, data);
	}
	(*env)->DeleteLocalRef(env, jstr);

	return retStr;
}

jobject getStaticObjectFieldValue(JNIEnv* env, const char *clsName,
								const char *fieldName,
								const char *fieldSigName)
{
	jclass cls = getClassRef(env, clsName);
	if (!cls) {
		return NULL;
	}

	jfieldID fid;
	fid = (*env)->GetStaticFieldID(env, cls , fieldName, fieldSigName);
	if (!fid) {
		LOGE("failed to get field, name=%s", fieldName);
		return NULL;
	}
	return (jobject)(*env)->GetStaticObjectField(env, cls, fid);

}

/********************
 * Exception Handle
 ********************/

int ExceptionCheck__throwAny(JNIEnv *env)
{
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        return 1;
    }

    return 0;
}

int ExceptionCheck__catchAll(JNIEnv *env)
{
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return 1;
    }

    return 0;
}

int ThrowExceptionOfClass(JNIEnv* env, jclass clazz, const char* msg)
{
    if ((*env)->ThrowNew(env, clazz, msg) != JNI_OK)
        LOGE("%s: Failed: msg: '%s'\n", __func__, msg);

    return 0;
}

int ThrowException(JNIEnv* env, const char* class_sign, const char* msg)
{
    int ret = -1;

    if (ExceptionCheck__catchAll(env)) {
        LOGE("pending exception throwed.\n");
    }

    jclass exceptionClass = FindClass__catchAll(env, class_sign);
    if (exceptionClass == NULL) {
        LOG_FUNC_FAIL_TRACE();
        ret = -1;
        goto fail;
    }

    ret = ThrowExceptionOfClass(env, exceptionClass, msg);
    if (ret) {
        LOG_FUNC_FAIL_TRACE();
        goto fail;
    }

    ret = 0;
    fail:
    DeleteLocalRef__p(env, exceptionClass);
    return ret;
}

int ThrowIllegalStateException(JNIEnv *env, const char* msg)
{
    return ThrowException(env, "java/lang/IllegalStateException", msg);
}

/********************
 * References
 ********************/

jclass NewGlobalRef__catchAll(JNIEnv *env, jobject obj)
{
    jclass obj_global = (*env)->NewGlobalRef(env, obj);
    if (ExceptionCheck__catchAll(env) || !(obj_global)) {
        LOG_FUNC_FAIL_TRACE();
        goto fail;
    }

    fail:
    return obj_global;
}

void DeleteLocalRef(JNIEnv *env, jobject obj)
{
    if (!obj)
        return;
    (*env)->DeleteLocalRef(env, obj);
}

void DeleteLocalRef__p(JNIEnv *env, jobject *obj)
{
    if (!obj)
        return;
    DeleteLocalRef(env, *obj);
    *obj = NULL;
}

void DeleteGlobalRef(JNIEnv *env, jobject obj)
{
    if (!obj)
        return;
    (*env)->DeleteGlobalRef(env, obj);
}

void DeleteGlobalRef__p(JNIEnv *env, jobject *obj)
{
    if (!obj)
        return;
    DeleteGlobalRef(env, *obj);
    *obj = NULL;
}

void ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *c_str)
{
    if (!str || !c_str)
        return;
    (*env)->ReleaseStringUTFChars(env, str, c_str);
}

void ReleaseStringUTFChars__p(JNIEnv *env, jstring str, const char **c_str)
{
    if (!str || !c_str)
        return;
    ReleaseStringUTFChars(env, str, *c_str);
    *c_str = NULL;
}

/********************
 * Class Load
 ********************/

jclass FindClass__catchAll(JNIEnv *env, const char *class_sign)
{
    jclass clazz = (*env)->FindClass(env, class_sign);
    if (ExceptionCheck__catchAll(env) || !(clazz)) {
        LOG_FUNC_FAIL_TRACE();
        clazz = NULL;
        goto fail;
    }

    fail:
    return clazz;
}

jclass FindClass__asGlobalRef__catchAll(JNIEnv *env, const char *class_sign)
{
    jclass clazz_global = NULL;
    jclass clazz = FindClass__catchAll(env, class_sign);
    if (!clazz) {
        LOG_FUNC_FAIL_TRACE1(class_sign);
        goto fail;
    }

    clazz_global = NewGlobalRef__catchAll(env, clazz);
    if (!clazz_global) {
        LOG_FUNC_FAIL_TRACE1(class_sign);
        goto fail;
    }

    fail:
    DeleteLocalRef__p(env, &clazz);
    return clazz_global;
}

jmethodID GetMethodID__catchAll(JNIEnv *env, jclass clazz, const char *method_name, const char *method_sign)
{
    jmethodID method_id = (*env)->GetMethodID(env, clazz, method_name, method_sign);
    if (ExceptionCheck__catchAll(env) || !method_id) {
        LOG_FUNC_FAIL_TRACE2(method_name, method_sign);
        method_id = NULL;
        goto fail;
    }

    fail:
    return method_id;
}

jmethodID GetStaticMethodID__catchAll(JNIEnv *env, jclass clazz, const char *method_name, const char *method_sign)
{
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, method_name, method_sign);
    if (ExceptionCheck__catchAll(env) || !method_id) {
        LOG_FUNC_FAIL_TRACE2(method_name, method_sign);
        method_id = NULL;
        goto fail;
    }

    fail:
    return method_id;
}

jfieldID GetFieldID__catchAll(JNIEnv *env, jclass clazz, const char *field_name, const char *field_sign)
{
    jfieldID field_id = (*env)->GetFieldID(env, clazz, field_name, field_sign);
    if (ExceptionCheck__catchAll(env) || !field_id) {
        LOG_FUNC_FAIL_TRACE2(field_name, field_sign);
        field_id = NULL;
        goto fail;
    }

    fail:
    return field_id;
}

jfieldID GetStaticFieldID__catchAll(JNIEnv *env, jclass clazz, const char *field_name, const char *field_sign)
{
    jfieldID field_id = (*env)->GetStaticFieldID(env, clazz, field_name, field_sign);
    if (ExceptionCheck__catchAll(env) || !field_id) {
        LOG_FUNC_FAIL_TRACE2(field_name, field_sign);
        field_id = NULL;
        goto fail;
    }

    fail:
    return field_id;
}

/********************
 * Misc Functions
 ********************/

jbyteArray NewByteArray__catchAll(JNIEnv *env, jsize capacity)
{
    jbyteArray local = (*env)->NewByteArray(env, capacity);
    if (ExceptionCheck__catchAll(env) || !local)
        return NULL;

    return local;
}

jbyteArray NewByteArray__asGlobalRef__catchAll(JNIEnv *env, jsize capacity)
{
    jbyteArray local = (*env)->NewByteArray(env, capacity);
    if (ExceptionCheck__catchAll(env) || !local)
        return NULL;

    jbyteArray global = (*env)->NewGlobalRef(env, local);
    DeleteLocalRef__p(env, &local);
    return global;
}

int JNI_ThrowException(JNIEnv* env, const char* className, const char* msg)
{
	if ((*env)->ExceptionCheck(env)) {
		jthrowable exception = (*env)->ExceptionOccurred(env);
		(*env)->ExceptionClear(env);

		if (exception != NULL) {
			LOGW("Discarding pending exception (%s) to throw", className);
			(*env)->DeleteLocalRef(env, exception);
		}
	}

	jclass exceptionClass = (*env)->FindClass(env, className);
	if (exceptionClass == NULL) {
		LOGE("Unable to find exception class %s", className);
		/* ClassNotFoundException now pending */
		goto fail;
	}

	if ((*env)->ThrowNew(env, exceptionClass, msg) != JNI_OK) {
		LOGE("Failed throwing '%s' '%s'", className, msg);
		/* an exception, most likely OOM, will now be pending */
		goto fail;
	}

	return 0;
	fail:
	if (exceptionClass)
		(*env)->DeleteLocalRef(env, exceptionClass);
	return -1;
}

int JNI_ThrowIllegalStateException(JNIEnv *env, const char* msg)
{
	return JNI_ThrowException(env, "java/lang/IllegalStateException", msg);
}
bool JNI_ExceptionCheck__catchAll(JNIEnv *env)
{
	if ((*env)->ExceptionCheck(env)) {
		(*env)->ExceptionDescribe(env);
		(*env)->ExceptionClear(env);
		return true;
	}

	return false;
}

int JNI_ThrowExceptionOfClass(JNIEnv* env, jclass clazz, const char* msg)
{
	if ((*env)->ThrowNew(env, clazz, msg) != JNI_OK)
		LOGE("%s: Failed: msg: '%s'\n", __func__, msg);

	return 0;
}