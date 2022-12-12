/*  CoordSystem_jni.cpp
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

#import "CoordSystem_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_CoordSystem.h"

using namespace WhirlyKit;

template<> CoordSystemRefClassInfo *CoordSystemRefClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_nativeInit
  (JNIEnv *env, jclass cls)
{
	CoordSystemRefClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_initialise
  (JNIEnv *, jobject)
{
	__android_log_print(ANDROID_LOG_WARN, "Maply", "CoordSystem::initialise() called.  This is a base class.  Oops.");
}

static std::mutex disposeMutex;

extern "C"
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
	MAPLY_STD_JNI_CATCH()
}

JNIEXPORT jobject JNICALL MakeCoordSystem(JNIEnv *env,CoordSystemRef coordSys)
{
    CoordSystemRefClassInfo *classInfo = CoordSystemRefClassInfo::getClassInfo(env,"com/mousebird/maply/CoordSystem");
    return classInfo->makeWrapperObject(env,new CoordSystemRef(coordSys));
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_geographicToLocal
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return nullptr;

		Point3d newPt = (*coordSys)->geographicToLocal3d(GeoCoord(pt->x(),pt->y()));
        
//        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "CoordSystem::geographicToLocal() in = (%f,%f), out = (%f,%f,%f)",pt->x(),pt->y(),newPt.x(),newPt.y(),newPt.z());
        
		return MakePoint3d(env,newPt);
	}
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_localToGeographic
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return nullptr;

		GeoCoord newCoord = (*coordSys)->localToGeographic(*pt);
		return MakePoint3d(env,Point3d(newCoord.x(),newCoord.y(),0.0));
	}
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_localToGeocentric
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return nullptr;

		Point3d newCoord = (*coordSys)->localToGeocentric(*pt);
		return MakePoint3d(env,newCoord);
	}
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_geocentricToLocal
  (JNIEnv *env, jobject obj, jobject ptObj)
{
	try
	{
		CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,obj);
		Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,ptObj);
		if (!coordSys || !pt)
			return nullptr;

		Point3d newCoord = (*coordSys)->geocentricToLocal(*pt);
		return MakePoint3d(env,newCoord);
	}
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_CoordSystem_getCanBeWrapped
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto coordSys = CoordSystemRefClassInfo::get(env, obj))
		{
			return (*coordSys)->canBeWrapped();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_CoordSystem_setCanBeWrapped
  (JNIEnv *env, jobject obj, jboolean value)
{
	try
	{
		if (const auto coordSys = CoordSystemRefClassInfo::get(env, obj))
		{
			(*coordSys)->setCanBeWrapped(value);
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_CoordSystem_CoordSystemConvert3d
  (JNIEnv *env, jclass, jobject inSystemObj, jobject outSystemObj, jobject coordObj)
{
    try
    {
		CoordSystemRef *inCoordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,inSystemObj);
		CoordSystemRef *outCoordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,outSystemObj);
        Point3d *pt = Point3dClassInfo::getClassInfo()->getObject(env,coordObj);
        if (!inCoordSys || !outCoordSys || !pt)
            return nullptr;

        Point3d outPt = CoordSystemConvert3d(inCoordSys->get(),outCoordSys->get(),*pt);
        return MakePoint3d(env,outPt);
    }
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_CoordSystem_isValidNative
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (const auto coordSys = CoordSystemRefClassInfo::get(env, obj))
		{
			return (*coordSys)->isValid();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}
