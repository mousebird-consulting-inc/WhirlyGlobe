/*  WideVectorInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/8/17.
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

#include <classInfo/Maply_jni.h>
#import "Vectors_jni.h"
#import "com_mousebird_maply_WideVectorInfo.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> WideVectorInfoClassInfo *WideVectorInfoClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    WideVectorInfoClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        WideVectorInfoRef *vecInfo = new WideVectorInfoRef(new WideVectorInfo());
        WideVectorInfoClassInfo::getClassInfo()->setHandle(env,obj,vecInfo);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            WideVectorInfoRef *vecInfo = classInfo->getObject(env,obj);
            if (!vecInfo)
                return;
            delete vecInfo;
            
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::dispose()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        WideVectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
        (*vecInfo)->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::setColor()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setLineWidth
(JNIEnv *env, jobject obj, jfloat val)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        WideVectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
        (*vecInfo)->width = val;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::setLineWidth()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setTextureRepeatLength
(JNIEnv *env, jobject obj, jdouble repeatLen)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        WideVectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
        (*vecInfo)->repeatSize = repeatLen;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::setTextureRepeatLength()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setEdgeFalloff
(JNIEnv *env, jobject obj, jdouble edgeFalloff)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        WideVectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
        (*vecInfo)->edgeSize = edgeFalloff;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::setEdgeFalloff()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setJoinTypeNative
(JNIEnv *env, jobject obj, jint joinType)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        WideVectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
        (*vecInfo)->joinType = (WideVectorLineJoinType)joinType;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::setJoinTypeNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setMitreLimit
(JNIEnv *env, jobject obj, jdouble mitreLimit)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        WideVectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
        (*vecInfo)->miterLimit = mitreLimit;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::setMitreLimit()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setTexID
(JNIEnv *env, jobject obj, jlong texID)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        WideVectorInfoRef *vecInfo = classInfo->getObject(env,obj);
        if (!vecInfo)
            return;
        (*vecInfo)->texID = texID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorInfo::setTexId()");
    }
}

