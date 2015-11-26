/*
 *  Quaternion_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/18/15.
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
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_Quaternion.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Quaternion_nativeInit
  (JNIEnv *env, jclass cls)
{
	QuaternionClassInfo::getClassInfo(env,cls);
}

jobject MakeQuaternion(JNIEnv *env,const Eigen::Quaterniond &quat)
{
	// Make a Java Quaternion
	QuaternionClassInfo *classInfo = QuaternionClassInfo::getClassInfo(env,"com/mousebird/maply/Quaternion");
	Quaterniond *newQuat = new Quaterniond(quat);
	return classInfo->makeWrapperObject(env,newQuat);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Quaternion_initialise__
  (JNIEnv *env, jobject obj)
{
	try
	{
		Quaterniond *quat = new Quaterniond();
		QuaternionClassInfo::getClassInfo()->setHandle(env,obj,quat);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Quaternion::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Quaternion_initialise__Lcom_mousebird_maply_Point3d_2Lcom_mousebird_maply_Point3d_2
  (JNIEnv *env, jobject obj, jobject pt1Obj, jobject pt2Obj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt1 = classInfo->getObject(env,pt1Obj);
		Point3d *pt2 = classInfo->getObject(env,pt2Obj);
		if (!pt1 || !pt2)
			return;

		Quaterniond *quat = new Quaterniond();
		*quat = QuatFromTwoVectors(*pt1,*pt2);
		QuaternionClassInfo::getClassInfo()->setHandle(env,obj,quat);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Quaternion::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Quaternion_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		QuaternionClassInfo *classInfo = QuaternionClassInfo::getClassInfo();
		Quaterniond *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Quaternion::dispose()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Quaternion_multiply__Lcom_mousebird_maply_Quaternion_2
  (JNIEnv *env, jobject obj, jobject otherObj)
{
	try
	{
		QuaternionClassInfo *classInfo = QuaternionClassInfo::getClassInfo();
		Quaterniond *quat = classInfo->getObject(env,obj);
		Quaterniond *otherQuat = classInfo->getObject(env,otherObj);
		if (!quat || !otherQuat)
			return NULL;

		Eigen::Quaterniond newQuat = (*quat) * (*otherQuat);

		return MakeQuaternion(env,newQuat);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Quaternion::multiply()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Quaternion_multiply__Lcom_mousebird_maply_Point3d_2
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		QuaternionClassInfo *classInfo = QuaternionClassInfo::getClassInfo();
		Quaterniond *quat = classInfo->getObject(env,obj);
		Point3dClassInfo *pt3dClassInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt = pt3dClassInfo->getObject(env,ptObj);
		if (!quat || !pt)
			return NULL;

		Vector3d newVec = (*quat) * Vector3d(pt->x(),pt->y(),pt->z());

		return MakePoint3d(env,newVec);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Quaternion::multiply()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Quaternion_multiply__Lcom_mousebird_maply_AngleAxis_2
  (JNIEnv *env, jobject obj, jobject angAxisObj)
{
	try
	{
		QuaternionClassInfo *classInfo = QuaternionClassInfo::getClassInfo();
		Quaterniond *quat = classInfo->getObject(env,obj);
		AngleAxisClassInfo *angAxisClassInfo = AngleAxisClassInfo::getClassInfo();
		AngleAxisd *angAxis = angAxisClassInfo->getObject(env,angAxisObj);
		if (!quat || !angAxis)
			return NULL;

		Eigen::Quaterniond newQuat = (*quat) * (*angAxis);

		return MakeQuaternion(env,newQuat);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Quaternion::multiply()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Quaternion_slerp
  (JNIEnv *env, jobject obj, jobject thatObj, jdouble t)
{
	try
	{
		QuaternionClassInfo *classInfo = QuaternionClassInfo::getClassInfo();
		Quaterniond *quat = classInfo->getObject(env,obj);
		Quaterniond *quat2 = classInfo->getObject(env,thatObj);
		if (!quat || !quat2)
			return NULL;

		Eigen::Quaterniond newQuat = (*quat).slerp(t,*quat2);

		return MakeQuaternion(env,newQuat);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Quaternion::slerp()");
	}
    
    return NULL;
}

