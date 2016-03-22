/*
 *  AngleAxis_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/15.
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
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_AngleAxis.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_AngleAxis_nativeInit
  (JNIEnv *env, jclass cls)
{
	AngleAxisClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_AngleAxis_initialise__
  (JNIEnv *env, jobject obj)
{
	try
	{
		AngleAxisd *angAxis = new AngleAxisd(0.0,Vector3d(0,0,1));
		AngleAxisClassInfo::getClassInfo()->setHandle(env,obj,angAxis);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AngleAxis::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_AngleAxis_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		AngleAxisClassInfo *classInfo = AngleAxisClassInfo::getClassInfo();
		AngleAxisd *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AngleAxis::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_AngleAxis_initialise__DLcom_mousebird_maply_Point3d_2
  (JNIEnv *env, jobject obj, double ang, jobject ptObj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt = classInfo->getObject(env,ptObj);
		if (!pt)
			return;

		AngleAxisd *angAxis = new AngleAxisd(ang,Vector3d(pt->x(),pt->y(),pt->z()));
		AngleAxisClassInfo::getClassInfo()->setHandle(env,obj,angAxis);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in AngleAxis::initialise(double,Point3d)");
	}
}

