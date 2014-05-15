/*
 * MaplyRenderer_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_MaplyRenderer.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// There are callbacks deep within the renderer.
// Some of those need to call out to
JNIEnv *maplyCurrentEnv = NULL;

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
		Maply::MapScene *scene = MapSceneClassInfo::getClassInfo()->getObject(env,sceneObj);
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
		Maply::MapView *view = MapViewClassInfo::getClassInfo()->getObject(env,objView);
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
