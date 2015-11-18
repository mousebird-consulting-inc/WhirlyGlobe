/*
 *  InternalMarker_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2015 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_InternalMarker.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

template<> MarkerClassInfo *MarkerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_nativeInit
  (JNIEnv *env, jclass cls)
{
	MarkerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Marker *marker = new Marker();
		MarkerClassInfo::getClassInfo()->setHandle(env,obj,marker);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;
		delete marker;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setSelectable
  (JNIEnv *env, jobject obj, jboolean sel)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->isSelectable = sel;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setSelectable()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setSelectID
  (JNIEnv *env, jobject obj, jlong newID)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->selectID = newID;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setSelectID()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!marker || !pt)
			return;

		marker->loc.x() = pt->x();
		marker->loc.y() = pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setLoc()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->color = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_addTexID
  (JNIEnv *env, jobject obj, jlong texID)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->texIDs.push_back(texID);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::addTexID()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setLockRotation
  (JNIEnv *env, jobject obj, jboolean lockRot)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->lockRotation = lockRot;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setLockRotation()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->height = height;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setHeight()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setWidth
  (JNIEnv *env, jobject obj, jdouble width)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->width = width;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setWidth()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setRotation
  (JNIEnv *env, jobject obj, jdouble rot)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->rotation = rot;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setRotation()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setOffset
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,obj);
		if (!marker || !pt)
			return;

		marker->offset.x() = pt->x();
		marker->offset.y() = pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setOffset()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setLayoutImportance
  (JNIEnv *env, jobject obj, jdouble imp)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->layoutImportance = imp;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setLayoutImportance()");
	}
}
