/*
 *  LayoutManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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
#import "com_mousebird_maply_LayoutManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

typedef JavaClassInfo<LayoutManager> LayoutManagerClassInfo;
template<> LayoutManagerClassInfo *LayoutManagerClassInfo::classInfoObj = NULL;

// Wrapper that tracks the generator as well
class LayoutManagerWrapper
{
public:
    LayoutManagerWrapper(LayoutManager *layoutManager,OurClusterGenerator *generator)
        : layoutManager(layoutManager), generator(generator) { }

    ~LayoutManagerWrapper() {
        //SJG: Delete layoutManager? It comes from the scene.
        delete layoutManager;
        delete generator;
    }

    LayoutManager *layoutManager;
    OurClusterGenerator *generator;
};

typedef JavaClassInfo<LayoutManagerWrapper> LayoutManagerWrapperClassInfo;
template<> LayoutManagerWrapperClassInfo *LayoutManagerWrapperClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_nativeInit
  (JNIEnv *env, jclass cls)
{
    LayoutManagerWrapperClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_initialise
  (JNIEnv *env, jobject obj,jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
        LayoutManager *layoutManager = dynamic_cast<LayoutManager *>(scene->getManager(kWKLayoutManager));
        OurClusterGenerator *generator = new OurClusterGenerator();
        generator->env = env;
        layoutManager->addClusterGenerator(generator);
        LayoutManagerWrapper *wrap = new LayoutManagerWrapper(layoutManager, generator);
        LayoutManagerWrapperClassInfo::getClassInfo()->setHandle(env, obj, wrap);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return
        classInfo->clearHandle(env, obj);
        delete wrap;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_setMaxDisplayObjects
  (JNIEnv *env, jobject obj, jint maxObjs)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return
            
        wrap->layoutManager->setMaxDisplayObjects(maxObjs);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::setMaxDisplayObjects()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_updateLayout
  (JNIEnv *env, jobject obj, jobject viewStateObj, jobject changeSetObj)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        ViewState *viewState = ViewStateClassInfo::getClassInfo()->getObject(env,viewStateObj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);

        if (!wrap || !viewState || !changeSet)
            return;

        wrap->layoutManager->updateLayout(viewState,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::updateLayout()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LayoutManager_hasChanges
  (JNIEnv *env, jobject obj)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return false;
        
        return wrap->layoutManager->hasChanges();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::hasChanges()");
    }
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_addClusterGenerator
(JNIEnv *env, jobject obj, jobject clusterObj, jint clusterID)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return;

        wrap->generator->clusterGens.insert(std::pair<int,jobject>(clusterID, clusterObj));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::addClusterGenerator()");
    }

}
