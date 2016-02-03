/*
 *  Batch_jni.cpp
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
#import "com_mousebird_maply_Batch.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_Batch_nativeInit
(JNIEnv *env, jclass cls)
{
    BatchClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Batch_initialize
(JNIEnv *env, jobject obj)
{
    try
    {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = new ParticleSystemDrawable::Batch();
        classInfo->setHandle(env, obj, inst);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Batch_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        
        classInfo->clearHandle(env, obj);
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Batch_setStartTime
(JNIEnv *env, jobject obj, jdouble startTime)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->startTime = startTime;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::setStartTime()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_Batch_getStartTime
(JNIEnv *env, jobject obj)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->startTime;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::getStartTime()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Batch_getActive
(JNIEnv *env, jobject obj)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        return inst->active;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::getActive()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Batch_setActive
(JNIEnv *env, jobject obj, jboolean active)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->active = active;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::setActive()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Batch_getLen
(JNIEnv *env, jobject obj)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->len;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::getLen()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Batch_setLen
(JNIEnv *env, jobject obj, jint len)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->len = len;
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::setLen()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Batch_getOffSet
(JNIEnv *env, jobject obj)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->offset;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::getOffSet()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Batch_setOffSet
(JNIEnv *env, jobject obj, jint offset)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->offset = offset;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::setOffSet()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_Batch_getBatchID
(JNIEnv *env, jobject obj)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->batchID;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::getBatchID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Batch_setBatchID
(JNIEnv *env, jobject obj, jint batchID)
{
    try {
        BatchClassInfo *classInfo = BatchClassInfo::getClassInfo();
        ParticleSystemDrawable::Batch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->batchID = batchID;
        
    } catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Batch::setBatchID()");
    }
}
