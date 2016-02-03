/*
 *  ParticleBatch_jni.cpp
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_dispose
(JNIEnv *env, jobject obj)
{
    try {
        ParticleBatchClassInfo *info = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch *batch = info->getObject(env, obj);
        if (!batch)
            return;
        delete batch;
        
        info->clearHandle(env, obj);
        
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
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_addAttributes
(JNIEnv *env, jobject obj, jobject vectObjList) {
    
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch *batch = classInfo->getObject(env, obj);
        if (!batch)
            return;
        jclass listClass = env->GetObjectClass(vectObjList);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass, "iterator", "()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(vectObjList, literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass, "hasNext", "()Z");
        jmethodID next = env->GetMethodID(iterClass, "next", "()Ljava/lang/Object");
        
        SingleVertexAttributeInfoClassInfo *classInfoParticle = SingleVertexAttributeInfoClassInfo::getClassInfo();
        
        while (env->CallBooleanMethod(liter, hasNext)) {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            SingleVertexAttributeInfo *attr = classInfoParticle->getObject(env, javaVecObj);
            if (attr) {
                batch->attrData.push_back(attr);
            }
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch:addAttributes()");
    }
}
