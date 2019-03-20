/*
 *  CoordSystem_jni.cpp
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

#import "CoordSystem_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_CoordSystem.h"

using namespace WhirlyKit;

template<> CoordSystemRefClassInfo *CoordSystemRefClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_nativeInit
  (JNIEnv *env, jclass cls)
{
	CoordSystemRefClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_initialise
  (JNIEnv *env, jobject obj)
{
	__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "CoordSystem::initialise() called.  This is a base class.  Oops.");
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		CoordSystemRefClassInfo *classInfo = CoordSystemRefClassInfo::getClassInfo();
        if (!classInfo)
            return;
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            CoordSystemRef *coordSys = classInfo->getObject(env,obj);
            if (!coordSys)
                return;

            delete coordSys;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::dispose()");
	}
}

JNIEXPORT jobject JNICALL MakeCoordSystem(JNIEnv *env,CoordSystemRef coordSys)
{
    CoordSystemRefClassInfo *classInfo = CoordSystemRefClassInfo::getClassInfo(env,"com/mousebird/maply/CoordSystem");
    return classInfo->makeWrapperObject(env,new CoordSystemRef(coordSys));
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_geographicToLocal
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return NULL;

		Point3d newPt = (*coordSys)->geographicToLocal3d(GeoCoord(pt->x(),pt->y()));
        
//        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "CoordSystem::geographicToLocal() in = (%f,%f), out = (%f,%f,%f)",pt->x(),pt->y(),newPt.x(),newPt.y(),newPt.z());
        
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
		CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return NULL;

		GeoCoord newCoord = (*coordSys)->localToGeographic(*pt);
		return MakePoint3d(env,Point3d(newCoord.x(),newCoord.y(),0.0));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::localToGeographic()");
	}
    
    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_localToGeocentric
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return NULL;

		Point3d newCoord = (*coordSys)->localToGeocentric(*pt);
		return MakePoint3d(env,newCoord);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::localToGeocentric()");
	}

    return NULL;
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_geocentricToLocal
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return NULL;

		Point3d newCoord = (*coordSys)->geocentricToLocal(*pt);
		return MakePoint3d(env,newCoord);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::geocentricToLocal()");
	}

    return NULL;
}


JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_CoordSystemConvert3d
(JNIEnv *env, jclass cls, jobject inSystemObj, jobject outSystemObj, jobject coordObj)
{
    try
    {
		CoordSystemRef *inCoordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,inSystemObj);
		CoordSystemRef *outCoordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,outSystemObj);
        Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,coordObj);
        if (!inCoordSys || !outCoordSys || !pt)
            return NULL;

        Point3d outPt = CoordSystemConvert3d(inCoordSys->get(),outCoordSys->get(),*pt);
        return MakePoint3d(env,outPt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in CoordSystem::localToGeographic()");
    }
    
    return NULL;
}
