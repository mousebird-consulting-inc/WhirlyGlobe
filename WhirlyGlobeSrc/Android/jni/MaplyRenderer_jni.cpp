/*
 *  MaplyRenderer_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2015 mousebird consulting
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

JNIEXPORT void JNICALL Java_com_mousebird_maply_MaplyRenderer_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MaplySceneRendererInfo *classInfo = MaplySceneRendererInfo::getClassInfo();
		MaplySceneRenderer *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
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
		// Note: Porting
		// Set up no lighting shaders here
        SimpleIdentity triNoLighting = scene->getProgramIDByName(kToolkitDefaultTriangleNoLightingProgram);
        if (triNoLighting != EmptyIdentity)
            scene->setSceneProgram(kSceneDefaultTriShader, triNoLighting);
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
		renderer->render();
		maplyCurrentEnv = NULL;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::render()");
	}
}
