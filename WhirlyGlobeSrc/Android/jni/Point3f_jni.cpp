/*
 *  Point3f_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#import "com_mousebird_maply_Point3f.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Eigen;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3f_nativeInit
(JNIEnv *env, jclass cls)
{
    Point3fClassInfo::getClassInfo(env, cls);
}

// Construct a Java Point3f
JNIEXPORT jobject JNICALL MakePoint3f(JNIEnv *env,const Point3f &pt)
{
    Point3fClassInfo *classInfo = Point3fClassInfo::getClassInfo(env,"com/mousebird/maply/Point3f");
    Point3f *newPt = new Point3f(pt);
    return classInfo->makeWrapperObject(env,newPt);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3f_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        Point3fClassInfo *classInfo = Point3fClassInfo::getClassInfo();
        Point3f *pt = new Point3f();
        classInfo->setHandle(env,obj,pt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3f::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3f_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        Point3fClassInfo *classInfo = Point3fClassInfo::getClassInfo();
        Point3f *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        delete inst;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3f::dispose()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point3f_getX
(JNIEnv *env, jobject obj)
{
    try
    {
        Point3fClassInfo *classInfo = Point3fClassInfo::getClassInfo();
        Point3f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return 0.0;
        
        return pt->x();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3f::getX()");
    }
    
    return 0.0;
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point3f_getY
(JNIEnv *env, jobject obj)
{
    try
    {
        Point3fClassInfo *classInfo = Point3fClassInfo::getClassInfo();
        Point3f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return 0.0;
        
        return pt->y();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3f::getY()");
    }
    
    return 0.0;
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point3f_getZ
(JNIEnv *env, jobject obj)
{
    try
    {
        Point3fClassInfo *classInfo = Point3fClassInfo::getClassInfo();
        Point3f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return 0.0;
        
        return pt->z();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3f::getZ()");
    }
    
    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point3f_setValue
(JNIEnv *env, jobject obj, jfloat x, jfloat y, jfloat z)
{
    try
    {
        Point3fClassInfo *classInfo = Point3fClassInfo::getClassInfo();
        Point3f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return;
        pt->x() = x;
        pt->y() = y;
        pt->z() = z;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point3f::setValue()");
    }
}
