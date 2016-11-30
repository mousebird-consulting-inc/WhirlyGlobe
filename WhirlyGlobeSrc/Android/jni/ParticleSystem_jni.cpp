/*
 *  ParticleSystem_jni.cpp
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
#import "com_mousebird_maply_ParticleSystem.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_nativeInit
(JNIEnv *env, jclass cls)
{
    ParticleSystemClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_initialise
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = new ParticleSystem();
        classInfo->setHandle(env, obj, inst);
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_dispose
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            ParticleSystem *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            
            classInfo->clearHandle(env, obj);
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ParticleSystem_getIdent
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return EmptyIdentity;
        return inst->getId();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getIdent()");
    }
    
    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setName
(JNIEnv *env, jobject obj, jstring name)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        JavaString jstr(env, name);
        inst->name = jstr.cStr;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setName()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setDrawPriority
(JNIEnv * env, jobject obj, jint drawPriority)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->drawPriority = drawPriority;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setDrawPriority()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setPointSize
(JNIEnv *env, jobject obj, jfloat pointSize)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->pointSize = pointSize;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setPointSize()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setParticleSystemTypeNative
(JNIEnv *env, jobject obj, jint type)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        switch (type) {
            case 0:
                inst->type = ParticleSystemPoint;
                break;
            case 1:
                inst->type = ParticleSystemRectangle;
                break;
            default:
                inst->type = ParticleSystemPoint;
                break;
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setParticleSystemType()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setShaderID
(JNIEnv *env, jobject obj, jlong shaderID)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->shaderID = shaderID;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setShaderID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setLifetime
(JNIEnv *env, jobject obj, jdouble lifeTime)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->lifetime = lifeTime;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setLifetime()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setBasetime
(JNIEnv *env, jobject obj, jdouble baseTime)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->baseTime = baseTime;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setBasetime()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_ParticleSystem_getBasetime
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0.0;
        
        return inst->baseTime;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getBasetime()");
    }
    
    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addParticleSystemAttributeNative
(JNIEnv *env, jobject obj, jstring name, jint type)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        SingleVertexAttributeInfo *attr = new SingleVertexAttributeInfo();
        JavaString jstr(env, name);
        attr->name = jstr.cStr;
        attr->type = (BDAttributeDataType) type;
        inst->vertAttrs.push_back(*attr);
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::addParticleSystemAttribute()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addTexID
(JNIEnv *env, jobject obj, jlong texID)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->texIDs.push_back(texID);
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::addTexID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setTotalParticles
(JNIEnv *env, jobject obj, jint totalParticles)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
        return;
        
        inst->totalParticles = totalParticles;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setBasetime()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setContinuousRender
(JNIEnv *env, jobject obj, jboolean newVal)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->continuousUpdate = newVal;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setContinuousRender()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setBatchSize
(JNIEnv *env, jobject obj, jint batchSize)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
        return;
        
        inst->batchSize = batchSize;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setBatchSize()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_ParticleSystem_getBatchSize
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0;
        
        return inst->batchSize;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getBatchSize()");
    }
    
    return 0;
}

JNIEXPORT jintArray JNICALL Java_com_mousebird_maply_ParticleSystem_getAttributesTypes
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        jintArray result;
        result = env->NewIntArray(inst->vertAttrs.size());
        jint values[inst->vertAttrs.size()];
        for (int i = 0; i < inst->vertAttrs.size(); i++) {
            values[i] = (int)inst->vertAttrs[i].type;
        }
        env->SetIntArrayRegion(result, 0,inst->vertAttrs.size(), values );
        
        return result;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getAttributesTypes()");
    }
    
    return NULL;
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_ParticleSystem_getAttributesNames
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        jobjectArray ret;
        int i;
        
        ret = (jobjectArray) env->NewObjectArray(inst->vertAttrs.size(), env->FindClass("java/lang/String"), env->NewStringUTF(""));
        for (i = 0; i < inst->vertAttrs.size(); i++){
            
            const char* value=inst->vertAttrs[i].name.c_str();
            env->SetObjectArrayElement(ret, i, env->NewStringUTF(value));
        }
        return ret;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getAttributesNames()");
    }
    
    return NULL;
}
