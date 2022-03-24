/*  MarkerInfo_jni.cpp
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

#import "LabelsAndMarkers_jni.h"
#import "Expressions_jni.h"
#import "com_mousebird_maply_MarkerInfo.h"

using namespace WhirlyKit;

template<> MarkerInfoClassInfo *MarkerInfoClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_nativeInit
  (JNIEnv *env, jclass cls)
{
	MarkerInfoClassInfo::getClassInfo(env,cls);
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		MarkerInfoRef *info = new MarkerInfoRef(std::make_shared<MarkerInfo>(true));
        std::lock_guard<std::mutex> lock(disposeMutex);
		delete MarkerInfoClassInfo::get(env, obj);
		MarkerInfoClassInfo::set(env,obj,info);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MarkerInfoClassInfo *classInfo = MarkerInfoClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env,obj);
        classInfo->clearHandle(env,obj);
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setColorARGB
        (JNIEnv *env, jobject obj, jint color)
{
    try
    {
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            (*info)->color = RGBAColor::FromARGBInt(color);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setComponents
  (JNIEnv *env, jobject obj, jint r, jint g, jint b, jint a)
{
	try
	{
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            (*info)->color = RGBAColor(r, g, b, a);
        }
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_MarkerInfo_getColorARGB
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            return (*info)->color.asARGBInt();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setColorExp
  (JNIEnv *env, jobject obj, jobject exprObj)
{
    try
    {
        if (const auto &mrkInfo = *MarkerInfoClassInfo::get(env, obj))
        {
            mrkInfo->colorExp.reset();
            if (exprObj)
            {
                if (const auto wrap = ColorExpressionClassInfo::get(env, exprObj))
                {
                    mrkInfo->colorExp = *wrap;
                }
            }
            mrkInfo->hasExp = mrkInfo->colorExp || mrkInfo->opacityExp || mrkInfo->scaleExp;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_MarkerInfo_getColorExp
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto mrkInfo = MarkerInfoClassInfo::get(env, obj))
        {
            if (const auto exp = (*mrkInfo)->colorExp)
            {
                return MakeWrapper(env, std::move(exp));
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setOpacityExp
  (JNIEnv *env, jobject obj, jobject exprObj)
{
    try
    {
        if (const auto &mrkInfo = *MarkerInfoClassInfo::get(env, obj))
        {
            mrkInfo->opacityExp.reset();
            if (exprObj)
            {
                if (const auto wrap = FloatExpressionClassInfo::get(env, exprObj))
                {
                    mrkInfo->opacityExp = *wrap;
                }
            }
            mrkInfo->hasExp = mrkInfo->colorExp || mrkInfo->opacityExp || mrkInfo->scaleExp;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_MarkerInfo_geOpacityExp
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto mrkInfo = MarkerInfoClassInfo::get(env, obj))
        {
            if (const auto exp = (*mrkInfo)->opacityExp)
            {
                return MakeWrapper(env, std::move(exp));
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setScaleExp
  (JNIEnv *env, jobject obj, jobject exprObj)
{
    try
    {
        if (const auto &mrkInfo = *MarkerInfoClassInfo::get(env, obj))
        {
            mrkInfo->scaleExp.reset();
            if (exprObj)
            {
                if (const auto wrap = FloatExpressionClassInfo::get(env, exprObj))
                {
                    mrkInfo->scaleExp = *wrap;
                }
            }
            mrkInfo->hasExp = mrkInfo->colorExp || mrkInfo->opacityExp || mrkInfo->scaleExp;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_MarkerInfo_getScaleExp
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto mrkInfo = MarkerInfoClassInfo::get(env, obj))
        {
            if (const auto exp = (*mrkInfo)->scaleExp)
            {
                return MakeWrapper(env, std::move(exp));
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}

extern "C"
JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_MarkerInfo_getLayoutImportance
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            return (*info)->layoutImportance;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1.0f;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setLayoutImportance
  (JNIEnv *env, jobject obj, jfloat import)
{
    try
    {
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            (*info)->layoutImportance = import;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_MarkerInfo_getClusterGroup
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            return (*info)->clusterGroup;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setClusterGroup
(JNIEnv *env, jobject obj, jint clusterGroup)
{
    try
    {
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            (*info)->clusterGroup = clusterGroup;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setZoomSlot
        (JNIEnv *env, jobject obj, jint slot)
{
    try
    {
        if (const auto vecInfo = MarkerInfoClassInfo::get(env,obj))
        {
            (*vecInfo)->zoomSlot = slot;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_MarkerInfo_getZoomSlot
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto vecInfo = MarkerInfoClassInfo::get(env,obj))
        {
            return (*vecInfo)->zoomSlot;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_MarkerInfo_getShaderProgramId
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            return (*info)->programID;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerInfo_setShaderProgramId
        (JNIEnv *env, jobject obj, jlong id)
{
    try
    {
        if (const auto info = MarkerInfoClassInfo::get(env,obj))
        {
            (*info)->programID = id;
        }
    }
    MAPLY_STD_JNI_CATCH()
}
