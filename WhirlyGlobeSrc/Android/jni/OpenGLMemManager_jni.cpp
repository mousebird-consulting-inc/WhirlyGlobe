/*
 *  OpenGLMemManager_jni.cpp
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
#import "com_mousebird_maply_OpenGLMemManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_nativeInit
(JNIEnv *env, jclass cls)
{
    OpenGLMemManagerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_initialise
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *manager = new OpenGLMemManager();
        classInfo->setHandle(env, obj, manager);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_dispose
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::dispose()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_OpenGLMemManager_getBufferID
(JNIEnv *env, jobject obj, jint size, jint drawType)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getBufferID(size, drawType);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::getBufferID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_removeBufferID
(JNIEnv *env, jobject obj, jint budID)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->removeBufferID(budID);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::removeBufferID()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_OpenGLMemManager_getTexID
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getTexID;
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::getTexID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_removeTexID
(JNIEnv *env, jobject obj, jint texID)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->removeTexID(texID);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::removeTexID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_clearBufferIDs
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->clearBufferIDs();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::clearBufferIDs()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_clearTextureIDs
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->clearTextureIDs();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::clearTextureIDs()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_dumpStats
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->dumpStats();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::dumpStats()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_lock
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->lock();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::lock()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLMemManager_unlock
(JNIEnv *env, jobject obj)
{
    try {
        OpenGLMemManagerClassInfo *classInfo = OpenGLMemManagerClassInfo::getClassInfo();
        OpenGLMemManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->unlock();
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLMemManager::unlock()");
    }
}
