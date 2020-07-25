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

#import "Vectors_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_WideVectorManager.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace Maply;

typedef JavaClassInfo<WideVectorManager> WideVectorManagerClassInfo;
template<> WideVectorManagerClassInfo *WideVectorManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_WideVectorManager_nativeInit
(JNIEnv *env, jclass cls)
{
    WideVectorManagerClassInfo::getClassInfo(env,cls);
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
        WideVectorManagerClassInfo::getClassInfo()->setHandle(env,obj,vecManager);
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
        WideVectorManagerClassInfo *classInfo = WideVectorManagerClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            WideVectorManager *vecManage = classInfo->getObject(env,obj);
            if (!vecManage)
                return;
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorManager::dispose()");
    }
}


JNIEXPORT jlong JNICALL Java_com_mousebird_maply_WideVectorManager_addVectors
(JNIEnv *env, jobject obj, jobjectArray vecObjArray, jobject vecInfoObj, jobject changeSetObj)
{
    try
    {
        WideVectorManager *vecManager = WideVectorManagerClassInfo::getClassInfo()->getObject(env,obj);
        WideVectorInfoRef *vecInfo = WideVectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!vecManager || !vecInfo || !changeSet)
            return EmptyIdentity;
        
        // Collect up all the shapes to add at once
        VectorObjectClassInfo *vecObjClassInfo = VectorObjectClassInfo::getClassInfo();
		ShapeSet shapes;
		JavaObjectArrayHelper vecHelper(env,vecObjArray);
		while (jobject vecObjObj = vecHelper.getNextObject()) {
            VectorObjectRef *vecObj = vecObjClassInfo->getObject(env,vecObjObj);
            if (vecObj)
                shapes.insert((*vecObj)->shapes.begin(),(*vecObj)->shapes.end());
		}

        // Resolve a missing program
        if ((*vecInfo)->programID == EmptyIdentity)
        {
            ProgramGLES *prog = (ProgramGLES *)vecManager->getScene()->findProgramByName(MaplyDefaultWideVectorShader);
            if (prog)
                (*vecInfo)->programID = prog->getId();
        }
                
        SimpleIdentity vecID = vecManager->addVectors(&shapes,*(*vecInfo),*(changeSet->get()));
        
        return vecID;
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
        WideVectorManager *vecManager = WideVectorManagerClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!vecManager || !changeSet)
            return;
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);
        
        vecManager->removeVectors(idSet,*(changeSet->get()));
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
        WideVectorManager *vecManager = WideVectorManagerClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!vecManager || !changeSet)
            return;
        
        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);
        
        vecManager->enableVectors(idSet,enable,*(changeSet->get()));
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
        WideVectorManager *vecManager = WideVectorManagerClassInfo::getClassInfo()->getObject(env,obj);
        WideVectorInfoRef *vecInfo = WideVectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!vecManager || !vecInfo || !changeSet)
            return EmptyIdentity;
        
        return vecManager->instanceVectors(vecID,*(*vecInfo),*(changeSet->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WideVectorManager::instanceVectors()");
    }

    return EmptyIdentity;
}

