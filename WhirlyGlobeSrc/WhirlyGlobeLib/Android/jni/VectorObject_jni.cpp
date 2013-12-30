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
	VectorObject *inst = new VectorObject();
	setHandle(env,obj,inst);
}

void Java_com_mousebirdconsulting_maply_VectorObject_dispose
  (JNIEnv *env, jobject obj)
{
	VectorObject *inst = getHandle<VectorObject>(env,obj);
	if (!inst)
		return;
	delete inst;
	inst = NULL;

	setHandle(env,obj,inst);
}

jboolean Java_com_mousebirdconsulting_maply_VectorObject_fromGeoJSON
  (JNIEnv *env, jobject obj, jstring jstr)
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
