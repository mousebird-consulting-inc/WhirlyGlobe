/*  ParticleSystemManager_jni.cpp
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
#import "Scene_jni.h"
#import "com_mousebird_maply_ParticleSystemManager.h"

using namespace WhirlyKit;
using namespace Maply;

template<> ParticleSystemManagerClassInfo * ParticleSystemManagerClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_nativeInit(JNIEnv *env, jclass cls)
{
    ParticleSystemManagerClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_initialize(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try {
        if (auto scene = SceneClassInfo::get(env, sceneObj)) {
            const auto manager = scene->getManager<ParticleSystemManager>(kWKParticleSystemManager);
            ParticleSystemManagerClassInfo::getClassInfo()->setHandle(env, obj,
                    new ParticleSystemManagerRef(manager));
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in ParticleSystemManager::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_dispose(JNIEnv *env, jobject obj)
{
    try {
        auto manager = ParticleSystemManagerClassInfo::get(env, obj);
        delete manager;
        ParticleSystemManagerClassInfo::getClassInfo()->clearHandle(env, obj);
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in ParticleSystemManager::dispose()");
    }
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ParticleSystemManager_addParticleSystem
    (JNIEnv *env, jobject obj, jobject parSysObj, jobject changesObj)
{
    try {
        if (const auto manager = ParticleSystemManagerClassInfo::get(env, obj)) {
            if (const auto parSys = ParticleSystemClassInfo::get(env, parSysObj)) {
                if (const auto changes = ChangeSetClassInfo::get(env, changesObj)) {
                    return (*manager)->addParticleSystem(*parSys, **changes);
                }
            }
        }
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in ParticleSystemManager::addParticleSystem");
    }
    
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_addParticleBatch
    (JNIEnv *env, jobject obj, jlong id, jobject batchObj, jobject changeObj)
{
    try {
        if (const auto manager = ParticleSystemManagerClassInfo::get(env, obj)) {
            if (const auto batch = ParticleBatchClassInfo::get(env, batchObj)) {
                if (const auto changes = ChangeSetClassInfo::get(env, changeObj)) {
                    (*manager)->addParticleBatch(id, *batch, **changes);
                }
            }
        }
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in ParticleSystemManager::addParticleBatch");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_enableParticleSystem
(JNIEnv *env, jobject obj, jlong id, jboolean enable, jobject changeObj)
{
    try {
        if (const auto manager = ParticleSystemManagerClassInfo::get(env, obj)) {
            if (const auto changes = ChangeSetClassInfo::get(env, changeObj)) {
                (*manager)->enableParticleSystem(id, enable, **changes);
            }
        }
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in ParticleSystemManager::enableParticleSystem");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemManager_removeParticleSystem
    (JNIEnv *env, jobject obj, jlong sysID, jobject changeObj)
{
    try {
        if (const auto manager = ParticleSystemManagerClassInfo::get(env, obj)) {
            if (const auto changes = ChangeSetClassInfo::get(env, changeObj)) {
                (*manager)->removeParticleSystem(sysID, **changes);
            }
        }
    }
    catch(...) {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in ParticleSystemManager::removeParticleSystems");
    }
}

                                                                                              
