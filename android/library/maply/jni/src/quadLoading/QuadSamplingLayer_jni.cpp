/*
 *  QuadSamplingLayer_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/28/19.
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

#import "QuadLoading_jni.h"
#import "View_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_QuadSamplingLayer.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> QuadSamplingControllerInfo *QuadSamplingControllerInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadSamplingLayer_nativeInit
        (JNIEnv *env, jclass cls)
{
    QuadSamplingControllerInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadSamplingLayer_initialise
        (JNIEnv *env, jobject obj, jobject)
{
    try
    {
        QuadSamplingController_Android *control = new QuadSamplingController_Android();
        QuadSamplingControllerInfo::getClassInfo()->setHandle(env,obj,control);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageTile::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadSamplingLayer_dispose
        (JNIEnv *env, jobject obj)
{
    try
    {
        QuadSamplingControllerInfo *classInfo = QuadSamplingControllerInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            QuadSamplingController_Android *control = classInfo->getObject(env,obj);
            if (!control)
                return;
            delete control;
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadSamplingLayer::dispose()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadSamplingLayer_getNumClients
        (JNIEnv *env, jobject obj)
{
    try
    {
        QuadSamplingController_Android *control = QuadSamplingControllerInfo::getClassInfo()->getObject(env,obj);
        if (!control)
            return 0;
        return control->getNumClients();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadSamplingLayer::getNumClients()");
    }

    return 0;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadSamplingLayer_viewUpdatedNative
        (JNIEnv *env, jobject obj, jobject viewStateObj, jobject changeObj)
{
    try
    {
        QuadSamplingController_Android *control = QuadSamplingControllerInfo::getClassInfo()->getObject(env,obj);
        ViewStateRef *viewState = ViewStateRefClassInfo::getClassInfo()->getObject(env,viewStateObj);
        ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
        if (!control || !viewState || !changes || !control->getDisplayControl())
            return true;
        PlatformInfo_Android platformInfo(env);
        return control->getDisplayControl()->viewUpdate(&platformInfo,*viewState,*(changes->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadSamplingLayer::getNumClients()");
    }

    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadSamplingLayer_startNative
        (JNIEnv *env, jobject obj, jobject paramsObj, jobject sceneObj, jobject renderObj)
{
    try
    {
        QuadSamplingController_Android *control = QuadSamplingControllerInfo::getClassInfo()->getObject(env,obj);
        SamplingParams *params = SamplingParamsClassInfo::getClassInfo()->getObject(env,paramsObj);
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
        SceneRendererGLES_Android *render = SceneRendererInfo::getClassInfo()->getObject(env,renderObj);
        if (!control || !params || !scene || !render)
            return;
        PlatformInfo_Android platformInfo(env);
        control->start(*params,scene,render);
        control->getDisplayControl()->start();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadSamplingLayer::startNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadSamplingLayer_preSceneFlushNative
        (JNIEnv *env, jobject obj, jobject changeObj)
{
    try
    {
        QuadSamplingController_Android *control = QuadSamplingControllerInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
        if (!control || !changes || !control->getDisplayControl())
            return;
        control->getDisplayControl()->preSceneFlush(*(changes->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadSamplingLayer::preSceneFlushNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadSamplingLayer_shutdownNative
        (JNIEnv *env, jobject obj, jobject changeObj)
{
    try
    {
        QuadSamplingController_Android *control = QuadSamplingControllerInfo::getClassInfo()->getObject(env,obj);
        ChangeSetRef *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
        if (!control || !changes || !control->getDisplayControl())
            return;
        PlatformInfo_Android platformInfo(env);
        control->getDisplayControl()->stop(&platformInfo,*(changes->get()));
        control->stop();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadSamplingLayer::shutdownNative()");
    }
}
