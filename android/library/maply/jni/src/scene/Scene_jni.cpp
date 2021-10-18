/*  Scene_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
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

#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "CoordSystem_jni.h"
#import "com_mousebird_maply_Scene.h"

using namespace WhirlyKit;

template<> SceneClassInfo *SceneClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_nativeInit(JNIEnv *env, jclass cls)
{
	SceneClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_initialise
  (JNIEnv *env, jobject obj, jobject coordAdapterObj,
   jobject renderControlObj, jobject charRendererObj)
{
    try
    {
        CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::get(env,coordAdapterObj);
        SceneRendererGLES_Android *sceneRender = SceneRendererInfo::get(env,renderControlObj);

        auto scene = std::make_unique<SceneGLES>(coordAdapter);

        PlatformInfo_Android inst(env);
        auto mgr = std::make_shared<FontTextureManager_Android>(&inst,sceneRender,scene.get(),charRendererObj);
        scene->setFontTextureManager(std::move(mgr));

        SceneClassInfo::set(env,obj,scene.release());
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        SceneClassInfo *classInfo = SceneClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        if (Scene *scene = classInfo->getObject(env, obj))
        {
            // This should already have been done, but this is our last chance to do anything with
            // a JNI environment, so do it again just in case.
            if (scene->getRenderer())
            {
                wkLogLevel(Warn, "Scene disposed without teardown");
            }

            PlatformInfo_Android inst(env);
            scene->teardown(&inst);

            delete scene;
            classInfo->clearHandle(env,obj);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_addChangesNative(JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		SceneClassInfo *classInfo = SceneClassInfo::getClassInfo();
        if (auto changes = ChangeSetClassInfo::getClassInfo()->getObject(env, changesObj))
        {
            if (Scene *scene = classInfo->getObject(env, obj))
            {
                scene->addChangeRequests(*(changes->get()));
                (*changes)->clear();
            }
        }
	}
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_addShaderProgram(JNIEnv *env, jobject obj, jobject shaderObj)
{
    try
    {
        if (Scene *scene = SceneClassInfo::get(env,obj))
        if (Shader_AndroidRef *shader = ShaderClassInfo::get(env,shaderObj))
        {
            scene->addProgram((*shader)->prog);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_removeShaderProgram(JNIEnv *env, jobject obj, jlong shaderID)
{
    try
    {
        if (Scene *scene = SceneClassInfo::get(env,obj))
        {
            scene->removeProgram(shaderID, nullptr);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_teardownGL(JNIEnv *env, jobject obj)
{
    try
    {
        if (Scene *scene = SceneClassInfo::get(env,obj))
        {
            PlatformInfo_Android platformInfo(env);
            scene->teardown(&platformInfo);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_addRenderTargetNative
  (JNIEnv *env, jobject obj, jlong renderTargetID, jint width, jint height, jlong texID,
   jboolean clearEveryFrame, jfloat clearVal, jboolean blend, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        if (Scene *scene = SceneClassInfo::get(env,obj))
        {
            ChangeSet changes;
            const RGBAColor color(r * 255.0, g * 255.0, b * 255.0, a * 255.0);
            changes.push_back(
                    new AddRenderTargetReq(renderTargetID, width, height, texID, clearEveryFrame,
                                           blend, color, clearVal, RenderTargetMipmapNone, false));

            scene->addChangeRequests(changes);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_changeRenderTarget(JNIEnv *env, jobject obj, jlong renderTargetID, jlong texID)
{
    try
    {
        if (Scene *scene = SceneClassInfo::get(env,obj))
        {
            ChangeSet changes = { new ChangeRenderTargetReq(renderTargetID, texID) };
            scene->addChangeRequests(changes);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_removeRenderTargetNative(JNIEnv *env, jobject obj, jlong targetID)
{
    try
    {
        if (Scene *scene = SceneClassInfo::get(env,obj))
        {
            ChangeSet changes = { new RemRenderTargetReq(targetID) };
            scene->addChangeRequests(changes);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT float JNICALL Java_com_mousebird_maply_Scene_getZoomSlotValue(JNIEnv *env, jobject obj, jint slot)
{
    try
    {
        if (Scene *scene = SceneClassInfo::get(env,obj))
        {
            return scene->getZoomSlotValue(slot);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Scene_copyZoomSlots(JNIEnv *env, jobject obj, jobject otherObj, jfloat offset)
{
    try
    {
        if (Scene *thisScene = SceneClassInfo::get(env,obj))
        if (Scene *otherScene = SceneClassInfo::get(env,otherObj))
        {
            thisScene->copyZoomSlotsFrom(otherScene, offset);
        }
    }
    MAPLY_STD_JNI_CATCH()
}
