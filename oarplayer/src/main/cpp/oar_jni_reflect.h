
#ifndef __OAR_JNI_REFLECT_H
#define __OAR_JNI_REFLECT_H
#include "oarplayer_type_def.h"

void oar_jni_reflect_java_class(oar_java_class ** p_jc, JNIEnv *jniEnv);
void oar_jni_free(oar_java_class **p_jc, JNIEnv *jniEnv);
#endif //__OAR_JNI_REFLECT_H
