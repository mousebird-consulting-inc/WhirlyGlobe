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
#import "Particles_jni.h"
#import "com_mousebird_maply_ParticleBatch.h"

using namespace WhirlyKit;

template<> ParticleBatchClassInfo *ParticleBatchClassInfo::classInfoObj = NULL;

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
        ParticleBatch_Android *batch = new ParticleBatch_Android();
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
            ParticleBatch_Android *batch = info->getObject(env, obj);
            if (!batch)
                return;
            delete batch;
            
            info->clearHandle(env, obj);
        }
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_setPartSysNative
  (JNIEnv *env, jobject obj, jobject partSysObj)
{
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch_Android *inst = classInfo->getObject(env, obj);
        ParticleSystem *partSys = ParticleSystemClassInfo::getClassInfo()->getObject(env, partSysObj);
        if (!inst || !partSys)
            return;

        inst->partSys = partSys;
        inst->batchSize = partSys->batchSize;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::setPartSysNative");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleBatch_setTime
  (JNIEnv *env, jobject obj, jdouble baseTime)
{
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch_Android *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->baseTime = baseTime;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::setTime");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_ParticleBatch_getTime
  (JNIEnv *env, jobject obj)
{
    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch_Android *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0.0;

        return inst->baseTime;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::getTime");
    }

    return 0.0;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ParticleBatch_addAttribute__Ljava_lang_String_2_3F
  (JNIEnv *env, jobject obj, jstring inName, jfloatArray floatArray)
{
    bool ret = false;

    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch_Android *batch = classInfo->getObject(env, obj);
        if (!batch)
            return false;
        
        jfloat *body = env->GetFloatArrayElements(floatArray, 0);
        jsize len = env->GetArrayLength(floatArray);
        JavaString name(env,inName);
        ret = batch->addAttributeDataFloat(name.cStr,body,len);
        env->ReleaseFloatArrayElements(floatArray, body, 0);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::addAttribute()");
    }

    return ret;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ParticleBatch_addAttribute__Ljava_lang_String_2_3C
  (JNIEnv *env, jobject obj, jstring inName, jcharArray charArray)
{
    bool ret = false;

    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch_Android *batch = classInfo->getObject(env, obj);
        if (!batch)
            return false;
        
        jchar *body = env->GetCharArrayElements(charArray, 0);
        jsize len = env->GetArrayLength(charArray);
        JavaString name(env,inName);
        ret = batch->addAttributeDataChar(name.cStr,(const char *)body,len);
        env->ReleaseCharArrayElements(charArray, body, 0);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::addAttribute()");
    }

    return ret;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ParticleBatch_isValid
        (JNIEnv *env, jobject obj)
{
    bool ret = false;

    try {
        ParticleBatchClassInfo *classInfo = ParticleBatchClassInfo::getClassInfo();
        ParticleBatch_Android *batch = classInfo->getObject(env, obj);
        if (!batch)
            return false;

        // TODO: Actually compare the attributes going in
        ret = true;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleBatch::isValid()");
    }

    return ret;
}