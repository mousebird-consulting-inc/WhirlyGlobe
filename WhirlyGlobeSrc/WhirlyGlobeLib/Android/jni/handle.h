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
static jfieldID getHandleField(JNIEnv *env, jobject obj,const char *name)
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
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Null object handle in getHandle().");
	}
    jlong handle = env->GetLongField(obj, getHandleField(env, obj, "nativeHandle"));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
T *getHandleNamed(JNIEnv *env, jobject obj,const char *handleName)
{
	if (!obj)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Null object handle in getHandleNamed().");
	}
    jlong handle = env->GetLongField(obj, getHandleField(env, obj, handleName));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t)
{
	if (!t)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Null handle in setHandle()");
	}
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj, "nativeHandle"), handle);
}

static void clearHandle(JNIEnv *env, jobject obj)
{
    env->SetLongField(obj, getHandleField(env, obj, "nativeHandle"), 0);
}

template <typename T>
void setHandleNamed(JNIEnv *env, jobject obj, T *t,const char *handleName)
{
	if (!t)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Null handle in setHandle()");
	}
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj, handleName), handle);
}

static void clearHandleNamed(JNIEnv *env, jobject obj,const char *handleName)
{
    env->SetLongField(obj, getHandleField(env, obj, handleName), 0);
}

#endif /* HANDLE_H_ */
