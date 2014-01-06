/*
 * handle.h
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */
#include <android/log.h>

// From: http://thebreakfastpost.com/2012/01/26/wrapping-a-c-library-with-jni-part-2/

#ifndef HANDLE_H_
#define HANDLE_H_

// Note: Porting.  This is bogus.  Move it into a cpp file
static jfieldID getHandleField(JNIEnv *env, jobject obj)
{
    jclass c = env->GetObjectClass(obj);
    // J is the type signature for long:
    return env->GetFieldID(c, "nativeHandle", "J");
}

template <typename T>
T *getHandle(JNIEnv *env, jobject obj)
{
	if (!obj)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Got null object handle.");
	}
    jlong handle = env->GetLongField(obj, getHandleField(env, obj));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t)
{
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj), handle);
}

#endif /* HANDLE_H_ */
