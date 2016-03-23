/*
 *  VectorInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2016 mousebird consulting
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
#import "com_mousebird_maply_VectorInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorInfoClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Dictionary dict;
		VectorInfo *vecInfo = new VectorInfo(dict);
		VectorInfoClassInfo::getClassInfo()->setHandle(env,obj,vecInfo);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfo *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		delete vecInfo;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setFilled
  (JNIEnv *env, jobject obj, jboolean bVal)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfo *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		vecInfo->filled = bVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setFilled()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setTexId
  (JNIEnv *env, jobject obj, jlong val)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfo *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		vecInfo->texId = val;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setTexId()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setTexScale
  (JNIEnv *env, jobject obj, jfloat s, jfloat t)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfo *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		vecInfo->texScale.x() = s;
		vecInfo->texScale.y() = t;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setEnable()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_subdivEps
  (JNIEnv *env, jobject obj, jfloat eps)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfo *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		vecInfo->subdivEps = eps;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::subdivEps()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setGridSubdiv
  (JNIEnv *env, jobject obj, jboolean bVal)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfo *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		vecInfo->gridSubdiv = bVal;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setGridSubdiv()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfo *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		vecInfo->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorInfo_setLineWidth
  (JNIEnv *env, jobject obj, jfloat val)
{
	try
	{
		VectorInfoClassInfo *classInfo = VectorInfoClassInfo::getClassInfo();
		VectorInfo *vecInfo = classInfo->getObject(env,obj);
		if (!vecInfo)
			return;
		vecInfo->lineWidth = val;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::setLineWidth()");
	}
}
