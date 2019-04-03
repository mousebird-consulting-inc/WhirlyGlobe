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

#import "Renderer_jni.h"
#import "Scene_jni.h"
#import "View_jni.h"
#import "com_mousebird_maply_RenderController.h"

using namespace WhirlyKit;

template<> SceneRendererInfo *SceneRendererInfo::classInfoObj = NULL;

namespace WhirlyKit {
GLenum ImageFormatToGLenum(MaplyImageType format) {
	GLenum ret = GL_UNSIGNED_BYTE;

	switch (format) {
		case MaplyImageIntRGBA:
		case MaplyImage4Layer8Bit:
			ret = GL_UNSIGNED_BYTE;
			break;
		case MaplyImageUShort565:
			ret = GL_UNSIGNED_SHORT_5_6_5;
			break;
		case MaplyImageUShort4444:
			ret = GL_UNSIGNED_SHORT_4_4_4_4;
			break;
		case MaplyImageUShort5551:
			ret = GL_UNSIGNED_SHORT_5_5_5_1;
			break;
		case MaplyImageUByteRed:
		case MaplyImageUByteGreen:
		case MaplyImageUByteBlue:
		case MaplyImageUByteAlpha:
		case MaplyImageUByteRGB:
			ret = GL_ALPHA;
			break;
		default:
			break;
			// Note: Not supporting everything
//				MaplyImageETC2RGB8,MaplyImageETC2RGBA8,MaplyImageETC2RGBPA8,
//				MaplyImageEACR11,MaplyImageEACR11S,MaplyImageEACRG11,MaplyImageEACRG11S,
	}

	return ret;
}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_nativeInit
  (JNIEnv *env, jclass cls)
{
	SceneRendererInfo::getClassInfo(env,cls);
}

void Java_com_mousebird_maply_RenderController_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		SceneRendererES2_Android *renderer = new SceneRendererES2_Android();
		renderer->setZBufferMode(zBufferOffDefault);
		renderer->setClearColor(RGBAColor(255,255,255,255));
		SceneRendererInfo *classInfo = SceneRendererInfo::getClassInfo();
		classInfo->setHandle(env,obj,renderer);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::initialise()");
	}

//	renderer->setup();
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		SceneRendererInfo *classInfo = SceneRendererInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
			SceneRendererES2_Android *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setScene
		(JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		if (!renderer || !scene)
			return;

		renderer->setScene(scene);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::setScene()");
	}
}

// Wrapper used to call back into Scene Renderer for its addPreBuildShader method
class SceneRendererWrapper
{
public:
	SceneRendererWrapper(JNIEnv *env,Scene *scene,jobject obj)
	: env(env), scene(scene), renderControlObj(obj)
	{
		addShaderID = env->GetMethodID(SceneRendererInfo::getClassInfo()->getClass(),"addPreBuiltShader","(Lcom/mousebird/maply/Shader;)V");
	}

	// Add a shader and let the Java side RenderController keep it
	void addShader(const std::string &name,OpenGLES2Program *prog)
	{
		Shader_Android *localShader = new Shader_Android();
		localShader->setupPreBuildProgram(prog);
		scene->addProgram(prog);
		jobject shaderObj = MakeShader(env,localShader);
		env->CallVoidMethod(renderControlObj,addShaderID,shaderObj);
	}

	JNIEnv *env;
	Scene *scene;
	jobject renderControlObj;
	jmethodID addShaderID;
};

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setupShadersNative
		(JNIEnv *env, jobject obj)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		SceneRendererWrapper rendWrap(env,renderer->getScene(),obj);

		// Default line shaders
		OpenGLES2Program *defaultLineShader = BuildDefautLineShaderCulling(MaplyDefaultLineShader);
		OpenGLES2Program *defaultLineShaderNoBack = BuildDefaultLineShaderNoCulling(MaplyNoBackfaceLineShader);
		if (dynamic_cast<WhirlyGlobe::GlobeScene *>(renderer->getScene()))
			rendWrap.addShader(MaplyDefaultLineShader,defaultLineShader);
		else
			rendWrap.addShader(MaplyNoBackfaceLineShader,defaultLineShaderNoBack);

		// Default triangle shaders
		rendWrap.addShader(MaplyDefaultTriangleShader,BuildDefaultTriShaderLighting(MaplyDefaultTriangleShader));
		rendWrap.addShader(MaplyNoLightTriangleShader,BuildDefaultTriShaderNoLighting(MaplyNoLightTriangleShader));

		// Model instancing
		rendWrap.addShader(MaplyDefaultModelTriShader,BuildDefaultTriShaderModel(MaplyDefaultModelTriShader));

		// Screen space texture application
		rendWrap.addShader(MaplyDefaultTriScreenTexShader,BuildDefaultTriShaderScreenTexture(MaplyDefaultTriScreenTexShader));

		// Multi-texture support
		rendWrap.addShader(MaplyDefaultTriMultiTexShader,BuildDefaultTriShaderMultitex(MaplyDefaultTriMultiTexShader));
		rendWrap.addShader(MaplyDefaultMarkerShader,BuildDefaultTriShaderMultitex(MaplyDefaultMarkerShader));

		// Ramp texture support
		rendWrap.addShader(MaplyDefaultTriMultiTexRampShader,BuildDefaultTriShaderRamptex(MaplyDefaultTriMultiTexRampShader));

		// Night/day shading for globe
		rendWrap.addShader(MaplyDefaultTriNightDayShader,BuildDefaultTriShaderNightDay(MaplyDefaultTriNightDayShader));

		// Billboards
		rendWrap.addShader(MaplyBillboardGroundShader,BuildBillboardGroundProgram(MaplyBillboardGroundShader));
		rendWrap.addShader(MaplyBillboardEyeShader,BuildBillboardEyeProgram(MaplyBillboardEyeShader));

		// Wide vectors
		rendWrap.addShader(MaplyDefaultWideVectorShader,BuildWideVectorProgram(MaplyDefaultWideVectorShader));
		rendWrap.addShader(MaplyDefaultWideVectorGlobeShader,BuildWideVectorGlobeProgram(MaplyDefaultWideVectorGlobeShader));
		// Screen space
		rendWrap.addShader(MaplyScreenSpaceDefaultMotionShader,BuildScreenSpaceProgram(MaplyScreenSpaceDefaultMotionShader));
		rendWrap.addShader(MaplyScreenSpaceDefaultShader,BuildScreenSpaceMotionProgram(MaplyScreenSpaceDefaultShader));
		// Particles
		rendWrap.addShader(MaplyParticleSystemPointDefaultShader,BuildParticleSystemProgram(MaplyParticleSystemPointDefaultShader));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::setScene()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setViewNative
		(JNIEnv *env, jobject obj, jobject objView)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		WhirlyKit::View *view = ViewClassInfo::getClassInfo()->getObject(env,objView);
		if (!renderer || !view)
			return;

		renderer->setView(view);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::setView()");
	}
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setClearColor
		(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		renderer->setClearColor(RGBAColor(r*255,g*255,b*255,a*255));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::setClearColor()");
	}
}


JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_RenderController_teardown
		(JNIEnv *env, jobject obj)
{
	return true;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_RenderController_resize
		(JNIEnv *env, jobject obj, jint width, jint height)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return false;

		renderer->resize(width,height);

		// TODO: Turn this back on?
		//    if (theView)
		//        theView->runViewUpdates();

		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::resize()");
	}

	return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_render
		(JNIEnv *env, jobject obj)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		// Force a draw every time
		renderer->triggerDraw = true;
		renderer->render(1/60.0 * renderer->perfInterval);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::render()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_RenderController_hasChanges
		(JNIEnv *env, jobject obj)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return false;

		return renderer->hasChanges();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::hasChanges()");
	}

	return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setPerfInterval
		(JNIEnv *env, jobject obj, jint perfInterval)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		renderer->setPerfInterval(perfInterval);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::setPerfInterval()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_addLight
		(JNIEnv *env, jobject obj, jobject lightObj)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		DirectionalLight *light = DirectionalLightClassInfo::getClassInfo()->getObject(env, lightObj);
		if (!renderer || !light)
			return;

		renderer->addLight(*light);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::addLight()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_replaceLights
		(JNIEnv *env, jobject obj, jobjectArray lightArray)
{
	try
	{
		SceneRendererES2_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		// Work through the array of lights
		std::vector<DirectionalLight> lights;
		JavaObjectArrayHelper arrayHelp(env,lightArray);
        DirectionalLightClassInfo *lightClassInfo = DirectionalLightClassInfo::getClassInfo();
		while (jobject lightObj = arrayHelp.getNextObject()) {
            DirectionalLight *light = lightClassInfo->getObject(env,lightObj);
            if (light)
                lights.push_back(*light);
		}

		renderer->replaceLights(lights);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::replaceLights()");
	}
}
