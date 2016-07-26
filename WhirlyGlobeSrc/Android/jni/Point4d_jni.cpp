/*
 *  Point4d_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/21/15.
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
#import "com_mousebird_maply_Point4d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point4d_nativeInit
  (JNIEnv *env, jclass cls)
{
	Point4dClassInfo::getClassInfo(env,cls);
}

// Construct a Java Point4d
JNIEXPORT jobject JNICALL MakePoint4d(JNIEnv *env,const Point4d &pt)
{
	Point4dClassInfo *classInfo = Point4dClassInfo::getClassInfo(env,"com/mousebird/maply/Point4d");
	Point4d *newPt = new Point4d(pt);
	return classInfo->makeWrapperObject(env,newPt);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point4d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point4dClassInfo *classInfo = Point4dClassInfo::getClassInfo();
		Point4d *pt = new Point4d();
		classInfo->setHandle(env,obj,pt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4d::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point4d_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point4dClassInfo *classInfo = Point4dClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Point4d *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4d::dispose()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point4d_getX
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point4dClassInfo *classInfo = Point4dClassInfo::getClassInfo();
		Point4d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->x();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4d::getX()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point4d_getY
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point4dClassInfo *classInfo = Point4dClassInfo::getClassInfo();
		Point4d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4d::getY()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point4d_getZ
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point4dClassInfo *classInfo = Point4dClassInfo::getClassInfo();
		Point4d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->z();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4d::getZ()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point4d_getW
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point4dClassInfo *classInfo = Point4dClassInfo::getClassInfo();
		Point4d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->w();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4d::getW()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point4d_setValue
  (JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z, jdouble w)
{
	try
	{
		Point4dClassInfo *classInfo = Point4dClassInfo::getClassInfo();
		Point4d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return;
		pt->x() = x;
		pt->y() = y;
		pt->z() = z;
		pt->w() = w;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4d::setValue()");
	}
}
