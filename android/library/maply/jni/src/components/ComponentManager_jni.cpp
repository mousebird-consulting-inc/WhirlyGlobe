/*
 *  ComponentManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/19/19.
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

#import "Components_jni.h"
#import "Scene_jni.h"
#import "com_mousebird_maply_ComponentManager.h"

using namespace WhirlyKit;

template<> ComponentManagerClassInfo *ComponentManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	ComponentManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
		ComponentManager *compManager = dynamic_cast<ComponentManager *>(scene->getManager(kWKComponentManager));
		ComponentManagerClassInfo::getClassInfo()->setHandle(env,obj,compManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentManager::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		ComponentManagerClassInfo *classInfo = ComponentManagerClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentManager::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_addComponentObject
  (JNIEnv *env, jobject obj, jobject compObjObj)
{
    try
    {
        ComponentManager *compManager = ComponentManagerClassInfo::getClassInfo()->getObject(env,obj);
        ComponentObjectRef *compObj = ComponentObjectRefClassInfo::getClassInfo()->getObject(env,compObjObj);
        if (!compManager || !compObj)
            return;

        compManager->addComponentObject(*compObj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentManager::addComponentObject()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ComponentManager_hasComponentObject
  (JNIEnv *env, jobject obj, jlong compObjID)
{
    try
    {
        ComponentManager *compManager = ComponentManagerClassInfo::getClassInfo()->getObject(env,obj);
        if (!compManager)
            return false;

        return compManager->hasComponentObject(compObjID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentManager::hasComponentObject()");
    }

    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_removeComponentObjectsNative
  (JNIEnv *env, jobject obj, jobjectArray compObjArray, jobject changeSetObj)
{
    try
    {
        ComponentManager *compManager = ComponentManagerClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!compManager || !changeSet)
            return;

        // Unwrap the raw geometry objects
        SimpleIDSet compObjIDs;
        JavaObjectArrayHelper compArrayHelp(env,compObjArray);
        ComponentObjectRefClassInfo *compObjClassInfo = ComponentObjectRefClassInfo::getClassInfo();
        while (jobject compObjObj = compArrayHelp.getNextObject()) {
            ComponentObjectRef *compObj = compObjClassInfo->getObject(env,compObjObj);
            if (compObj)
                compObjIDs.insert((*compObj)->getId());
        }

        PlatformInfo_Android platformInfo(env);
        compManager->removeComponentObjects(&platformInfo,compObjIDs,*(changeSet->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentManager::enableComponentObjects()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_enableComponentObjects
  (JNIEnv *env, jobject obj, jobjectArray compObjArray, jboolean enable, jobject changeSetObj)
{
    try
    {
        ComponentManager *compManager = ComponentManagerClassInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!compManager || !changeSet)
            return;

        // Unwrap the raw geometry objects
        SimpleIDSet compObjIDs;
        JavaObjectArrayHelper compArrayHelp(env,compObjArray);
        ComponentObjectRefClassInfo *compObjClassInfo = ComponentObjectRefClassInfo::getClassInfo();
        while (jobject compObjObj = compArrayHelp.getNextObject()) {
            ComponentObjectRef *compObj = compObjClassInfo->getObject(env,compObjObj);
            if (compObj)
                compObjIDs.insert((*compObj)->getId());
        }

        compManager->enableComponentObjects(compObjIDs,enable,*(changeSet->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ComponentManager::enableComponentObjects()");
    }
}
