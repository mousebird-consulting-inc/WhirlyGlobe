/*
 * VectorObject_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */

#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_VectorObject.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

void Java_com_mousebirdconsulting_maply_VectorObject_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObject *inst = new VectorObject();
		setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::initialise()");
	}
}

void Java_com_mousebirdconsulting_maply_VectorObject_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorObject *inst = getHandle<VectorObject>(env,obj);
		if (!inst)
			return;
		delete inst;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::dispose()");
	}
}

jboolean Java_com_mousebirdconsulting_maply_VectorObject_fromGeoJSON
  (JNIEnv *env, jobject obj, jstring jstr)
{
	try
	{
		VectorObject *vecObj = getHandle<VectorObject>(env,obj);
		if (!vecObj)
			return false;

		const char *cStr = env->GetStringUTFChars(jstr,0);
		if (!cStr)
			return false;
		std::string jsonStr(cStr);

		bool ret = vecObj->fromGeoJSON(jsonStr);

		env->ReleaseStringUTFChars(jstr, cStr);

		return ret;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::fromGeoJSON()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebirdconsulting_maply_VectorObject_fromGeoJSONAssembly
  (JNIEnv *env, jobject obj, jstring jstr)
{
	try
	{
		VectorObject *vecObj = getHandle<VectorObject>(env,obj);
		if (!vecObj)
			return false;

		const char *cStr = env->GetStringUTFChars(jstr,0);
		if (!cStr)
			return false;
		std::string jsonStr(cStr);

		bool ret = vecObj->fromGeoJSONAssembly(jsonStr);

		env->ReleaseStringUTFChars(jstr, cStr);

		return ret;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::fromGeoJSONAssembly()");
	}
}

