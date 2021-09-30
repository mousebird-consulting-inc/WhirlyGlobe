/*  Point3d_jni.cpp
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
#import "com_mousebird_maply_Point3d.h"

template<> Point3dClassInfo *Point3dClassInfo::classInfoObj = nullptr;

using namespace Eigen;
using namespace WhirlyKit;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3d_nativeInit
  (JNIEnv *env, jclass cls)
{
	Point3dClassInfo::getClassInfo(env,cls);
}

// Construct a Java Point3d
JNIEXPORT jobject JNICALL MakePoint3d(JNIEnv *env,const Point3d &pt)
{
	Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo(env,"com/mousebird/maply/Point3d");
	jobject newObj = classInfo->makeWrapperObject(env,nullptr);
	Point3d *inst = classInfo->getObject(env,newObj);
	*inst = pt;
	return newObj;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3d_initialise
    (JNIEnv *env, jobject obj,jdouble x,jdouble y,jdouble z)
{
	try
	{
		Point3dClassInfo::set(env,obj,new Point3d(x, y, z));
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3d_dispose
    (JNIEnv *env, jobject obj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        Point3d *inst = classInfo->getObject(env,obj);
        delete inst;
        classInfo->clearHandle(env,obj);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point3d_getX
    (JNIEnv *env, jobject obj)
{
	try
	{
		if (const Point3d *pt = Point3dClassInfo::get(env,obj))
        {
    		return pt->x();
        }
	}
	MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point3d_getY
    (JNIEnv *env, jobject obj)
{
	try
	{
		if (const Point3d *pt = Point3dClassInfo::get(env,obj))
		{
		    return pt->y();
        }
	}
	MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point3d_getZ
    (JNIEnv *env, jobject obj)
{
	try
	{
		if (const Point3d *pt = Point3dClassInfo::get(env,obj))
        {
		    return pt->z();
        }
	}
	MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3d_setValue
    (JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
	try
	{
		if (Point3d *pt = Point3dClassInfo::get(env,obj))
		{
            pt->x() = x;
            pt->y() = y;
            pt->z() = z;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Point3d_cross
    (JNIEnv *env, jobject ptObj, jobject pt2Obj)
{
	try
	{
		Point3dClassInfo *classInfo = Point3dClassInfo::getClassInfo();
		const Point3d *pt = classInfo->getObject(env,ptObj);
		const Point3d *pt2 = classInfo->getObject(env,pt2Obj);
		if (pt && pt2)
		{
            return MakePoint3d(env,pt->cross(*pt2));
		}
	}
	MAPLY_STD_JNI_CATCH()
    return nullptr;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Point3d_norm
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (const Point3d *pt = Point3dClassInfo::get(env,obj))
        {
            return pt->norm();
        }
    }
	MAPLY_STD_JNI_CATCH()
    return 0.0;
}
