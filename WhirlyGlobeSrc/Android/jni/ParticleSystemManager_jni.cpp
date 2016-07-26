/*
 *  ParticleSystemManager_jni.cpp
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
#import "com_mousebird_maply_ParticleSystemManager.h"
#import "WhirlyGlobe.h"


using namespace WhirlyKit;
using namespace Maply;


JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_nativeInit
(JNIEnv *env, jclass cls)
{
    ParticleSystemManagerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_initialize
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    
    try {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        ParticleSystemManager *particleSystemManager = dynamic_cast<ParticleSystemManager *>(scene->getManager(kWKParticleSystemManager));
        ParticleSystemManagerClassInfo::getClassInfo()->setHandle(env, obj, particleSystemManager);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemManager::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_dispose
(JNIEnv *env, jobject obj)
{
    try {
        ParticleSystemManagerClassInfo *classInfo = ParticleSystemManagerClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemManager::dispose()");
    }
}
                                                                                              
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ParticleSystemManager_addParticleSystem
(JNIEnv *env, jobject obj, jobject parSysObj, jobject changesObj)
{
    try {
        ParticleSystemManagerClassInfo *classInfo = ParticleSystemManagerClassInfo::getClassInfo();
        ParticleSystemManager *particleSystemManager = classInfo->getObject(env, obj);
        
        ParticleSystem *parSys = ParticleSystemClassInfo::getClassInfo()->getObject(env, parSysObj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env, changesObj);
        
        if (!particleSystemManager || !parSys || !changes)
            return EmptyIdentity;
        
        if (parSys->shaderID == EmptyIdentity)
            parSys->shaderID = particleSystemManager->getScene()->getProgramIDBySceneName(kToolkitDefaultParticleSystemProgram);
        
        return particleSystemManager->addParticleSystem(*parSys, *changes);
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemManager::addParticleSystem");
    }
}
                                                                                              
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_addParticleBatch
(JNIEnv *env, jobject obj, jlong id, jobject batchObj, jobject changeObj)
{
    try {
        ParticleSystemManagerClassInfo *classInfo = ParticleSystemManagerClassInfo::getClassInfo();
        ParticleSystemManager *particleSystemManager = classInfo->getObject(env, obj);
        
        ParticleBatch *batch = ParticleBatchClassInfo::getClassInfo()->getObject(env, batchObj);
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        
        if (!particleSystemManager || !batch || !changes)
            return;
        
        particleSystemManager->addParticleBatch(id, *batch, *changes);
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemManager::addParticleBatch");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_enableParticleSystem
(JNIEnv *env, jobject obj, jlong id, jboolean enable, jobject changeObj)
{
    try {
        ParticleSystemManagerClassInfo *classInfo = ParticleSystemManagerClassInfo::getClassInfo();
        ParticleSystemManager *particleSystemManager = classInfo->getObject(env, obj);
        
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!particleSystemManager || !changes)
            return;
        
        particleSystemManager->enableParticleSystem(id, enable, *changes);
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemManager::enableParticleSystem");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_removeParticleSystem
(JNIEnv *env, jobject obj, jlong sysID, jobject changeObj)
{
    try {
        ParticleSystemManagerClassInfo *classInfo = ParticleSystemManagerClassInfo::getClassInfo();
        ParticleSystemManager *particleSystemManager = classInfo->getObject(env, obj);
        
        ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!particleSystemManager || !changes)
            return;
        particleSystemManager->removeParticleSystem(sysID, *changes);
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemManager::removeParticleSystems");
    }
}

                                                                                              
