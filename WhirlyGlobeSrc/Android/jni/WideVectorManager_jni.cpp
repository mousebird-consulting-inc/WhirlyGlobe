/*
 *  WideVectorManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/8/17.
 *  Copyright 2011-2017 mousebird consulting
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
#import "com_mousebird_maply_WideVectorManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

// Wrapper that tracks the scene as well
class WideVecManagerWrapper
{
public:
    WideVecManagerWrapper(WideVectorManager *vecManager,Scene *scene)
    : vecManager(vecManager), scene(scene)
    {
        
    }
    WideVectorManager *vecManager;
    Scene *scene;
};

typedef JavaClassInfo<WideVecManagerWrapper> WideVectorManagerWrapperClassInfo;
template<> WideVectorManagerWrapperClassInfo *WideVectorManagerWrapperClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_nativeInit
(JNIEnv *env, jclass cls)
{
    WideVectorManagerWrapperClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_initialise
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        WideVectorManager *vecManager = dynamic_cast<WideVectorManager *>(scene->getManager(kWKWideVectorManager));
        WideVecManagerWrapper *wrap = new WideVecManagerWrapper(vecManager,scene);
        WideVectorManagerWrapperClassInfo::getClassInfo()->setHandle(env,obj,wrap);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorManager::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        WideVectorManagerWrapperClassInfo *classInfo = WideVectorManagerWrapperClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            WideVecManagerWrapper *wrap = classInfo->getObject(env,obj);
            if (!wrap)
                return;
            classInfo->clearHandle(env,obj);
            delete wrap;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorManager::dispose()");
    }
}


JNIEXPORT jlong JNICALL Java_com_mousebird_maply_WideVectorManager_addVectors
(JNIEnv *env, jobject obj, jobject vecObjList, jobject vecInfoObj, jobject changeSetObj)
{
    try
    {
        WideVectorManagerWrapperClassInfo *classInfo = WideVectorManagerWrapperClassInfo::getClassInfo();
        WideVecManagerWrapper *wrap = classInfo->getObject(env,obj);
        WideVectorInfo *vecInfo = WideVectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!wrap || !vecInfo || !changeSet)
            return EmptyIdentity;
        
        //        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "WideVectorInfo: (min,max) = (%f, %f), color = (%d,%d,%d,%d)",vecInfo->minVis,vecInfo->maxVis,(int)vecInfo->color.r,(int)vecInfo->color.g,(int)vecInfo->color.b,(int)vecInfo->color.a);
        
        // Get the iterator
        // Note: Look these up once
        jclass listClass = env->GetObjectClass(vecObjList);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(vecObjList,literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass,"hasNext","()Z");
        jmethodID next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(iterClass);
        env->DeleteLocalRef(listClass);
        
        ShapeSet shapes;
        VectorObjectClassInfo *vecObjClassInfo = VectorObjectClassInfo::getClassInfo();
        while (env->CallBooleanMethod(liter, hasNext))
        {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            VectorObject *vecObj = vecObjClassInfo->getObject(env,javaVecObj);
            shapes.insert(vecObj->shapes.begin(),vecObj->shapes.end());
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);
        
        SimpleIdentity vecId = wrap->vecManager->addVectors(&shapes,*vecInfo,*changeSet);
        
        return vecId;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorManager::addVectors()");
    }
    
    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_removeVectors
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
    try
    {
        WideVectorManagerWrapperClassInfo *classInfo = WideVectorManagerWrapperClassInfo::getClassInfo();
        WideVecManagerWrapper *wrap = classInfo->getObject(env,obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!wrap || !changeSet)
            return;
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);
        
        wrap->vecManager->removeVectors(idSet,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorManager::removeVectors()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_enableVectors
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
    try
    {
        WideVectorManagerWrapperClassInfo *classInfo = WideVectorManagerWrapperClassInfo::getClassInfo();
        WideVecManagerWrapper *wrap = classInfo->getObject(env,obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!wrap || !changeSet)
            return;
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);
        
        wrap->vecManager->enableVectors(idSet,enable,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorManager::enableVectors()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_WideVectorManager_instanceVectors
(JNIEnv *env, jobject obj, jlong vecID, jobject vecInfoObj, jobject changeSetObj)
{
    try
    {
        WideVectorManagerWrapperClassInfo *classInfo = WideVectorManagerWrapperClassInfo::getClassInfo();
        WideVecManagerWrapper *wrap = classInfo->getObject(env,obj);
        WideVectorInfo *vecInfo = WideVectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!wrap || !vecInfo || !changeSet)
            return EmptyIdentity;
        
        wrap->vecManager->instanceVectors(vecID,*vecInfo,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorManager::instanceVectors()");
    }
}

