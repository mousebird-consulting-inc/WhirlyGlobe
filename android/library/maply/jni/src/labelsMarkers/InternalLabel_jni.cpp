/*
 *  InternalLabel_jni.cpp
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

#import "LabelsAndMarkers_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_InternalLabel.h"

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

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            SingleLabel *label = classInfo->getObject(env,obj);
            if (!label)
                return;
            delete label;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setSelectID
(JNIEnv *env, jobject obj, jlong newID)
{
    try
    {
        LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
        SingleLabel *label = classInfo->getObject(env,obj);
        if (!label)
            return;
        
        label->selectID = newID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setSelectID()");
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setEndLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!label || !pt)
			return;

		label->endLoc.x() = pt->x();
		label->endLoc.y() = pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setEndLoc()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setAnimationRange
  (JNIEnv *env, jobject obj, jdouble startTime, jdouble endTime)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		if (!label)
			return;

        label->startTime = startTime;
        label->endTime = endTime;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setAnimationRange()");
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLockRotation
  (JNIEnv *env, jobject obj, jboolean lockRotation)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		if (!label)
			return;

		label->keepUpright = !lockRotation;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setLockRotation()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_addText
(JNIEnv *env, jobject obj, jintArray textArray, jint len)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabelAndroid *label = classInfo->getObject(env,obj);
		if (!label)
			return;

        JavaIntArray intArray(env,textArray);
        std::vector<int> codePoints;
        codePoints.resize(len);
        for (int ii=0;ii<intArray.len;ii++)
            codePoints[ii] = intArray.rawInt[ii];
        label->codePointsLines.push_back(codePoints);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::addText()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setOffset
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabelAndroid *label = classInfo->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutPlacement
		(JNIEnv *env, jobject obj, jint layoutPlacement)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		if (!label)
			return;
		label->layoutPlacement = layoutPlacement;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setLayoutPlacement()");
	}

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutImportance
(JNIEnv *env, jobject obj, jfloat layoutImportance)
{
    try
    {
        LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
        SingleLabel *label = classInfo->getObject(env,obj);
        if (!label)
            return;
        label->layoutImportance = layoutImportance;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setLayoutImportance()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutSize
		(JNIEnv *env, jobject obj, jdouble sizeX, jdouble sizeY)
{
	try
	{
		LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
		SingleLabel *label = classInfo->getObject(env,obj);
		if (!label)
			return;
		label->layoutSize = Point2d(sizeX,sizeY);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setLayoutSize()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setUniqueID
  (JNIEnv *env, jobject obj, jstring uniqueStr)
{
    try
    {
        LabelClassInfo *classInfo = LabelClassInfo::getClassInfo();
        SingleLabel *label = classInfo->getObject(env,obj);
        if (!label)
            return;
        JavaString jStr(env,uniqueStr);
        label->uniqueID = jStr.cStr;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalLabel::setUniqueID()");
    }
}
