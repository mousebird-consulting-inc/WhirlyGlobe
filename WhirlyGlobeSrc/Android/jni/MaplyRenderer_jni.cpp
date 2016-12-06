/*
 *  MaplyRenderer_jni.cpp
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
#import "com_mousebird_maply_MaplyRenderer.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


MaplySceneRenderer::MaplySceneRenderer()
:context(0)
{
    extraFrameMode = true;
}

// Called when the window changes size (or on startup)
bool MaplySceneRenderer::resize(int width,int height)
{
    context = eglGetCurrentContext();
    
    framebufferWidth = width;
    framebufferHeight = height;
    lastDraw = 0;
    forceRenderSetup();
    
    return true;
}

// There are callbacks deep within the renderer.
// Some of those need to call out to
JNIEnv *maplyCurrentEnv = NULL;

static bool glSetup = false;

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_nativeInit
  (JNIEnv *env, jclass cls)
{
	MaplySceneRendererInfo::getClassInfo(env,cls);
}

void Java_com_mousebird_maply_MaplyRenderer_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		MaplySceneRenderer *renderer = new MaplySceneRenderer();
		renderer->setZBufferMode(zBufferOffDefault);
		renderer->setClearColor(RGBAColor(255,255,255,255));
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
		classInfo->setHandle(env,obj,renderer);

		if (!glSetup)
		{
			SetupGLESExtensions();
			glSetup = true;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::initialise()");
	}

//	renderer->setup();
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            MaplySceneRenderer *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_setScene
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
		MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		if (!renderer || !scene)
			return;

		renderer->setScene(scene);
		// Set up lighting shaders here
        SimpleIdentity triLighting = scene->getProgramIDByName(kToolkitDefaultTriangleProgram);
        if (triLighting != EmptyIdentity)
            scene->setSceneProgram(kSceneDefaultTriShader, triLighting);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::setScene()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_setViewNative
  (JNIEnv *env, jobject obj, jobject objView)
{
	try
	{
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
		MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
		WhirlyKit::View *view = ViewClassInfo::getClassInfo()->getObject(env,objView);
		if (!renderer || !view)
			return;

		renderer->setView(view);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::setView()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_setClearColor
  (JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
		MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
		if (!renderer)
			return;

		renderer->setClearColor(RGBAColor(r*255,g*255,b*255,a*255));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::setClearColor()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_setPerfInterval
  (JNIEnv *env, jobject obj, jint perfInterval)
{
	try
	{
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
		MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
		if (!renderer)
			return;

		renderer->setPerfInterval(perfInterval);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::setPerfInterval()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MaplyRenderer_teardown
  (JNIEnv *, jobject)
{
	return true;
}

/*
 * Class:     com_mousebird_maply_MaplyRenderer
 * Method:    resize
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MaplyRenderer_resize
  (JNIEnv *env, jobject obj, jint width, jint height)
{
	try
	{
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
		MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
		if (!renderer)
			return false;
        
		renderer->resize(width,height);

		// Note: Porting.
	//    if (theView)
	//        theView->runViewUpdates();

		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::resize()");
	}
    
    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MaplyRenderer_hasChanges
(JNIEnv *env, jobject obj)
{
    try
    {
        MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
        MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
        
        if (!renderer)
            return false;
        
        return renderer->hasChanges();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::hasChanges()");
    }
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_render
  (JNIEnv *env, jobject obj)
{
	try
	{
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
		MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
        
		if (!renderer)
			return;

		maplyCurrentEnv = env;
        // Force a draw every time
        renderer->triggerDraw = true;
		renderer->render();
		maplyCurrentEnv = NULL;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::render()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_addLight
(JNIEnv *env, jobject obj, jobject lightObj)
{
    try
    {
        MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
        MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
        WhirlyKitDirectionalLight *light = DirectionalLightClassInfo::getClassInfo()->getObject(env, lightObj);
        if (!renderer || !light)
            return;

        renderer->addLight(light);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::addLight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_replaceLights
(JNIEnv *env, jobject obj, jobject arrayObj)
{
    try
    {
        MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
        MaplySceneRenderer *renderer = classInfo->getObject(env,obj);
        if (!renderer)
            return;

        // Get the iterator
        // Note: Look these up once
        jclass listClass = env->GetObjectClass(arrayObj);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(arrayObj,literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass,"hasNext","()Z");
        jmethodID next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(iterClass);
        env->DeleteLocalRef(listClass);

        std::vector<WhirlyKitDirectionalLight> lights;
        DirectionalLightClassInfo *directionalLightClassInfo = DirectionalLightClassInfo::getClassInfo();
        while (env->CallBooleanMethod(liter, hasNext))
        {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            WhirlyKitDirectionalLight *light = directionalLightClassInfo->getObject(env,javaVecObj);
            lights.push_back(*light);
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);
        renderer->replaceLights(lights);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::replaceLights()");
    }
}
