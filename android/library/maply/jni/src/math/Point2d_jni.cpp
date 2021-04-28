/*  Point2d_jni.cpp
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

#import <jni.h>
#import "Geometry_jni.h"
#import "com_mousebird_maply_Point2d.h"

template<> Point2dClassInfo *Point2dClassInfo::classInfoObj = nullptr;

using namespace Eigen;
using namespace WhirlyKit;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_nativeInit(JNIEnv *env, jclass cls)
{
	Point2dClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakePoint2d(JNIEnv *env)
{
	auto classInfo = Point2dClassInfo::getClassInfo(env,"com/mousebird/maply/Point2d");
	return classInfo->makeWrapperObject(env,nullptr);
}

JNIEXPORT jobject JNICALL MakePoint2d(JNIEnv *env,const WhirlyKit::Point2d &pt)
{
	jobject newObj = MakePoint2d(env);
	auto inst = Point2dClassInfo::getClassInfo()->getObject(env,newObj);
	*inst = pt;
	return newObj;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_initialise(JNIEnv *env, jobject obj)
{
	try
	{
		auto pt = new Point2d(0, 0);
		Point2dClassInfo::getClassInfo()->setHandle(env,obj,pt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Point2d::initialise()");
	}
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		Point2d *inst = classInfo->getObject(env,obj);
		delete inst;
		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Point2d::dispose()");
	}
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getX(JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *pt = classInfo->getObject(env,obj);
		return pt ? pt->x() : 0.0;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Point2d::getX()");
	}
    return 0.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getY(JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *pt = classInfo->getObject(env,obj);
		return pt ? pt->y() : 0.0;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Point2d::getY()");
	}
    
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_setValue(JNIEnv *env, jobject obj, jdouble x, jdouble y)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		if (auto pt = classInfo->getObject(env,obj))
		{
			pt->x() = x;
			pt->y() = y;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in Point2d::setValue()");
	}
}
