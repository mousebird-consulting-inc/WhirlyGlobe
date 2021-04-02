/*  InternalMarker_jni.cpp
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
#import "Base_jni.h"
#import "com_mousebird_maply_InternalMarker.h"

using namespace WhirlyKit;

template<> MarkerClassInfo *MarkerClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_nativeInit(JNIEnv *env, jclass cls)
{
	MarkerClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_initialise(JNIEnv *env, jobject obj)
{
	try
	{
		auto marker = new Marker();
        marker->rotation = 0.0;
        marker->lockRotation = false;
		MarkerClassInfo::getClassInfo()->setHandle(env,obj,marker);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::initialise()");
	}
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		const auto classInfo = MarkerClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		auto marker = classInfo->getObject(env,obj);
		delete marker;
		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::dispose()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setSelectID
	(JNIEnv *env, jobject obj, jlong selectID)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->isSelectable = true;
			marker->selectID = selectID;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setSelectID()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			if (auto pt = Point2dClassInfo::get(env, ptObj))
			{
				marker->loc.x() = pt->x();
				marker->loc.y() = pt->y();
			}
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setLoc()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setEndLoc
	(JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			if (auto pt = Point2dClassInfo::get(env, ptObj))
			{
				marker->endLoc.x() = pt->x();
				marker->endLoc.y() = pt->y();
			}
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setEndLoc()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setAnimationRange
	(JNIEnv *env, jobject obj, jdouble startTime, jdouble endTime)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->startTime = startTime;
			marker->endTime = endTime;
			if (startTime < endTime)
			{
				marker->hasMotion = true;
			}
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setAnimationRange()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setColor
	(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->color = RGBAColor(r*255,g*255,b*255,a*255);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setColor()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_addTexID
	(JNIEnv *env, jobject obj, jlong texID)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->texIDs.push_back(texID);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::addTexID()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setRotation
	(JNIEnv *env, jobject obj, jdouble rot)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->lockRotation = true;
			marker->rotation = rot;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setRotation()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setLockRotation
	(JNIEnv *env, jobject obj, jboolean lockRot)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->lockRotation = lockRot;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setLockRotation()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setSize
	(JNIEnv *env, jobject obj, jdouble width, jdouble height)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->width = width;
			marker->height = height;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setSize()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setLayoutSize
	(JNIEnv *env, jobject obj, jdouble width, jdouble height)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->layoutWidth = width;
			marker->layoutHeight = height;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setLayoutSize()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setOffset
	(JNIEnv *env, jobject obj, jdouble offX,jdouble offY)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->offset.x() = offX;
			marker->offset.y() = offY;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setOffset()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setPeriod
	(JNIEnv *env, jobject obj, jdouble period)
{
    try
    {
        if (auto marker = MarkerClassInfo::get(env,obj))
		{
        	marker->period = period;
		}
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setPeriod()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setVertexAttributes
	(JNIEnv *env, jobject obj, jobjectArray vertAttrArray)
{
    try
    {
        if (auto marker = MarkerClassInfo::get(env,obj))
        {
			marker->vertexAttrs.clear();
			
			const auto vertClassInfo = SingleVertexAttributeClassInfo::getClassInfo();
			
			JavaObjectArrayHelper vertAttrHelp(env, vertAttrArray);
			while (jobject vertAttrObj = vertAttrHelp.getNextObject())
			{
				auto vertAttr = vertClassInfo->getObject(env, vertAttrObj);
				marker->vertexAttrs.insert(*vertAttr);
			}
		}
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setVertexAttributes()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setLayoutImportance
	(JNIEnv *env, jobject obj, jfloat importance)
{
	try
	{
		if (auto marker = MarkerClassInfo::get(env,obj))
		{
			marker->layoutImportance = importance;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in InternalMarker::setLayoutImportance()");
	}
}
