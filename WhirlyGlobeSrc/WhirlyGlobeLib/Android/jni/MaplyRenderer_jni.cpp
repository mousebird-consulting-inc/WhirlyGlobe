/*
 * MaplyRenderer_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */

#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_MaplyRenderer.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

class MaplySceneRenderer : public SceneRendererES2
{
public:
	MaplySceneRenderer()
		:context(0)
	{
	}

	// Called when the window changes size (or on startup)
	bool resize(int width,int height)
	{
		context = eglGetCurrentContext();

		framebufferWidth = width;
		framebufferHeight = height;
		lastDraw = 0;

		return true;
	}

	EGLContext context;
};

static bool glSetup = false;

void Java_com_mousebirdconsulting_maply_MaplyRenderer_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		MaplySceneRenderer *renderer = new MaplySceneRenderer();
		setHandle(env,obj,renderer);

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

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MaplyRenderer_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MaplySceneRenderer *inst = getHandle<MaplySceneRenderer>(env,obj);
		if (!inst)
			return;
		delete inst;

		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MaplyRenderer_setScene
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		SceneRendererES2 *renderer = getHandle<SceneRendererES2>(env,obj);
		Maply::MapScene *scene = getHandle<Maply::MapScene>(env,sceneObj);
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

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MaplyRenderer_setView
  (JNIEnv *env, jobject obj, jobject objView)
{
	try
	{
		SceneRendererES2 *renderer = getHandle<SceneRendererES2>(env,obj);
		Maply::MapView *view = getHandle<Maply::MapView>(env,objView);
		if (!renderer || !view)
			return;

		renderer->setView(view);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::setView()");
	}
}

/*
 * Class:     com_mousebirdconsulting_maply_MaplyRenderer
 * Method:    teardown
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebirdconsulting_maply_MaplyRenderer_teardown
  (JNIEnv *, jobject)
{
	return true;
}

/*
 * Class:     com_mousebirdconsulting_maply_MaplyRenderer
 * Method:    resize
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebirdconsulting_maply_MaplyRenderer_resize
  (JNIEnv *env, jobject obj, jint width, jint height)
{
	try
	{
		MaplySceneRenderer *renderer = getHandle<MaplySceneRenderer>(env,obj);
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
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MaplyRenderer_render
  (JNIEnv *env, jobject obj)
{
	try
	{
		MaplySceneRenderer *renderer = getHandle<MaplySceneRenderer>(env,obj);
		if (!renderer)
			return;

		renderer->render();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MaplyRenderer::render()");
	}
}
