/*
 *  Point2f_jni.cpp
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
#import "com_mousebird_maply_Point2f.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Eigen;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2f_nativeInit
(JNIEnv *env, jclass cls)
{
    Point2fClassInfo::getClassInfo(env, cls);
}

JNIEXPORT jobject JNICALL MakePoint2f(JNIEnv *env,const WhirlyKit::Point2f &pt)
{
    Point2fClassInfo *classInfo = Point2fClassInfo::getClassInfo(env, "com/mousebird/maply/Point2f");
    Point2f *newPt = new Point2f(pt);
    return classInfo->makeWrapperObject(env, newPt);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2f_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        Point2fClassInfo *classInfo = Point2fClassInfo::getClassInfo();
        Point2f *pt = new Point2f();
        classInfo->setHandle(env, obj, pt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2f::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2f_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        Point2fClassInfo *classInfo = Point2fClassInfo::getClassInfo();
        Point2f *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2f::dispose()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point2f_getX
(JNIEnv *env, jobject obj)
{
    try
    {
        Point2fClassInfo *classInfo = Point2fClassInfo::getClassInfo();
        Point2f *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0.0;
        
        return inst->x();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2f::getX()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Point2f_getY
(JNIEnv *env, jobject obj)
{
    try
    {
        Point2fClassInfo *classInfo = Point2fClassInfo::getClassInfo();
        Point2f *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0.0;
        
        return inst->y();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2f::getY()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Point2f_setValue
(JNIEnv *env, jobject obj, jfloat x, jfloat y)
{
    try
    {
        Point2fClassInfo *classInfo = Point2fClassInfo::getClassInfo();
        Point2f *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->x() = x;
        inst->y() = y;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Point2f::setValue()");
    }
}
