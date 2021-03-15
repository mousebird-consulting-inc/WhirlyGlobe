/*  ParticleSystem_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 23/1/16.
 *  Copyright 2011-2021 mousebird consulting
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
#import "Particles_jni.h"
#import "com_mousebird_maply_ParticleSystem.h"

using namespace WhirlyKit;

template<> ParticleSystemClassInfo *ParticleSystemClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_nativeInit(JNIEnv *env, jclass cls)
{
    ParticleSystemClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_initialise(JNIEnv *env, jobject obj)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        classInfo->setHandle(env, obj, new ParticleSystem());
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_dispose(JNIEnv *env, jobject obj)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        if (auto inst = classInfo->getObject(env, obj)) {
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::dispose()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setName(JNIEnv *env, jobject obj, jstring name)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            const JavaString jstr(env, name);
            inst->name = jstr.getCString();
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setName()");
    }
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ParticleSystem_getID(JNIEnv *env, jobject obj)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        const auto inst = classInfo->getObject(env, obj);
        return inst ? inst->getId() : EmptyIdentity;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getID()");
    }
    
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setParticleSystemTypeNative(JNIEnv *env, jobject obj, jint type)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            switch (type) {
                default:
                case 0:  inst->type = ParticleSystemPoint; break;
                case 1:  inst->type = ParticleSystemRectangle; break;
            }
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setParticleSystemType()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setPositionShaderID(JNIEnv *env, jobject obj, jlong shaderID)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->calcShaderID = shaderID;
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setPositionShaderID()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setRenderShaderID(JNIEnv *env, jobject obj, jlong shaderID)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->renderShaderID = shaderID;
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setRenderShaderID()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setLifetime(JNIEnv *env, jobject obj, jdouble lifeTime)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->lifetime = lifeTime;
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setLifetime()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setBasetime(JNIEnv *env, jobject obj, jdouble baseTime)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->baseTime = baseTime;
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setBasetime()");
    }
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_ParticleSystem_getBasetime(JNIEnv *env, jobject obj)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        const auto inst = classInfo->getObject(env, obj);
        return inst ? inst->baseTime : 0.0;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getBasetime()");
    }
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setTotalParticles(JNIEnv *env, jobject obj, jint totalParticles)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->totalParticles = totalParticles;
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setTotalParticles()");
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_ParticleSystem_getTotalParticles(JNIEnv *env, jobject obj)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        const auto inst = classInfo->getObject(env, obj);
        return inst ? inst->totalParticles : 0;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getTotalParticles()");
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setBatchSize(JNIEnv *env, jobject obj, jint batchSize)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->batchSize = batchSize;
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setBatchSize()");
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_ParticleSystem_getBatchSize(JNIEnv *env, jobject obj)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        const auto inst = classInfo->getObject(env, obj);
        return inst ? inst->batchSize : 0;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::getBatchSize()");
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setContinuousUpdate(JNIEnv *env, jobject obj, jboolean newVal)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->continuousUpdate = newVal;
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setContinuousRender()");
    }
}



extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setDrawPriority(JNIEnv * env, jobject obj, jint drawPriority)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->drawPriority = drawPriority;
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setDrawPriority()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setPointSize(JNIEnv *env, jobject obj, jfloat pointSize)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->pointSize = pointSize;
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setPointSize()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addAttributeNative(JNIEnv *env, jobject obj, jstring name, jint type)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            SingleVertexAttributeInfo attr;
            const JavaString jstr(env, name);
            attr.nameID = StringIndexer::getStringID(jstr.getCString());
            attr.type = (BDAttributeDataType) type;
            inst->vertAttrs.push_back(attr);
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::addParticleSystemAttribute()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addVaryingNative
  (JNIEnv *env, jobject obj, jstring inName, jstring inVaryAttrName, jint type)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            SingleVertexAttributeInfo attr;
            const JavaString name(env, inName);
            const JavaString varyName(env, inVaryAttrName);
            attr.nameID = StringIndexer::getStringID(name.getCString());
            attr.type = (BDAttributeDataType) type;
            inst->varyingAttrs.push_back(attr);
            inst->varyNames.push_back(StringIndexer::getStringID(varyName.getCString()));
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::addVaryingNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addTextureID(JNIEnv *env, jobject obj, jlong texID)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->texIDs.push_back(texID);
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::addTexID()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setRenderTargetNative(JNIEnv *env, jobject obj, jlong targetID)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->renderTargetID = targetID;
        }
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setRenderTargetNative()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setZBufferRead(JNIEnv *env, jobject obj, jboolean val)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->zBufferRead = val;
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setZBufferRead()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setZBufferWrite(JNIEnv *env, jobject obj, jboolean val)
{
    try {
        const auto classInfo = ParticleSystemClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env, obj)) {
            inst->zBufferWrite = val;
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setZBufferWrite()");
    }
}
