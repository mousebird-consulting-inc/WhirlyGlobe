/*
 *  Point2d_jni.cpp
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
#import "com_mousebird_maply_Point2d.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_nativeInit
  (JNIEnv *env, jclass cls)
{
	Point2dClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakePoint2d(JNIEnv *env,const WhirlyKit::Point2d &pt)
{
	Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo(env,"com/mousebird/maply/Point2d");
	Point2d *newPt = new Point2d(pt);
	return classInfo->makeWrapperObject(env,newPt);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2d *pt = new Point2d();
		Point2dClassInfo::getClassInfo()->setHandle(env,obj,pt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::dispose()");
	}
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getX
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->x();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::getX()");
	}
    
    return 0.0;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point2d_getY
  (JNIEnv *env, jobject obj)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return 0.0;

		return pt->y();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::getY()");
	}
    
    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2d_setValue
  (JNIEnv *env, jobject obj, jdouble x, jdouble y)
{
	try
	{
		Point2dClassInfo *classInfo = Point2dClassInfo::getClassInfo();
		Point2d *pt = classInfo->getObject(env,obj);
		if (!pt)
			return;
		pt->x() = x;
		pt->y() = y;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2d::setValue()");
	}
}
