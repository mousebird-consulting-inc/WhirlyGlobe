/*
 *  ParticleBatch_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 23/1/16.
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
#import "com_mousebird_maply_ParticleBatch.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_nativeInit
(JNIEnv *env, jclass cls)
{
    ParticleBatchClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_initialise
(JNIEnv *env, jobject obj)
{
    try {
        ParticleBatchClassInfo *info = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch *batch = new ParticleBatch();
        info->setHandle(env, obj, batch);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_dispose
(JNIEnv *env, jobject obj)
{
    try {
        ParticleBatchClassInfo *info = ParticleBatchClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            ParticleBatch *batch = info->getObject(env, obj);
            if (!batch)
                return;
            delete batch;
            
            info->clearHandle(env, obj);
        }
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_setBatchSize
(JNIEnv *env, jobject obj, jint batchSize)
{
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->batchSize = batchSize;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::setBatchSize");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_ParticleBatch_getBatchSize
(JNIEnv *env, jobject obj)
{
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0;
        
        return inst->batchSize;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::getBatchSize");
    }
    
    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_addAttributeValues___3F
(JNIEnv *env, jobject obj, jfloatArray floatArray){
    
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch *batch = classInfo->getObject(env, obj);
        if (!batch)
            return;
        
        jfloat *body = env->GetFloatArrayElements(floatArray, 0);
        jsize len = env->GetArrayLength(floatArray);
        
        batch->attrData.push_back(body);

    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch:addAttributes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_addAttributeValues___3C
(JNIEnv *env, jobject obj, jcharArray charArray)
{
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch *batch = classInfo->getObject(env, obj);
        if (!batch)
            return;
        
        jchar *body = env->GetCharArrayElements(charArray, 0);
        jsize len = env->GetArrayLength(charArray);
        
        batch->attrData.push_back(body);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch:addAttributes()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_ParticleBatch_getAttributesValueSize
(JNIEnv *env, jobject obj)
{
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
            
        return inst->attrData.size();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::getAttributesValueSize");
    }
    
    return -1;
}
