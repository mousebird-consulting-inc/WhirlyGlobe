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
#import "Particles_jni.h"
#import "com_mousebird_maply_ParticleSystem.h"

using namespace WhirlyKit;

template<> ParticleSystemClassInfo *ParticleSystemClassInfo::classInfoObj = NULL;

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

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ParticleSystem_getID
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
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getID()");
    }
    
    return EmptyIdentity;
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setPositionShaderID
  (JNIEnv *env, jobject obj, jlong shaderID)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->calcShaderID = shaderID;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setPositionShaderID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setRenderShaderID
  (JNIEnv *env, jobject obj, jlong shaderID)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->renderShaderID = shaderID;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setRenderShaderID()");
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
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setTotalParticles()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_ParticleSystem_getTotalParticles
  (JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0;

        return inst->totalParticles;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getTotalParticles()");
    }

    return 0;
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setContinuousUpdate
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addAttributeNative
(JNIEnv *env, jobject obj, jstring name, jint type)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        SingleVertexAttributeInfo attr;
        JavaString jstr(env, name);
        attr.nameID = StringIndexer::getStringID(jstr.cStr);
        attr.type = (BDAttributeDataType) type;
        inst->vertAttrs.push_back(attr);
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::addParticleSystemAttribute()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addVaryingNative
  (JNIEnv *env, jobject obj, jstring inName, jstring inVaryAttrName, jint type)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        SingleVertexAttributeInfo attr;
        JavaString name(env, inName);
        JavaString varyName(env, inVaryAttrName);
        attr.nameID = StringIndexer::getStringID(name.cStr);
        attr.type = (BDAttributeDataType) type;
        inst->varyingAttrs.push_back(attr);
        inst->varyNames.push_back(StringIndexer::getStringID(varyName.cStr));
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::addVaryingNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addTextureID
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setRenderTargetNative
  (JNIEnv *env, jobject obj, jlong targetID)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->renderTargetID = targetID;
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setRenderTargetNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setZBufferRead
        (JNIEnv *env, jobject obj, jboolean val)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->zBufferRead = val;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setZBufferRead()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setZBufferWrite
        (JNIEnv *env, jobject obj, jboolean val)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->zBufferWrite = val;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setZBufferWrite()");
    }
}
