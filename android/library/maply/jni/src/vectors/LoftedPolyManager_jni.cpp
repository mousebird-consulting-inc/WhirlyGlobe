/*
 *  LoftedPolyManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/4/19.
 *  Copyright 2011-2019 mousebird consulting
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
#import "com_mousebird_maply_LoftedPolyManager.h"

using namespace WhirlyKit;
using namespace Eigen;

typedef JavaClassInfo<WhirlyKit::LoftManager> LoftManagerClassInfo;
template<> LoftManagerClassInfo *LoftManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoftedPolyManager_nativeInit
        (JNIEnv *env, jclass cls)
{
    LoftManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoftedPolyManager_initialise
        (JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        LoftManager *loftManager = dynamic_cast<LoftManager *>(scene->getManager(kWKLoftedPolyManager));
        LoftManagerClassInfo::getClassInfo()->setHandle(env,obj,loftManager);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoftedPolyManager::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoftedPolyManager_dispose
        (JNIEnv *env, jobject obj)
{
    try
    {
        LoftManagerClassInfo *classInfo = LoftManagerClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoftedPolyManager::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_LoftedPolyManager_addPolys
        (JNIEnv *env, jobject obj, jobjectArray vecObjArray, jobject loftInfoObj, jobject changeSetObj)
{
    try
    {
        LoftManager *loftManager = LoftManagerClassInfo::getClassInfo()->getObject(env,obj);
        LoftedPolyInfoRef *loftInfo = LoftedPolyInfoClassInfo::getClassInfo()->getObject(env,loftInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!loftManager || !loftInfo || !changeSet)
            return EmptyIdentity;

        // Collect up all the shapes to add at once
        VectorObjectClassInfo *vecObjClassInfo = VectorObjectClassInfo::getClassInfo();
        ShapeSet shapes;
        JavaObjectArrayHelper vecHelp(env,vecObjArray);
        while (jobject vecObjObj = vecHelp.getNextObject()) {
            VectorObjectRef *vecObj = vecObjClassInfo->getObject(env,vecObjObj);
            if (vecObj)
                shapes.insert((*vecObj)->shapes.begin(),(*vecObj)->shapes.end());
        }

        // Resolve a missing program
        if ((*loftInfo)->programID == EmptyIdentity)
        {
            ProgramGLES *prog = NULL;
            prog = (ProgramGLES *)loftManager->getScene()->findProgramByName(MaplyDefaultTriangleShader);
            if (prog)
                (*loftInfo)->programID = prog->getId();
        }

        SimpleIdentity loftID = loftManager->addLoftedPolys(&shapes,*(*loftInfo),*(changeSet->get()));

        return loftID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoftedPolyManager::addVectors()");
    }

    return EmptyIdentity;
}