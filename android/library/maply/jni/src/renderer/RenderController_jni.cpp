/*  MaplyRenderer_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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

#import <android/bitmap.h>
#import "Renderer_jni.h"
#import "Scene_jni.h"
#import "View_jni.h"
#import "com_mousebird_maply_RenderController.h"

using namespace WhirlyKit;

template<> SceneRendererInfo *SceneRendererInfo::classInfoObj = nullptr;

namespace WhirlyKit {
GLenum ImageFormatToGLenum(MaplyImageType format) {
	switch (format) {
		case MaplyImageIntRGBA:
		case MaplyImage4Layer8Bit:    return GL_UNSIGNED_BYTE;
		case MaplyImageUShort565:     return GL_UNSIGNED_SHORT_5_6_5;
		case MaplyImageUShort4444:    return GL_UNSIGNED_SHORT_4_4_4_4;
		case MaplyImageUShort5551:    return GL_UNSIGNED_SHORT_5_5_5_1;
		case MaplyImageUByteRed:
		case MaplyImageUByteGreen:
		case MaplyImageUByteBlue:
		case MaplyImageUByteAlpha:
		case MaplyImageUByteRGB:      return GL_ALPHA;
		case MaplyImageSingleFloat16: return GL_R16F;
		case MaplyImageDoubleFloat16: return GL_RG16F;
		case MaplyImageSingleFloat32: return GL_R32F;
		case MaplyImageDoubleFloat32: return GL_RG32F;
		default:                      return GL_UNSIGNED_BYTE;
			// Note: Not supporting everything
//				MaplyImageETC2RGB8,MaplyImageETC2RGBA8,MaplyImageETC2RGBPA8,
//				MaplyImageEACR11,MaplyImageEACR11S,MaplyImageEACRG11,MaplyImageEACRG11S,
	}
}

TextureType ImageFormatToTexType(MaplyImageType format) {
	switch (format) {
		case MaplyImageIntRGBA:
		case MaplyImage4Layer8Bit:    return TexTypeUnsignedByte;
		case MaplyImageUShort565:     return TexTypeShort565;
		case MaplyImageUShort4444:    return TexTypeShort4444;
		case MaplyImageUShort5551:    return TexTypeShort5551;
		case MaplyImageUByteRed:
		case MaplyImageUByteGreen:
		case MaplyImageUByteBlue:
		case MaplyImageUByteAlpha:
		case MaplyImageUByteRGB:      return TexTypeSingleChannel;
		case MaplyImageSingleFloat16: return TexTypeSingleFloat16;
		case MaplyImageDoubleFloat16: return TexTypeDoubleFloat16;
		case MaplyImageSingleFloat32: return TexTypeSingleFloat32;
		case MaplyImageDoubleFloat32: return TexTypeDoubleFloat32;
		case MaplyImageDoubleInt8:
		case MaplyImageDoubleUInt8:
		case MaplyImageUByteRG: return TexTypeDoubleChannel;
		//case MaplyImageInt8: return TexTypeSignedByte;
		case MaplyImageUInt8: return TexTypeUnsignedByte;
		case MaplyImageInt16: return TexTypeSingleInt16;
		case MaplyImageUInt16: return TexTypeSingleUInt16;
		//case MaplyImageDoubleInt16: return TexTypeDoubleInt16;
		case MaplyImageDoubleUInt16: return TexTypeDoubleUInt16;
		//case MaplyImageInt32: return TexTypeSingleInt32;
		case MaplyImageUInt32: return TexTypeSingleUInt32;
		//case MaplyImageDoubleInt32: return TexTypeDoubleUInt32;
		case MaplyImageDoubleUInt32: return TexTypeDoubleUInt32;
		case MaplyImageQuadUInt32: return TexTypeQuadUInt32;
		case MaplyImageQuadFloat16: return TexTypeQuadFloat16;
		case MaplyImageQuadFloat32: return TexTypeQuadFloat32;
		case MaplyImageETC2RGB8:
		case MaplyImageETC2RGBA8:
		case MaplyImageETC2RGBPA8:
		case MaplyImageEACR11:
		case MaplyImageEACR11S:
		case MaplyImageEACRG11:
		case MaplyImageEACRG11S:      wkLogLevel(Warn, "Unsupported image type %d", format);
		default:                      return TexTypeUnsignedByte;
	}
}

	MaplyImageType TexTypeToImageFormat(TextureType format) {
	switch (format) {
		case TexTypeUnsignedByte:  return MaplyImageIntRGBA;
		case TexTypeShort565:      return MaplyImageUShort565;
		case TexTypeShort4444:     return MaplyImageUShort4444;
		case TexTypeShort5551:     return MaplyImageUShort5551;
		case TexTypeSingleChannel: return MaplyImageUByteRGB;
		case TexTypeSingleFloat16: return MaplyImageSingleFloat16;
		case TexTypeDoubleFloat16: return MaplyImageDoubleFloat16;
		case TexTypeSingleFloat32: return MaplyImageSingleFloat32;
		case TexTypeDoubleFloat32: return MaplyImageDoubleFloat32;
		case TexTypeDoubleChannel: return MaplyImageUByteRG;
		case TexTypeQuadFloat16:   return MaplyImageQuadFloat16;
		case TexTypeQuadFloat32:   return MaplyImageQuadFloat32;
		case TexTypeSingleInt16:   return MaplyImageInt16;
		case TexTypeSingleUInt16:  return MaplyImageUInt16;
		case TexTypeDoubleUInt16:  return MaplyImageDoubleUInt16;
		case TexTypeSingleUInt32:  return MaplyImageUInt32;
		case TexTypeDoubleUInt32:  return MaplyImageDoubleUInt16;
		case TexTypeQuadUInt32:    return MaplyImageQuadUInt32;
		case TexTypeDepthFloat32:
		default:
			wkLogLevel(Warn, "Unsupported texture type %d", format);
			return MaplyImageIntRGBA;
	}
}

}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_nativeInit(JNIEnv *env, jclass cls)
{
	SceneRendererInfo::getClassInfo(env,cls);
}

extern "C"
void Java_com_mousebird_maply_RenderController_initialise__(JNIEnv *env, jobject obj)
{
	try
	{
		SceneRendererGLES_Android *renderer = new SceneRendererGLES_Android();
		renderer->setZBufferMode(zBufferOffDefault);
		renderer->setClearColor(RGBAColor(0,0,0,0));
		renderer->setExtraFrameMode(true);
        SceneRendererInfo::getClassInfo()->setHandle(env,obj,renderer);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::initialise()");
	}

//	renderer->setup();
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_initialise__II
		(JNIEnv *env, jobject obj, jint width, jint height)
{
	try
	{
		auto renderer = new SceneRendererGLES_Android(width,height);
		renderer->setZBufferMode(zBufferOffDefault);
		renderer->setClearColor(RGBAColor(255,255,255,255));
		SceneRendererInfo *classInfo = SceneRendererInfo::getClassInfo();
		classInfo->setHandle(env,obj,renderer);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::initialise()");
	}
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		SceneRendererInfo *classInfo = SceneRendererInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
			SceneRendererGLES_Android *inst = classInfo->getObject(env,obj);
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

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setScene(JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		if (SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj))
		{
			if (sceneObj)
			{
				if (Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj))
				{
					renderer->setScene(scene);
				}
			}
			else
			{
				renderer->setScene(nullptr);
			}
		}
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
	void addShader(const std::string &name,ProgramGLESRef prog) const
	{
		auto localShader = std::make_shared<Shader_Android>();
		localShader->setupPreBuildProgram(std::move(prog));
		if (localShader->prog)
		{
			scene->addProgram(localShader->prog);
		}
		if (jobject shaderObj = MakeShader(env,localShader))
		{
			env->CallVoidMethod(renderControlObj, addShaderID, shaderObj);
		}
	}

	JNIEnv *env;
	Scene *scene;
	jobject renderControlObj;
	jmethodID addShaderID;
};

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setupShadersNative
		(JNIEnv *env, jobject obj)
{
	try
	{
		SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		bool isGlobe = !renderer->getScene()->getCoordAdapter()->isFlat();

		SceneRendererWrapper rendWrap(env,renderer->getScene(),obj);

		// Default line shaders
		if (isGlobe)
			rendWrap.addShader(MaplyDefaultLineShader,ProgramGLESRef(BuildDefaultLineShaderCullingGLES(MaplyDefaultLineShader,renderer)));
		else
			rendWrap.addShader(MaplyDefaultLineShader,ProgramGLESRef(BuildDefaultLineShaderNoCullingGLES(MaplyDefaultLineShader,renderer)));
		rendWrap.addShader(MaplyNoBackfaceLineShader,ProgramGLESRef(BuildDefaultLineShaderNoCullingGLES(MaplyNoBackfaceLineShader,renderer)));

		// Default triangle shaders
		rendWrap.addShader(MaplyDefaultTriangleShader,ProgramGLESRef(BuildDefaultTriShaderLightingGLES(MaplyDefaultTriangleShader,renderer)));
		rendWrap.addShader(MaplyNoLightTriangleShader,ProgramGLESRef(BuildDefaultTriShaderNoLightingGLES(MaplyNoLightTriangleShader,renderer)));

		// Model instancing
		rendWrap.addShader(MaplyDefaultModelTriShader,ProgramGLESRef(BuildDefaultTriShaderModelGLES(MaplyDefaultModelTriShader,renderer)));

		// Screen space texture application
		rendWrap.addShader(MaplyDefaultTriScreenTexShader,ProgramGLESRef(BuildDefaultTriShaderScreenTextureGLES(MaplyDefaultTriScreenTexShader,renderer)));

		// Multi-texture support
		rendWrap.addShader(MaplyDefaultTriMultiTexShader,ProgramGLESRef(BuildDefaultTriShaderMultitexGLES(MaplyDefaultTriMultiTexShader,renderer)));
		rendWrap.addShader(MaplyDefaultMarkerShader,ProgramGLESRef(BuildDefaultTriShaderMultitexGLES(MaplyDefaultMarkerShader,renderer)));

		// Ramp texture support
		rendWrap.addShader(MaplyDefaultTriMultiTexRampShader,ProgramGLESRef(BuildDefaultTriShaderRamptexGLES(MaplyDefaultTriMultiTexRampShader,renderer)));

		// Night/day shading for globe
		rendWrap.addShader(MaplyDefaultTriNightDayShader,ProgramGLESRef(BuildDefaultTriShaderNightDayGLES(MaplyDefaultTriNightDayShader,renderer)));

		// Billboards
		rendWrap.addShader(MaplyBillboardGroundShader,ProgramGLESRef(BuildBillboardGroundProgramGLES(MaplyBillboardGroundShader,renderer)));
		rendWrap.addShader(MaplyBillboardEyeShader,ProgramGLESRef(BuildBillboardEyeProgramGLES(MaplyBillboardEyeShader,renderer)));

		// Wide vectors
		rendWrap.addShader(MaplyDefaultWideVectorGlobeShader,ProgramGLESRef(BuildWideVectorGlobeProgramGLES(MaplyDefaultWideVectorGlobeShader,renderer)));
		if (isGlobe) {
            rendWrap.addShader(MaplyDefaultWideVectorShader,ProgramGLESRef(BuildWideVectorGlobeProgramGLES(MaplyDefaultWideVectorShader,renderer)));
		} else {
            rendWrap.addShader(MaplyDefaultWideVectorShader,ProgramGLESRef(BuildWideVectorProgramGLES(MaplyDefaultWideVectorShader,renderer)));
		}
		// Screen space
		rendWrap.addShader(MaplyScreenSpaceDefaultMotionShader,ProgramGLESRef(BuildScreenSpaceMotionProgramGLES(MaplyScreenSpaceDefaultMotionShader,renderer)));
		rendWrap.addShader(MaplyScreenSpaceDefaultShader,ProgramGLESRef(BuildScreenSpaceProgramGLES(MaplyScreenSpaceDefaultShader,renderer)));
		// Particles
		rendWrap.addShader(MaplyParticleSystemPointDefaultShader,ProgramGLESRef(BuildParticleSystemProgramGLES(MaplyParticleSystemPointDefaultShader,renderer)));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::setScene()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setViewNative(JNIEnv *env, jobject obj, jobject objView)
{
	try
	{
		SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
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


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setClearColor
		(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
	try
	{
        SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		renderer->setClearColor(RGBAColor(r*255,g*255,b*255,a*255));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::setClearColor()");
	}
}


extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_RenderController_teardownNative(JNIEnv *env, jobject obj)
{
	return true;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_RenderController_resize
		(JNIEnv *env, jobject obj, jint width, jint height)
{
	try
	{
        SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
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

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_render(JNIEnv *env, jobject obj)
{
	try
	{
        SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		const bool changes = renderer->hasChanges();

		/// TODO: Make sure this is actually what we're using
		renderer->render(1/60.0, nullptr);

		// Count down the extra frames if we need them
		if (renderer->extraFrameMode) {
			renderer->extraFrameCount = changes ? 1 : renderer->extraFrameCount-1;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::render()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_renderToBitmapNative
        (JNIEnv *env, jobject obj, jobject bitmapObj)
{
	try
	{
		SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

        const auto snapshot = std::make_shared<Snapshot_Android>();
		renderer->addSnapshotDelegate(snapshot);

		renderer->forceDrawNextFrame();
		renderer->render(1/60.0, nullptr);

		// Framebuffer info
		const auto size = renderer->getFramebufferSize();
		const int width = size.x();
		const int height = size.y();

		const RawDataRef data = renderer->getSnapshotAt(EmptyIdentity,0,0,0,0);
		renderer->removeSnapshotDelegate(snapshot);

		if (!data) {
			wkLogLevel(Warn,"Failed to snapshot in RenderController:renderToBitmapNative() (no data)");
			return;
		}

		// Make sure sizes match
		AndroidBitmapInfo bitmapInfo;
		AndroidBitmap_getInfo(env, bitmapObj, &bitmapInfo);
		if (width != bitmapInfo.width || height != bitmapInfo.height) {
			wkLogLevel(Warn,"Failed to snapshot in RenderController:renderToBitmapNative() due to size.");
			return;
		}

		// Copy the data
		void* bitmapPixels = nullptr;
		if (AndroidBitmap_lockPixels(env, bitmapObj, &bitmapPixels) != ANDROID_BITMAP_RESULT_SUCCESS) {
			wkLogLevel(Warn,"Failed to snapshot in RenderController:renderToBitmapNative() because of lockPixels.");
			return;
		}

		try
		{
			// Convert pixels to Bitmap order
			auto *b = (unsigned *) data->getRawData();
			auto *bt = (unsigned *) bitmapPixels;
			for (int i = 0, k = 0; i < height; i++, k++)
			{
				memcpy(&bt[(height - k - 1) * width], &b[i * width], width * 4);
			}
		}
		catch (...)
		{
			AndroidBitmap_unlockPixels(env, bitmapObj);
			throw;
		}
		AndroidBitmap_unlockPixels(env, bitmapObj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::render()");
	}
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_RenderController_hasChanges(JNIEnv *env, jobject obj)
{
	try
	{
        SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return false;

		bool changes = renderer->hasChanges();
        if (renderer->extraFrameMode) {
            // If there were changes, we need two extra frames after things settle
            if (changes) {
                renderer->extraFrameCount = 4;
            } else {
                // No changes, make sure we don't have extra frames to draw
                changes = renderer->extraFrameCount > 0;
            }
        }

        return changes;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::hasChanges()");
	}

	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_setPerfInterval(JNIEnv *env, jobject obj, jint perfInterval)
{
	try
	{
        SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
		if (!renderer)
			return;

		renderer->setPerfInterval(perfInterval);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in RenderController::setPerfInterval()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_addLight(JNIEnv *env, jobject obj, jobject lightObj)
{
	try
	{
        SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
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

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RenderController_replaceLights
		(JNIEnv *env, jobject obj, jobjectArray lightArray)
{
	try
	{
        SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,obj);
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

#include <utility>
