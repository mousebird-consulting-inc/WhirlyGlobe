/*  WideVectorManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/8/17.
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
 *
 */

#import "Vectors_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_WideVectorManager.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace Maply;

typedef JavaClassInfo<WideVectorManagerRef> WideVectorManagerClassInfo;
template<> WideVectorManagerClassInfo *WideVectorManagerClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_nativeInit
    (JNIEnv *env, jclass cls)
{
    WideVectorManagerClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_initialise
    (JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        auto vecManager = scene->getManager<WideVectorManager>(kWKWideVectorManager);
        WideVectorManagerClassInfo::getClassInfo()->setHandle(env,obj,new WideVectorManagerRef(vecManager));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in WideVectorManager::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_dispose
    (JNIEnv *env, jobject obj)
{
    try
    {
        WideVectorManagerClassInfo *classInfo = WideVectorManagerClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        WideVectorManagerRef *vecManage = classInfo->getObject(env,obj);
        delete vecManage;
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in WideVectorManager::dispose()");
    }
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_WideVectorManager_addVectors
    (JNIEnv *env, jobject obj, jobjectArray vecObjArray,
     jobject vecInfoObj, jobject changeSetObj)
{
    try
    {
        const auto vecManager = WideVectorManagerClassInfo::get(env,obj);
        const WideVectorInfoRef *vecInfo = WideVectorInfoClassInfo::get(env,vecInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::get(env,changeSetObj);
        if (!vecManager || !vecInfo || !changeSet)
        {
            return EmptyIdentity;
        }
        
        // Collect up all the shapes to add at once
        const auto vecObjClassInfo = VectorObjectClassInfo::getClassInfo();
        std::vector<VectorShapeRef> shapes;
        JavaObjectArrayHelper vecHelper(env,vecObjArray);
        shapes.reserve(vecHelper.numObjects());
		while (const jobject vecObjObj = vecHelper.getNextObject())
		{
            if (const VectorObjectRef *vecObj = vecObjClassInfo->getObject(env,vecObjObj))
            {
                shapes.insert(shapes.end(),(*vecObj)->shapes.begin(),(*vecObj)->shapes.end());
            }
		}

        // Resolve a missing program
        if ((*vecInfo)->programID == EmptyIdentity)
        {
            const auto scene = (*vecManager)->getScene();
            if (const auto prog = (ProgramGLES*)scene->findProgramByName(MaplyDefaultWideVectorShader))
            {
                (*vecInfo)->programID = prog->getId();
            }
        }

        return (*vecManager)->addVectors(shapes,**vecInfo,**changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in WideVectorManager::addVectors()");
    }
    
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_removeVectors
    (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
    try
    {
        WideVectorManagerRef *vecManager = WideVectorManagerClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!vecManager || !changeSet)
            return;
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);
        
        (*vecManager)->removeVectors(idSet,*(changeSet->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in WideVectorManager::removeVectors()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_enableVectors
    (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
    try
    {
        WideVectorManagerRef *vecManager = WideVectorManagerClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!vecManager || !changeSet)
            return;
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);
        
        (*vecManager)->enableVectors(idSet,enable,*(changeSet->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in WideVectorManager::enableVectors()");
    }
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_WideVectorManager_instanceVectors
    (JNIEnv *env, jobject obj, jlong vecID, jobject vecInfoObj, jobject changeSetObj)
{
    try
    {
        WideVectorManagerRef *vecManager = WideVectorManagerClassInfo::getClassInfo()->getObject(env,obj);
        WideVectorInfoRef *vecInfo = WideVectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!vecManager || !vecInfo || !changeSet)
            return EmptyIdentity;
        
        return (*vecManager)->instanceVectors(vecID,*(*vecInfo),*(changeSet->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in WideVectorManager::instanceVectors()");
    }

    return EmptyIdentity;
}

