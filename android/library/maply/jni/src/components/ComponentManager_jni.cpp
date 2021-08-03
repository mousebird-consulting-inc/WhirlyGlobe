/*  ComponentManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/19/19.
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

#import <classInfo/Geometry_jni.h>
#import <classInfo/Maply_jni.h>
#import <classInfo/View_jni.h>
#import <classInfo/Selection_jni.h>
#import <classInfo/Components_jni.h>
#import <classInfo/Scene_jni.h>
#import <com_mousebird_maply_ComponentManager.h>

using namespace WhirlyKit;

template<> ComponentManagerClassInfo *ComponentManagerClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	ComponentManagerClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
        if (Scene *scene = SceneClassInfo::get(env, sceneObj))
        {
            const auto compManager = scene->getManager<ComponentManager_Android>(kWKComponentManager);
            compManager->setupJNI(env, obj);

            const auto classInfo = ComponentManagerClassInfo::getClassInfo();
            classInfo->setHandle(env, obj,new ComponentManager_AndroidRef(compManager));
        }
	}
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		const auto classInfo = ComponentManagerClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        if (const auto compManager = classInfo->getObject(env,obj))
        {
            (*compManager)->clearJNI(env);
            delete compManager;
        }
        classInfo->clearHandle(env,obj);
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_addComponentObject
  (JNIEnv *env, jobject obj, jobject compObjObj, jobject changeSetObj)
{
    try
    {
        if (const auto compManager = ComponentManagerClassInfo::get(env,obj))
        if (const auto compObj = ComponentObjectRefClassInfo::get(env,compObjObj))
        if (const auto changeSet = ChangeSetClassInfo::get(env,changeSetObj))
        {
            (*compManager)->addComponentObject(*compObj, **changeSet);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ComponentManager_hasComponentObject
  (JNIEnv *env, jobject obj, jlong compObjID)
{
    try
    {
        if (const auto compManager = ComponentManagerClassInfo::get(env,obj))
        {
            return (*compManager)->hasComponentObject(compObjID);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_removeComponentObjectsNative
  (JNIEnv *env, jobject obj, jobjectArray compObjArray,
   jobject changeSetObj, jboolean disposeAfterRemoval)
{
    try
    {
        const auto compManager = ComponentManagerClassInfo::get(env,obj);
        const auto changeSet = ChangeSetClassInfo::get(env,changeSetObj);
        if (!compManager || !changeSet)
            return;

        // Unwrap the raw geometry objects
        SimpleIDSet compObjIDs;
        JavaObjectArrayHelper compArrayHelp(env,compObjArray);

        const auto compObjClassInfo = ComponentObjectRefClassInfo::getClassInfo();
        while (jobject compObjObj = compArrayHelp.getNextObject())
        {
            if (const auto compObj = compObjClassInfo->getObject(env,compObjObj))
            {
                compObjIDs.insert((*compObj)->getId());
            }
        }

        PlatformInfo_Android platformInfo(env);
        (*compManager)->removeComponentObjects(&platformInfo,compObjIDs,**changeSet, disposeAfterRemoval);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ComponentManager_enableComponentObjects
  (JNIEnv *env, jobject obj, jobjectArray compObjArray, jboolean enable, jobject changeSetObj)
{
    try
    {
        const auto compManager = ComponentManagerClassInfo::get(env,obj);
        const auto changeSet = ChangeSetClassInfo::get(env,changeSetObj);
        if (!compManager || !changeSet)
            return;

        // Unwrap the raw geometry objects
        SimpleIDSet compObjIDs;
        JavaObjectArrayHelper compArrayHelp(env,compObjArray);

        const auto compObjClassInfo = ComponentObjectRefClassInfo::getClassInfo();
        while (jobject compObjObj = compArrayHelp.getNextObject())
        {
            if (const auto compObj = compObjClassInfo->getObject(env,compObjObj))
            {
                compObjIDs.insert((*compObj)->getId());
            }
        }

        (*compManager)->enableComponentObjects(compObjIDs,enable,**changeSet);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_ComponentManager_findVectors
  (JNIEnv *env, jobject obj, jobject geoPtObj, jdouble maxDist,
   jobject viewStateObj, jobject frameSizeObj, jint limit)
{
    try
    {
        const auto compManager = ComponentManagerClassInfo::get(env,obj);

        const auto ptClassInfo = Point2dClassInfo::getClassInfo();
        const auto geoPt = ptClassInfo->getObject(env, geoPtObj);
        const auto frameSize = ptClassInfo->getObject(env, frameSizeObj);

        const auto viewState = ViewStateRefClassInfo::get(env, viewStateObj);

        if (!compManager || !*compManager || !geoPt || !frameSize || !viewState)
        {
            return nullptr;
        }

        const auto frameSize2f = frameSize->cast<float>();
        const std::vector<std::pair<ComponentObjectRef,VectorObjectRef>> results =
            (*compManager)->findVectors(*geoPt, maxDist, *viewState, frameSize2f, limit);

        std::vector<jobject> selObjs;
        selObjs.reserve(results.size());
        for (const auto &item : results)
        {
            //const auto compObj = item.first;
            const auto vectorObj = item.second;

            SelectionManager::SelectedObject selObj;
            selObj.selectIDs.push_back(vectorObj->getId());
            selObj.screenDist = 0.0;    // todo
            selObj.distIn3D = 0.0;      // todo

            if (const auto selJObj = MakeSelectedObject(env, std::move(selObj)))
            {
                selObjs.push_back(selJObj);
            }
        }

        return BuildObjectArray(env,SelectedObjectClassInfo::getClassInfo()->getClass(),selObjs);
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

