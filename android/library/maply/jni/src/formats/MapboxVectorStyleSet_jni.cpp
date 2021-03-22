/*  MapboxVectorStyleSet_jni.cpp
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

#import <Formats_jni.h>
#import <Scene_jni.h>
#import <CoordSystem_jni.h>
#import <Vectors_jni.h>
#import "com_mousebird_maply_MapboxVectorStyleSet.h"

using namespace WhirlyKit;

template<> MapboxVectorStyleSetClassInfo *MapboxVectorStyleSetClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_nativeInit(JNIEnv *env, jclass cls)
{
    MapboxVectorStyleSetClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_initialise
(JNIEnv *env, jobject obj, jobject sceneObj, jobject coordSysObj, jobject settingsObj, jobject attrObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
        CoordSystemRef *coordSystem = CoordSystemRefClassInfo::getClassInfo()->getObject(env,coordSysObj);
        MutableDictionary_AndroidRef *attrDict = AttrDictClassInfo::getClassInfo()->getObject(env,attrObj);
        if (!scene || !coordSystem || !attrDict)
            return;

        // Use settings or provide a default
        VectorStyleSettingsImplRef settings;
        if (settingsObj) {
            settings = *(VectorStyleSettingsClassInfo::getClassInfo()->getObject(env,settingsObj));
        } else {
            settings = std::make_shared<VectorStyleSettingsImpl>(1.0);
        }

        auto inst = new MapboxVectorStyleSetImpl_AndroidRef(new MapboxVectorStyleSetImpl_Android(scene,(*coordSystem).get(),settings));

        // Need a pointer to this JNIEnv for low level parsing callbacks
        PlatformInfo_Android threadInst(env);

        (*inst)->thisObj = env->NewGlobalRef(obj);
        MapboxVectorStyleSetClassInfo::getClassInfo()->setHandle(env,obj,inst);

        const bool success = (*inst)->parse(&threadInst,*attrDict);
        if (!success)
        {
            __android_log_print(ANDROID_LOG_WARN, "Maply", "Failed to parse attrs in MapboxVectorStyleSet::initialise()");
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        MapboxVectorStyleSetClassInfo *classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            MapboxVectorStyleSetImpl_AndroidRef *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            (*inst)->cleanup(env);
            env->DeleteGlobalRef((*inst)->thisObj);
            (*inst)->thisObj = nullptr;
            delete inst;
        }

        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorStyleSet::dispose()");
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_backgroundColorForZoomNative
        (JNIEnv *env, jobject obj, jdouble zoom)
{
    try
    {
        MapboxVectorStyleSetClassInfo *classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        MapboxVectorStyleSetImpl_AndroidRef *inst = classInfo->getObject(env,obj);
        if (!inst)
            return 0;

        PlatformInfo_Android platformInfo(env);

        RGBAColorRef backColor = (*inst)->backgroundColor(&platformInfo,zoom);
        if (!backColor)
            return 0;

        return backColor->asInt();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorStyleSet::backgroundColorForZoomNative()");
    }

    return 0;
}

/*
 * Class:     com_mousebird_maply_MapboxVectorStyleSet
 * Method:    getZoomSlot
 * Signature: ()I
 */
extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_getZoomSlot(JNIEnv *env, jobject obj)
{
    try
    {
        auto classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        auto instPtr = classInfo->getObject(env,obj);
        if (auto inst = instPtr ? *instPtr : nullptr)
        {
            return inst->getZoomSlot();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::getZoomSlot()");
    }
    return -1;
}

/*
 * Class:     com_mousebird_maply_MapboxVectorStyleSet
 * Method:    setZoomSlot
 * Signature: (I)V
 */
extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_setZoomSlot(JNIEnv *env, jobject obj, jint slot)
{
    try
    {
        auto classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        auto instPtr = classInfo->getObject(env,obj);
        if (auto inst = instPtr ? *instPtr : nullptr)
        {
            inst->setZoomSlot(slot);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in MapboxVectorStyleSet::setZoomSlot()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorStyleSet_setArealShaderNative
        (JNIEnv *env, jobject obj, jlong shaderID)
{
    try
    {
        MapboxVectorStyleSetClassInfo *classInfo = MapboxVectorStyleSetClassInfo::getClassInfo();
        MapboxVectorStyleSetImpl_AndroidRef *inst = classInfo->getObject(env,obj);
        if (!inst || shaderID == EmptyIdentity)
            return;

        (*inst)->vectorArealProgramID = shaderID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorStyleSet::setArealShaderNative()");
    }
}