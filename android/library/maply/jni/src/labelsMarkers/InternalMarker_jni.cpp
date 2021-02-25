/*
 *  InternalMarker_jni.cpp
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
#import "Base_jni.h"
#import "com_mousebird_maply_InternalMarker.h"

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
        marker->rotation = 0.0;
        marker->lockRotation = false;
		MarkerClassInfo::getClassInfo()->setHandle(env,obj,marker);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Marker *marker = classInfo->getObject(env,obj);
            if (!marker)
                return;
            delete marker;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setSelectID
  (JNIEnv *env, jobject obj, jlong selectID)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->isSelectable = true;
		marker->selectID = selectID;
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setEndLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!marker || !pt)
			return;

		marker->endLoc.x() = pt->x();
		marker->endLoc.y() = pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setEndLoc()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setAnimationRange
  (JNIEnv *env, jobject obj, jdouble startTime, jdouble endTime)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

        marker->startTime = startTime;
        marker->endTime = endTime;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setAnimationRange()");
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setRotation
  (JNIEnv *env, jobject obj, jdouble rot)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->lockRotation = true;
		marker->rotation = rot;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setRotation()");
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setSize
  (JNIEnv *env, jobject obj, jdouble width, jdouble height)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->width = width;
		marker->height = height;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setSize()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setLayoutSize
  (JNIEnv *env, jobject obj, jdouble width, jdouble height)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->layoutWidth = width;
		marker->layoutHeight = height;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setLayoutSize()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setOffset
(JNIEnv *env, jobject obj, jdouble offX,jdouble offY)
{
	try
	{
		MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
		Marker *marker = classInfo->getObject(env,obj);
		if (!marker)
			return;

		marker->offset.x() = offX;
		marker->offset.y() = offY;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setOffset()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setPeriod
(JNIEnv *env, jobject obj, jdouble period)
{
    try
    {
        MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
        Marker *marker = classInfo->getObject(env,obj);
        if (!marker)
            return;

        marker->period = period;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setPeriod()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_InternalMarker_setVertexAttributes
(JNIEnv *env, jobject obj, jobjectArray vertAttrArray)
{
    try {
        MarkerClassInfo *classInfo = MarkerClassInfo::getClassInfo();
        Marker *marker = classInfo->getObject(env,obj);
        if (!marker)
            return;

        marker->vertexAttrs.clear();
        SingleVertexAttributeClassInfo *vertClassInfo = SingleVertexAttributeClassInfo::getClassInfo();
        JavaObjectArrayHelper vertAttrHelp(env,vertAttrArray);
        while (jobject vertAttrObj = vertAttrHelp.getNextObject()) {
			SingleVertexAttribute *vertAttr = vertClassInfo->getObject(env,vertAttrObj);
			marker->vertexAttrs.insert(*vertAttr);
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in InternalMarker::setVertexAttributes()");
    }
}
