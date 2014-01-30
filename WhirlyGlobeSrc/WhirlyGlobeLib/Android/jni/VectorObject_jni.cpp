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

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorObject_addPoint
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		VectorObject *vecObj = getHandle<VectorObject>(env,obj);
		Point2d *pt = getHandle<Point2d>(env,ptObj);
		if (!vecObj)
			return;

		VectorPointsRef pts = VectorPoints::createPoints();
		pts->pts.push_back(GeoCoord(pt->x(),pt->y()));
		pts->initGeoMbr();
		vecObj->shapes.insert(pts);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addPoint()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorObject_addLinear
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
		VectorObject *vecObj = getHandle<VectorObject>(env,obj);
		if (!vecObj)
			return;

		VectorLinearRef lin = VectorLinear::createLinear();

		int count = env->GetArrayLength(ptsObj);
		if (count == 0)
			return;
		for (int ii=0;ii<count;ii++)
		{
			jobject ptObj = env->GetObjectArrayElement(ptsObj,ii);
			Point2d *pt = getHandle<Point2d>(env,ptObj);
			lin->pts.push_back(GeoCoord(pt->x(),pt->y()));
		}
		lin->initGeoMbr();
		vecObj->shapes.insert(lin);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addLinear()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorObject_addAreal
  (JNIEnv *env, jobject obj, jobjectArray ptsObj)
{
	try
	{
		VectorObject *vecObj = getHandle<VectorObject>(env,obj);
		if (!vecObj)
			return;

		VectorArealRef ar = VectorAreal::createAreal();
		ar->loops.resize(1);

		int count = env->GetArrayLength(ptsObj);
		if (count == 0)
			return;
		for (int ii=0;ii<count;ii++)
		{
			jobject ptObj = env->GetObjectArrayElement(ptsObj,ii);
			Point2d *pt = getHandle<Point2d>(env,ptObj);
			ar->loops[0].push_back(GeoCoord(pt->x(),pt->y()));
		}
		ar->initGeoMbr();
		vecObj->shapes.insert(ar);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorObject::addAreal()");
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

