/*
 *  BaseInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/16/15.
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

#import "Base_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_BaseInfo.h"

using namespace WhirlyKit;

template<> BaseInfoClassInfo *BaseInfoClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    BaseInfoClassInfo::getClassInfo(env,cls);
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setVisibleHeightRange
  (JNIEnv *env, jobject obj, jdouble minVis, jdouble maxVis)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->minVis = minVis;
        (*info)->maxVis = maxVis;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setVisibleHeightRange()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_BaseInfo_getVisibleHeightMin
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return DrawVisibleInvalid;
        return (*info)->minVis;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getVisibleHeightMin()");
    }

    return DrawVisibleInvalid;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_BaseInfo_getVisibleHeightMax
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return DrawVisibleInvalid;
        return (*info)->maxVis;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getVisibleHeightMax()");
    }

    return DrawVisibleInvalid;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setViewDistRange
  (JNIEnv *env, jobject obj, jdouble viewMin, jdouble viewMax)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->minViewerDist = viewMin;
        (*info)->maxViewerDist = viewMax;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setViewDistRange()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_BaseInfo_getViewDistRangeMin
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return DrawVisibleInvalid;
        return (*info)->minViewerDist;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getViewDistRangeMin()");
    }

    return DrawVisibleInvalid;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_BaseInfo_getViewDistRangeMax
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return DrawVisibleInvalid;
        return (*info)->maxViewerDist;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getViewDistRangeMax()");
    }

    return DrawVisibleInvalid;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setViewerCenter
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
		Point3dClassInfo *pt3dClassInfo = Point3dClassInfo::getClassInfo();
		Point3d *pt = pt3dClassInfo->getObject(env,ptObj);
        (*info)->viewerCenter = *pt;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setViewerCenter()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_BaseInfo_getViewerCenter
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return NULL;
		jobject ptObj = MakePoint3d(env,(*info)->viewerCenter);
		return ptObj;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getViewerCenter()");
    }

    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setDrawOffset
(JNIEnv *env, jobject obj, jfloat drawOffset)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->drawOffset = drawOffset;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setDrawOffset()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_BaseInfo_getDrawOffset
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return 0.0;
        return (*info)->drawOffset;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getDrawOffset()");
    }

    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setDrawPriority
(JNIEnv *env, jobject obj, jint drawPriority)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->drawPriority = drawPriority;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setDrawPriority()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_BaseInfo_getDrawPriority
(JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return 0;
        return (*info)->drawPriority;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getDrawPriority()");
    }

    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setEnable
(JNIEnv *env, jobject obj, jboolean enable)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->enable = enable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setEnable()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_BaseInfo_getEnable
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return false;
        return (*info)->enable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getEnable()");
    }

    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setFade
(JNIEnv *env, jobject obj, jdouble fade)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->fade = fade;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setFade()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setFadeInOut
  (JNIEnv *env, jobject obj, jdouble fadeIn, jdouble fadeOut)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->fadeIn = fadeIn;
        (*info)->fadeOut = fadeOut;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setFadeInOut()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_BaseInfo_getFadeIn
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return 0.0;
        return (*info)->fadeIn;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getFadeIn()");
    }

    return 0.0;
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_BaseInfo_getFadeOut
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return 0.0;
        return (*info)->fadeOut;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getFadeOut()");
    }

    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setFadeOutTime
  (JNIEnv *env, jobject obj, jdouble fadeOutTime)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        (*info)->fadeOutTime = fadeOutTime;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setFadeOutTime()");
    }
}

/*
 * Class:     com_mousebird_maply_BaseInfo
 * Method:    getFadeOutTime
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_BaseInfo_getFadeOutTime
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return 0.0;
        return (*info)->fadeOutTime;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getFadeOutTime()");
    }

    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setEnableTimes
  (JNIEnv *env, jobject obj, jdouble startEnable, jdouble endEnable)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->startEnable = startEnable;
        (*info)->endEnable = endEnable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setEnableTimes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setShaderID
  (JNIEnv *env, jobject obj, jlong shaderID)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->programID = shaderID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setShader()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_BaseInfo_getShaderID
  (JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return EmptyIdentity;

        return (*info)->programID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getShaderID()");
    }

    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setZBufferRead
  (JNIEnv *env, jobject obj, jboolean val)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->zBufferRead = val;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setZBufferRead()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setZBufferWrite
  (JNIEnv *env, jobject obj, jboolean val)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->zBufferWrite = val;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setZBufferWrite()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setRenderTargetNative
  (JNIEnv *env, jobject obj, jlong targetID)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfoRef *info = classInfo->getObject(env,obj);
        if (!info)
            return;

        (*info)->renderTargetID = targetID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setRenderTargetNative()");
    }
}
