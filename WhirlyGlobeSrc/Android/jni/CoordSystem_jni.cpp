/*
 *  CoordSystem_jni.cpp
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
#import "com_mousebird_maply_CoordSystem.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_nativeInit
  (JNIEnv *env, jclass cls)
{
	CoordSystemClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "CoordSystem::initialise() called.  This is a base class.  Oops.");
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		CoordSystemClassInfo *classInfo = CoordSystemClassInfo::getClassInfo();
		CoordSystem *coordSys = classInfo->getObject(env,obj);
		if (!coordSys)
			return;

		delete coordSys;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::dispose()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_geographicToLocal
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystemClassInfo *classInfo = CoordSystemClassInfo::getClassInfo();
		CoordSystem *coordSys = classInfo->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return NULL;

		Point3d newPt = coordSys->geographicToLocal3d(GeoCoord(pt->x(),pt->y()));
		return MakePoint3d(env,newPt);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::geographicToLocal()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_localToGeographic
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return NULL;

		GeoCoord newCoord = coordSys->localToGeographic(*pt);
		return MakePoint3d(env,Point3d(newCoord.x(),newCoord.y(),0.0));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::localToGeographic()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_CoordSystemConvert3d
(JNIEnv *env, jclass cls, jobject inSystemObj, jobject outSystemObj, jobject coordObj)
{
    try
    {
        CoordSystem *inCoordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,inSystemObj);
        CoordSystem *outCoordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,outSystemObj);
        Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,coordObj);
        if (!inCoordSys || !outCoordSys || !pt)
            return NULL;

        Point3d outPt = CoordSystemConvert3d(inCoordSys,outCoordSys,*pt);
        return MakePoint3d(env,outPt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::localToGeographic()");
    }
    
    return NULL;
}
