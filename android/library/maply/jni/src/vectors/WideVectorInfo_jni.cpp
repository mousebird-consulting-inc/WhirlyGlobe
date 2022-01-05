/*  WideVectorInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/8/17.
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
        WideVectorInfoClassInfo::set(env, obj, new WideVectorInfoRef(std::make_shared<WideVectorInfo>()));
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        WideVectorInfoClassInfo *classInfo = WideVectorInfoClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env,obj);
        classInfo->clearHandle(env,obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setColorInt
  (JNIEnv *env, jobject obj, jint r, jint g, jint b, jint a)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->color = RGBAColor(r,g,b,a);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_WideVectorInfo_getColor
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->color.asARGBInt();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setColor
        (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    Java_com_mousebird_maply_WideVectorInfo_setColorInt(env, obj,
        (jint)(r*255.0f),(jint)(g*255.0f),(jint)(b*255.0f),(jint)(a*255.0f));
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setLineWidth
  (JNIEnv *env, jobject obj, jfloat val)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->width = val;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_WideVectorInfo_getLineWidth
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->width;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0f;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setTextureRepeatLength
  (JNIEnv *env, jobject obj, jdouble repeatLen)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->repeatSize = repeatLen;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_WideVectorInfo_getTextureRepeatLength
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->repeatSize;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setEdgeFalloff
  (JNIEnv *env, jobject obj, jdouble edgeFalloff)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->edgeSize = edgeFalloff;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_WideVectorInfo_getEdgeFalloff
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->edgeSize;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setJoinTypeNative
  (JNIEnv *env, jobject obj, jint joinType)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->joinType = (WideVectorLineJoinType)joinType;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_WideVectorInfo_getJoinTypeNative
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->joinType;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setMitreLimit
  (JNIEnv *env, jobject obj, jdouble mitreLimit)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->miterLimit = mitreLimit;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_WideVectorInfo_getMitreLimit
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->miterLimit;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setTexID
  (JNIEnv *env, jobject obj, jlong texID)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->texID = texID;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_WideVectorInfo_getTexID
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->texID;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setSelectable
        (JNIEnv *env, jobject obj, jboolean enable)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->selectable = enable;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_WideVectorInfo_getSelectable
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->selectable;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setOffset
        (JNIEnv *env, jobject obj, jdouble offset)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->offset = offset;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_WideVectorInfo_getOffset
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->offset;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorInfo_setCloseAreals
  (JNIEnv *env, jobject obj, jboolean close)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->closeAreals = close;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_WideVectorInfo_getCloseAreals
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = WideVectorInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->closeAreals;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}
