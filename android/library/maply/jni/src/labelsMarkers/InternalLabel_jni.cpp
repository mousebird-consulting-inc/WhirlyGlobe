/*  InternalLabel_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "LabelsAndMarkers_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_InternalLabel.h"

using namespace WhirlyKit;

typedef JavaClassInfo<WhirlyKit::SingleLabelAndroid> LabelClassInfo;
template<> LabelClassInfo *LabelClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_nativeInit(JNIEnv *env, jclass cls)
{
	LabelClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_initialise(JNIEnv *env, jobject obj)
{
	try
	{
		SingleLabelAndroid *label = new SingleLabelAndroid();
		LabelClassInfo::getClassInfo()->setHandle(env,obj,label);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::initialise()");
	}
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		const auto classInfo = LabelClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		if (auto label = LabelClassInfo::get(env,obj))
		{
			delete label;
			classInfo->clearHandle(env, obj);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::dispose()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setSelectID(JNIEnv *env, jobject obj, jlong newID)
{
    try
    {
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->selectID = newID;
		}
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setSelectID()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLoc(JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			if (const auto pt = Point2dClassInfo::getClassInfo()->getObject(env, ptObj))
			{
				label->loc.x() = pt->x();
				label->loc.y() = pt->y();
			}
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setLoc()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setEndLoc(JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			if (const auto pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj))
			{
				label->endLoc.x() = pt->x();
				label->endLoc.y() = pt->y();
			}
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setEndLoc()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setAnimationRange(JNIEnv *env, jobject obj, jdouble startTime, jdouble endTime)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->startTime = startTime;
			label->endTime = endTime;
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setAnimationRange()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setRotation(JNIEnv *env, jobject obj, jdouble rot)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->rotation = rot;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setRotation()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLockRotation(JNIEnv *env, jobject obj, jboolean lockRotation)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->keepUpright = !lockRotation;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setLockRotation()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_addText(JNIEnv *env, jobject obj, jintArray textArray, jint len)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			JavaIntArray intArray(env, textArray);
			std::vector<int> codePoints;
			codePoints.resize(len);
			for (int ii = 0; ii < intArray.len; ii++)
			{
				codePoints[ii] = intArray.rawInt[ii];
			}
			label->codePointsLines.push_back(codePoints);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::addText()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setOffset(JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			if (const auto pt = Point2dClassInfo::get(env, ptObj))
			{
				label->screenOffset.x() = pt->x();
				label->screenOffset.y() = pt->y();
			}
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setOffset()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutPlacement(JNIEnv *env, jobject obj, jint layoutPlacement)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->layoutPlacement = layoutPlacement;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setLayoutPlacement()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutImportance(JNIEnv *env, jobject obj, jfloat layoutImportance)
{
    try
    {
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->layoutImportance = layoutImportance;
			if (layoutImportance < MAXFLOAT)
			{
				label->layoutEngine = true;
			}
		}
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setLayoutImportance()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutSize(JNIEnv *env, jobject obj, jdouble sizeX, jdouble sizeY)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->layoutSize = Point2d(sizeX,sizeY);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setLayoutSize()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setUniqueID(JNIEnv *env, jobject obj, jstring uniqueStr)
{
    try
    {
        if (auto label = LabelClassInfo::get(env,obj))
        {
			const JavaString jStr(env, uniqueStr);
			label->uniqueID = jStr.getCString();
		}
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalLabel::setUniqueID()");
    }
}
