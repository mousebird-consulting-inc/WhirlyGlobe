/*  InternalLabel_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_nativeInit
  (JNIEnv *env, jclass cls)
{
	LabelClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelClassInfo::set(env,obj,new SingleLabelAndroid());
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_dispose
  (JNIEnv *env, jobject obj)
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
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setSelectID
  (JNIEnv *env, jobject obj, jlong newID)
{
    try
    {
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->selectID = newID;
		}
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
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
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setEndLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
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
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setAnimationRange
  (JNIEnv *env, jobject obj, jdouble startTime, jdouble endTime)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->startTime = startTime;
			label->endTime = endTime;
			label->hasMotion = (endTime > startTime);
        }
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setRotation
  (JNIEnv *env, jobject obj, jdouble rot)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->rotation = rot;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLockRotation
  (JNIEnv *env, jobject obj, jboolean lockRotation)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->keepUpright = !lockRotation;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_addText
  (JNIEnv *env, jobject obj, jintArray textArray, jint len)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			JavaIntArray intArray(env, textArray, false);
			std::vector<int> codePoints;
			codePoints.resize(len);
			for (int ii = 0; ii < intArray.len; ii++)
			{
				codePoints[ii] = intArray.rawInt[ii];
			}
			label->codePointsLines.push_back(codePoints);
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setOffset
  (JNIEnv *env, jobject obj, jobject ptObj)
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
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutPlacement
  (JNIEnv *env, jobject obj, jint layoutPlacement)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->layoutPlacement = layoutPlacement;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutImportance
  (JNIEnv *env, jobject obj, jfloat layoutImportance)
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
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setLayoutSize
  (JNIEnv *env, jobject obj, jdouble sizeX, jdouble sizeY)
{
	try
	{
		if (auto label = LabelClassInfo::get(env,obj))
		{
			label->layoutSize = Point2d(sizeX,sizeY);
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalLabel_setUniqueID
  (JNIEnv *env, jobject obj, jstring uniqueStr)
{
    try
    {
        if (auto label = LabelClassInfo::get(env,obj))
        {
			const JavaString jStr(env, uniqueStr);
			label->uniqueID = jStr.getCString();
		}
    }
	MAPLY_STD_JNI_CATCH()
}
