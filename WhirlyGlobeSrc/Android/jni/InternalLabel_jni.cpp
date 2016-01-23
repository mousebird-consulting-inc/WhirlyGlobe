/*
 *  InternalLabel_jni.cpp
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
#import "com_mousebird_maply_InternalLabel.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

typedef JavaClassInfo<WhirlyKit::SingleLabelAndroid> LabelClassInfo;
template<> LabelClassInfo *LabelClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_nativeInit
  (JNIEnv *env, jclass cls)
{
	LabelClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		SingleLabelAndroid *label = new SingleLabelAndroid();
		LabelClassInfo::getClassInfo()->setHandle(env,obj,label);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		if (!label)
			return;
		delete label;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!label || !pt)
			return;

		label->loc.x() = pt->x();
		label->loc.y() = pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setLoc()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setRotation
  (JNIEnv *env, jobject obj, jdouble rot)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		if (!label)
			return;

		label->rotation = rot;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setRotation()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setText
(JNIEnv *env, jobject obj, jintArray textArray, jint len)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabelAndroid *label = classInfo->getObject(env,obj);
		if (!label)
			return;

        JavaIntArray intArray(env,textArray);
        label->codePoints.resize(len);
        for (int ii=0;ii<intArray.len;ii++)
            label->codePoints[ii] = intArray.rawInt[ii];
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setText()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setOffset
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabelAndroid *label = classInfo->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,obj);
		if (!label || !pt)
			return;

		label->screenOffset.x() = pt->x();
		label->screenOffset.y() = pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setOffset()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setSelectable
  (JNIEnv *env, jobject obj, jboolean selectable)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		if (!label)
			return;

		label->isSelectable = selectable;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setSelectable()");
	}
}
