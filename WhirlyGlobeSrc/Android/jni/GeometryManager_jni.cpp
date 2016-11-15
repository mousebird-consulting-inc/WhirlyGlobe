/*
 *  GeometryManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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
#import "com_mousebird_maply_GeometryManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_nativeInit
(JNIEnv *env, jclass cls)
{
    GeometryManagerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_initialise
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        GeometryManager *geomManager = dynamic_cast<GeometryManager *>(scene->getManager(kWKGeometryManager));
        GeometryManagerClassInfo::getClassInfo()->setHandle(env,obj,geomManager);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryManager::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        GeometryManagerClassInfo *classInfo = GeometryManagerClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryManager::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_GeometryManager_addGeometry
(JNIEnv *env, jobject obj, jobjectArray rawGeomArr, jobjectArray modelInstArr, jobject geomInfoObj, jobject changeSetObj)
{
    try
    {
        GeometryManager *geomManager = GeometryManagerClassInfo::getClassInfo()->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        GeometryInfo *geomInfo = GeometryInfoClassInfo::getClassInfo()->getObject(env,geomInfoObj);
        if (!geomManager || !changeSet || !geomInfo)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in GeometryManager::addGeometry()");
            return EmptyIdentity;
        }
        
        // Unwrap the raw geometry objects
        std::vector<GeometryRaw *> geoms;
        int geomCount = env->GetArrayLength(rawGeomArr);
        GeometryRawClassInfo *rawGeomClassInfo = GeometryRawClassInfo::getClassInfo();
        for (unsigned int ii=0;ii<geomCount;ii++)
        {
            jobject geomRawObj = env->GetObjectArrayElement(rawGeomArr,ii);
            GeometryRaw *geomRaw = rawGeomClassInfo->getObject(env,geomRawObj);
            if (geomRaw)
                geoms.push_back(geomRaw);
            env->DeleteLocalRef(geomRawObj);
        }
        
        // Unwrap the instances
        std::vector<GeometryInstance *> geomInsts;
        int geomInstCount = env->GetArrayLength(modelInstArr);
        GeometryInstanceClassInfo *geomInfoClassInfo = GeometryInstanceClassInfo::getClassInfo();
        for (unsigned int ii=0;ii<geomInstCount;ii++)
        {
            jobject geomInstObj = env->GetObjectArrayElement(modelInstArr,ii);
            GeometryInstance *geomInst = geomInfoClassInfo->getObject(env,geomInstObj);
            if (geomInst)
                geomInsts.push_back(geomInst);
            env->DeleteLocalRef(geomInstObj);
        }
        
        return geomManager->addGeometry(geoms,geomInsts,*geomInfo,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryManager::addGeometry()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_GeometryManager_addBaseGeometry
(JNIEnv *env, jobject obj, jobjectArray rawGeomArr, jobject changeSetObj)
{
    try
    {
        GeometryManager *geomManager = GeometryManagerClassInfo::getClassInfo()->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!geomManager || !changeSet)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in GeometryManager::addBaseGeometry()");
            return EmptyIdentity;
        }
        
        // Unwrap the raw geometry objects
        std::vector<GeometryRaw *> geoms;
        int geomCount = env->GetArrayLength(rawGeomArr);
        GeometryRawClassInfo *rawGeomClassInfo = GeometryRawClassInfo::getClassInfo();
        for (unsigned int ii=0;ii<geomCount;ii++)
        {
            jobject geomRawObj = env->GetObjectArrayElement(rawGeomArr,ii);
            GeometryRaw *geomRaw = rawGeomClassInfo->getObject(env,geomRawObj);
            if (geomRaw)
                geoms.push_back(geomRaw);
            env->DeleteLocalRef(geomRawObj);
        }
        
        return geomManager->addBaseGeometry(geoms,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryManager::addBaseGeometry()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_GeometryManager_addGeometryInstances
(JNIEnv *env, jobject obj, jlong baseGeomID, jobjectArray modelInstArr, jobject geomInfoObj, jobject changeSetObj)
{
    try
    {
        GeometryManager *geomManager = GeometryManagerClassInfo::getClassInfo()->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        GeometryInfo *geomInfo = GeometryInfoClassInfo::getClassInfo()->getObject(env,geomInfoObj);
        if (!geomManager || !changeSet || !geomInfo)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in GeometryManager::addGeometryInstances()");
            return EmptyIdentity;
        }
        
        // Unwrap the instances
        std::vector<GeometryInstance *> geomInsts;
        int geomInstCount = env->GetArrayLength(modelInstArr);
        GeometryInstanceClassInfo *geomInfoClassInfo = GeometryInstanceClassInfo::getClassInfo();
        for (unsigned int ii=0;ii<geomInstCount;ii++)
        {
            jobject geomInstObj = env->GetObjectArrayElement(modelInstArr,ii);
            GeometryInstance *geomInst = geomInfoClassInfo->getObject(env,geomInstObj);
            if (geomInst)
                geomInsts.push_back(geomInst);
            env->DeleteLocalRef(geomInstObj);
        }
        
        return geomManager->addGeometryInstances(baseGeomID,geomInsts,*geomInfo,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryManager::addGeometryInstances()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_GeometryManager_addGeometryPoints
(JNIEnv *env, jobject obj, jobject pointsObj, jobject matObj, jobject geomInfoObj, jobject changeSetObj)
{
    try
    {
        GeometryManagerClassInfo *classInfo = GeometryManagerClassInfo::getClassInfo();
        GeometryManager *geomManager = classInfo->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        GeometryRawPoints *rawPoints = GeometryRawPointsClassInfo::getClassInfo()->getObject(env,pointsObj);
        Matrix4d *mat = Matrix4dClassInfo::getClassInfo()->getObject(env,matObj);
        GeometryInfo *geomInfo = GeometryInfoClassInfo::getClassInfo()->getObject(env,geomInfoObj);
        
        if (!geomManager || !rawPoints || !mat || !changeSet)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in GeometryManager::addGeometry()");
            return EmptyIdentity;
        }

        return geomManager->addGeometryPoints(*rawPoints,*mat,*geomInfo,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryManager::addGeometryPoints()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_enableGeometry
(JNIEnv *env, jobject obj, jlongArray geomIDs, jboolean enable, jobject changeSetObj)
{
    try
    {
        GeometryManagerClassInfo *classInfo = GeometryManagerClassInfo::getClassInfo();
        GeometryManager *geomManager = classInfo->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,geomIDs,idSet);

        geomManager->enableGeometry(idSet,enable,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryManager::enableGeometry()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_removeGeometry
(JNIEnv *env, jobject obj, jlongArray geomIDs, jobject changeSetObj)
{
    try
    {
        GeometryManagerClassInfo *classInfo = GeometryManagerClassInfo::getClassInfo();
        GeometryManager *geomManager = classInfo->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,geomIDs,idSet);
        
        geomManager->removeGeometry(idSet,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryManager::removeGeometry()");
    }
}
