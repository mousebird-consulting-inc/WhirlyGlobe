/*
 *  LabelInfo_jni.cpp
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
#import "com_mousebird_maply_LabelInfo.h"
#import "WhirlyGlobe.h"
#import "LabelInfoAndroid.h"

using namespace WhirlyKit;

template<> LabelInfoClassInfo *LabelInfoClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_nativeInit
  (JNIEnv *env, jclass cls)
{
	LabelInfoClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoAndroid *info = new LabelInfoAndroid();
		// Note: Porting
		info->screenObject = true;
		LabelInfoClassInfo::getClassInfo()->setHandle(env,obj,info);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroid *info = (LabelInfoAndroid *)classInfo->getObject(env,obj);
		if (!info)
			return;
		info->clearRefs(env);
		delete info;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setEnable
  (JNIEnv *env, jobject obj, jboolean enable)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->enable = enable;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setDrawOffset
  (JNIEnv *env, jobject obj, jfloat drawOffset)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->drawOffset = drawOffset;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setDrawOffset()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setDrawPriority
  (JNIEnv *env, jobject obj, jint drawPriority)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->drawPriority = drawPriority;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setDrawPriority()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setMinVis
  (JNIEnv *env, jobject obj, jfloat minVis)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->minVis = minVis;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setMinViz()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setMaxVis
  (JNIEnv *env, jobject obj, jfloat maxVis)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->maxVis = maxVis;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setMaxVis()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setTextColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->textColor = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setBackgroundColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->backColor = RGBAColor(r*255,g*255,b*255,a*255);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setFade
  (JNIEnv *env, jobject obj, jfloat fade)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfo *info = classInfo->getObject(env,obj);
		if (!info)
			return;
		info->fade = fade;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setFade()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setTypeface
  (JNIEnv *env, jobject obj, jobject typefaceObj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroid *info = (LabelInfoAndroid *)classInfo->getObject(env,obj);
		if (!info)
			return;
		info->setTypeface(env,typefaceObj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setFade()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_LabelInfo_getTypeface
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroid *info = (LabelInfoAndroid *)classInfo->getObject(env,obj);
		if (!info)
			return NULL;

		return info->typefaceObj;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getTypeface()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setFontSize
  (JNIEnv *env, jobject obj, jfloat fontSize)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroid *info = (LabelInfoAndroid *)classInfo->getObject(env,obj);
		if (!info)
			return;

		info->fontSize = fontSize;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getTypeface()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LabelInfo_getTextColor
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroid *info = (LabelInfoAndroid *)classInfo->getObject(env,obj);
		if (!info)
			return 0;

		int textColor = info->textColor.asInt();
		return textColor;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getTextColor()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LabelInfo_getBackColor
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroid *info = (LabelInfoAndroid *)classInfo->getObject(env,obj);
		if (!info)
			return 0;

		int backColor = info->backColor.asInt();
		return backColor;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::getBackColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelInfo_setLayoutImportance
  (JNIEnv *env, jobject obj, jfloat importance)
{
	try
	{
		LabelInfoClassInfo *classInfo = LabelInfoClassInfo::getClassInfo();
		LabelInfoAndroid *info = (LabelInfoAndroid *)classInfo->getObject(env,obj);
		if (!info)
			return;

		info->layoutImportance = importance;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelInfo::setLayoutImportance()");
	}
}
