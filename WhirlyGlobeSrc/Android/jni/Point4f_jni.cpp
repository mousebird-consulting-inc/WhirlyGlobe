/*
 *  Point4f_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 23/1/16.
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
#import "com_mousebird_maply_Point4f.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Eigen;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point4f_nativeInit
(JNIEnv *env, jclass cls)
{
    Point4fClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point4f_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        Point4fClassInfo *classInfo = Point4fClassInfo::getClassInfo();
        Point4f *pt = new Point4f();
        classInfo->setHandle(env,obj,pt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4f::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point4f_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        Point4fClassInfo *classInfo = Point4fClassInfo::getClassInfo();
        Point4f *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        delete inst;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4f::dispose()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point4f_getX
(JNIEnv *env, jobject obj)
{
    try
    {
        Point4fClassInfo *classInfo = Point4fClassInfo::getClassInfo();
        Point4f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return 0.0;
        
        return pt->x();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4f::getX()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point4f_getY
(JNIEnv *env, jobject obj)
{
    try
    {
        Point4fClassInfo *classInfo = Point4fClassInfo::getClassInfo();
        Point4f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return 0.0;
        
        return pt->y();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4f::getY()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point4f_getZ
(JNIEnv *env, jobject obj)
{
    try
    {
        Point4fClassInfo *classInfo = Point4fClassInfo::getClassInfo();
        Point4f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return 0.0;
        
        return pt->z();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4f::getZ()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point4f_getW
(JNIEnv *env, jobject obj)
{
    try
    {
        Point4fClassInfo *classInfo = Point4fClassInfo::getClassInfo();
        Point4f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return 0.0;
        
        return pt->w();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4f::getW()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point4f_setValue
(JNIEnv *env, jobject obj, jfloat x, jfloat y, jfloat z, jfloat w)
{
    try
    {
        Point4fClassInfo *classInfo = Point4fClassInfo::getClassInfo();
        Point4f *pt = classInfo->getObject(env,obj);
        if (!pt)
            return;
        pt->x() = x;
        pt->y() = y;
        pt->z() = z;
        pt->w() = w;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point4f::setValue()");
    }
}
