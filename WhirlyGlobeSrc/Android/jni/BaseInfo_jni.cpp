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

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_BaseInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    BaseInfoClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setEnable
(JNIEnv *env, jobject obj, jboolean enable)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        info->enable = enable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setDrawOffset
(JNIEnv *env, jobject obj, jfloat drawOffset)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        info->drawOffset = drawOffset;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setDrawOffset()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setDrawPriority
(JNIEnv *env, jobject obj, jint drawPriority)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        info->drawPriority = drawPriority;
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
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return 0;
        return info->drawPriority;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getDrawPriority()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setMinVis
(JNIEnv *env, jobject obj, jfloat minVis)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        info->minVis = minVis;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setMinViz()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_BaseInfo_getMinVis
(JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return 0.0;
        return info->minVis;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getMinVis()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setMaxVis
(JNIEnv *env, jobject obj, jfloat maxVis)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        info->maxVis = maxVis;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setMaxVis()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_BaseInfo_getMaxVis
(JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return 0.0;
        return info->maxVis;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getMaxVis()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setFade
(JNIEnv *env, jobject obj, jfloat fade)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return;
        info->fade = fade;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setFade()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_BaseInfo_getFade
(JNIEnv *env, jobject obj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        if (!info)
            return 0.0;
        return info->fade;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::getFade()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BaseInfo_setShader
(JNIEnv *env, jobject obj, jobject shaderObj)
{
    try
    {
        BaseInfoClassInfo *classInfo = BaseInfoClassInfo::getClassInfo();
        BaseInfo *info = classInfo->getObject(env,obj);
        OpenGLES2Program *shader = OpenGLES2ProgramClassInfo::getClassInfo()->getObject(env,shaderObj);
        if (!info || !shader)
            return;
        
        info->programID = shader->getId();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BaseInfo::setShader()");
    }
}
