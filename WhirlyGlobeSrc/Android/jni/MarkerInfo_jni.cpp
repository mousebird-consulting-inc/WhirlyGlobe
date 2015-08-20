/*
 *  MarkerInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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
#import "com_mousebird_maply_MarkerInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_nativeInit
  (JNIEnv *env, jclass cls)
{
	MarkerInfoClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Dictionary dict;
		MarkerInfo *info = new MarkerInfo(dict);
		// Note: Porting
		info->screenObject = true;
		MarkerInfoClassInfo::getClassInfo()->setHandle(env,obj,info);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
		MarkerInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		delete info;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setEnable
  (JNIEnv *env, jobject obj, jboolean enable)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
		MarkerInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->enable = enable;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setDrawOffset
  (JNIEnv *env, jobject obj, jfloat drawOffset)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
		MarkerInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->drawOffset = drawOffset;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setDrawOffset()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setDrawPriority
  (JNIEnv *env, jobject obj, jint drawPriority)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
		MarkerInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->drawPriority = drawPriority;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setDrawPriority()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setMinVis
  (JNIEnv *env, jobject obj, jfloat minVis)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
		MarkerInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->minVis = minVis;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setMinViz()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setMaxVis
  (JNIEnv *env, jobject obj, jfloat maxVis)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
		MarkerInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->maxVis = maxVis;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setMaxVis()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
		MarkerInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->color = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setFade
  (JNIEnv *env, jobject obj, jfloat fade)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
		MarkerInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->fade = fade;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerInfo::setFade()");
	}
}
