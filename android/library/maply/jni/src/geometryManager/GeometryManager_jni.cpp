/*  GeometryManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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
#import "GeometryManager_jni.h"
#import "Geometry_jni.h"
#import "Scene_jni.h"
#import "com_mousebird_maply_GeometryManager.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace Maply;

template<> GeometryManagerClassInfo *GeometryManagerClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_nativeInit
  (JNIEnv *env, jclass cls)
{
    GeometryManagerClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        if (Scene *scene = SceneClassInfo::get(env, sceneObj))
        {
            auto geomManager = scene->getManager<GeometryManager>(kWKGeometryManager);
            GeometryManagerClassInfo::getClassInfo()->setHandle(env, obj, new GeometryManagerRef(geomManager));
        }
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        GeometryManagerClassInfo *classInfo = GeometryManagerClassInfo::getClassInfo();
        std::lock_guard<std::mutex> guard(disposeMutex);
        if (GeometryManagerRef *geomManager = classInfo->getObject(env, obj))
        {
            delete geomManager;
            classInfo->clearHandle(env, obj);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_GeometryManager_addGeometry
  (JNIEnv *env, jobject obj, jobjectArray rawGeomArr,
   jobjectArray modelInstArr, jobject geomInfoObj, jobject changeSetObj)
{
    try
    {
        GeometryManagerRef *geomManager = GeometryManagerClassInfo::getClassInfo()->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        GeometryInfoRef *geomInfo = GeometryInfoClassInfo::getClassInfo()->getObject(env,geomInfoObj);
        if (!geomManager || !changeSet || !geomInfo)
        {
            __android_log_print(ANDROID_LOG_WARN, "Maply", "One of the inputs was null in GeometryManager::addGeometry()");
            return EmptyIdentity;
        }
        
        // Unwrap the raw geometry objects
        std::vector<GeometryRaw *> geoms;
        GeometryRawClassInfo *rawGeomClassInfo = GeometryRawClassInfo::getClassInfo();
        JavaObjectArrayHelper rawGeomArrHelp(env,rawGeomArr);
        while (jobject geomRawObj = rawGeomArrHelp.getNextObject()) {
            GeometryRaw *geomRaw = rawGeomClassInfo->getObject(env,geomRawObj);
            if (geomRaw)
                geoms.push_back(geomRaw);
        }

        // Unwrap the instances
        std::vector<GeometryInstance *> geomInsts;
        GeometryInstanceClassInfo *geomInstClassInfo = GeometryInstanceClassInfo::getClassInfo();
        JavaObjectArrayHelper geomInstArrHelp(env,modelInstArr);
        while (jobject geomInstObj = geomInstArrHelp.getNextObject()) {
            GeometryInstance *geomInst = geomInstClassInfo->getObject(env,geomInstObj);
            if (geomInst)
                geomInsts.push_back(geomInst);
        }

        // Resolve a missing program
        if ((*geomInfo)->programID == EmptyIdentity)
        {
            Program *prog = (*geomManager)->getScene()->findProgramByName(MaplyDefaultTriangleShader);
            if (prog)
                (*geomInfo)->programID = prog->getId();
        }


        return (*geomManager)->addGeometry(geoms,geomInsts,*(*geomInfo),*(changeSet->get()));
    }
    MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_GeometryManager_addBaseGeometry
  (JNIEnv *env, jobject obj, jobjectArray rawGeomArr, jobject changeSetObj)
{
    try
    {
        GeometryManagerRef *geomManager = GeometryManagerClassInfo::getClassInfo()->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!geomManager || !changeSet)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in GeometryManager::addBaseGeometry()");
            return EmptyIdentity;
        }
        
        // Unwrap the raw geometry objects
        std::vector<GeometryRaw *> geoms;
        GeometryRawClassInfo *rawGeomClassInfo = GeometryRawClassInfo::getClassInfo();
        JavaObjectArrayHelper rawGeomArrHelp(env,rawGeomArr);
        while (jobject geomRawObj = rawGeomArrHelp.getNextObject()) {
            GeometryRaw *geomRaw = rawGeomClassInfo->getObject(env,geomRawObj);
            if (geomRaw)
                geoms.push_back(geomRaw);
        }

        // TODO: Pass in the geomInfo
        GeometryInfo geomInfo;
        return (*geomManager)->addBaseGeometry(geoms,geomInfo,*(changeSet->get()));
    }
    MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_GeometryManager_addGeometryInstances
  (JNIEnv *env, jobject obj, jlong baseGeomID, jobjectArray modelInstArr, jobject geomInfoObj, jobject changeSetObj)
{
    try
    {
        GeometryManagerRef *geomManager = GeometryManagerClassInfo::getClassInfo()->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        GeometryInfoRef *geomInfo = GeometryInfoClassInfo::getClassInfo()->getObject(env,geomInfoObj);
        if (!geomManager || !changeSet || !geomInfo)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in GeometryManager::addGeometryInstances()");
            return EmptyIdentity;
        }
        
        // Unwrap the instances
        std::vector<GeometryInstance> geomInsts;
        GeometryInstanceClassInfo *geomInfoClassInfo = GeometryInstanceClassInfo::getClassInfo();
        JavaObjectArrayHelper geomInstArrHelp(env,modelInstArr);
        while (jobject geomInstObj = geomInstArrHelp.getNextObject()) {
            GeometryInstance *geomInst = geomInfoClassInfo->getObject(env,geomInstObj);
            if (geomInst)
                geomInsts.push_back(*geomInst);
        }

        return (*geomManager)->addGeometryInstances(baseGeomID,geomInsts,*(*geomInfo),*(changeSet->get()));
    }
    MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_GeometryManager_addGeometryPoints
  (JNIEnv *env, jobject obj, jobject pointsObj, jobject matObj, jobject geomInfoObj, jobject changeSetObj)
{
    try
    {
        GeometryManagerClassInfo *classInfo = GeometryManagerClassInfo::getClassInfo();
        GeometryManagerRef *geomManager = classInfo->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        GeometryRawPoints *rawPoints = GeometryRawPointsClassInfo::getClassInfo()->getObject(env,pointsObj);
        Matrix4d *mat = Matrix4dClassInfo::getClassInfo()->getObject(env,matObj);
        GeometryInfoRef *geomInfo = GeometryInfoClassInfo::getClassInfo()->getObject(env,geomInfoObj);
        
        if (!geomManager || !rawPoints || !mat || !changeSet)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in GeometryManager::addGeometry()");
            return EmptyIdentity;
        }

        return (*geomManager)->addGeometryPoints(*rawPoints,*mat,*(*geomInfo),*(changeSet->get()));
    }
    MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_enableGeometry
  (JNIEnv *env, jobject obj, jlongArray geomIDs, jboolean enable, jobject changeSetObj)
{
    try
    {
        GeometryManagerClassInfo *classInfo = GeometryManagerClassInfo::getClassInfo();
        GeometryManagerRef *geomManager = classInfo->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!geomManager || !changeSet)
            return;
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,geomIDs,idSet);

        (*geomManager)->enableGeometry(idSet,enable,*(changeSet->get()));
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryManager_removeGeometry
  (JNIEnv *env, jobject obj, jlongArray geomIDs, jobject changeSetObj)
{
    try
    {
        GeometryManagerClassInfo *classInfo = GeometryManagerClassInfo::getClassInfo();
        GeometryManagerRef *geomManager = classInfo->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!geomManager || !changeSet)
            return;

        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,geomIDs,idSet);
        
        (*geomManager)->removeGeometry(idSet,*(changeSet->get()));
    }
    MAPLY_STD_JNI_CATCH()
}
