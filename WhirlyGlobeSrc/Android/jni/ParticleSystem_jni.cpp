/*
 *  ParticleSystem_jni.cpp
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_dispose
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        
        classInfo->clearHandle(env, obj);
    }
	catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setIdent
(JNIEnv *env, jobject obj, jlong ident)
{
    try {
        ParticleSystemClassInfo *classInfo = ParticleSystemClassInfo::getClassInfo();
        ParticleSystem *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setId(ident);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setIdent()");
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_setParticleSystemType
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystem_addParticleSystemAttribute
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
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystem::setBasetime()");
    }
}
