/*  LoaderReturn_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/20/19.
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

#import "QuadLoading_jni.h"
#import "Scene_jni.h"
#import "Components_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_LoaderReturn.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> LoaderReturnClassInfo *LoaderReturnClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_nativeInit(JNIEnv *env, jclass cls)
{
	LoaderReturnClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_initialise(JNIEnv *env, jobject obj)
{
	try
	{
	    auto load = new QuadLoaderReturnRef(new QuadLoaderReturn(0));
		(*load)->frame = std::make_shared<QuadFrameInfo>();
		(*load)->frame->frameIndex = 0;
		LoaderReturnClassInfo::getClassInfo()->setHandle(env,obj,load);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::initialise()");
	}
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		LoaderReturnClassInfo *classInfo = LoaderReturnClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		auto loader = classInfo->getObject(env,obj);
		delete loader;
		classInfo->clearHandle(env, obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::dispose()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_setTileID
        (JNIEnv *env, jobject obj, jint tileX, jint tileY, jint tileLevel)
{
    try
    {
        if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			(*loadReturn)->ident.x = tileX;
			(*loadReturn)->ident.y = tileY;
			(*loadReturn)->ident.level = tileLevel;
		}
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::setTileID()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_setFrame(JNIEnv *env, jobject obj, jlong frameID, jint frameIndex)
{
	try
	{
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			(*loadReturn)->frame = std::make_shared<QuadFrameInfo>();
			(*loadReturn)->frame->setId(frameID);
			(*loadReturn)->frame->frameIndex = frameIndex;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::setTileID()");
	}
}

extern "C"
JNIEXPORT jintArray JNICALL Java_com_mousebird_maply_LoaderReturn_getTileIDNative(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			const std::vector<int> rets {
					(*loadReturn)->ident.x,
					(*loadReturn)->ident.y,
					(*loadReturn)->ident.level,
			};
			return BuildIntArray(env, rets);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::getTileIDNative()");
	}
	return nullptr;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_LoaderReturn_getFrame(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			return (*loadReturn)->frame->frameIndex;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::getFrame()");
	}
	return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_mergeChanges(JNIEnv *env, jobject obj, jobject changeObj)
{
	try
	{
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			if (auto changeSet = ChangeSetClassInfo::get(env, changeObj)) {
				(*loadReturn)->changes.insert((*loadReturn)->changes.end(),
				                              (*changeSet)->begin(),
				                              (*changeSet)->end());
				(*changeSet)->clear();
			}
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::mergeChanges()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_setGeneration(JNIEnv *env, jobject obj, jint generation)
{
	try
	{
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			(*loadReturn)->generation = generation;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::setGeneration()");
	}
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_LoaderReturn_getGeneration(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			return (*loadReturn)->generation;
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::getGeneration()");
	}
	return 0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_addComponentObjects
		(JNIEnv *env, jobject obj, jobjectArray compObjs, jboolean isOverlay)
{
	try
	{
		if (!compObjs)
			return;
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			// Work through the component object array
			const auto compObjClassInfo = ComponentObjectRefClassInfo::getClassInfo();
			JavaObjectArrayHelper compObjHelp(env, compObjs);
			if (compObjHelp.numObjects() == 0)
				return;
			while (jobject compObjObj = compObjHelp.getNextObject()) {
				ComponentObjectRef *compObj = compObjClassInfo->getObject(env, compObjObj);
				if (isOverlay)
					(*loadReturn)->ovlCompObjs.push_back(*compObj);
				else
					(*loadReturn)->compObjs.push_back(*compObj);
			}
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::addComponentObjects()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_clearComponentObjectsNative
		(JNIEnv *env, jobject obj, jboolean isOverlay)
{
	try
	{
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj)) {
			if (isOverlay)
				(*loadReturn)->ovlCompObjs.clear();
			else
				(*loadReturn)->compObjs.clear();
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::clearCompObjs()");
	}

}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_deleteComponentObjects
		(JNIEnv *env, jobject obj, jobject renderControlObj, jobject compManagerObj, jobject changeSetObj)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		SceneRendererGLES_Android *renderer = SceneRendererInfo::getClassInfo()->getObject(env,renderControlObj);
		ComponentManager_AndroidRef *compManager = ComponentManagerClassInfo::getClassInfo()->getObject(env,compManagerObj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!loadReturn || !renderer || !compManager || !changeSet)
			return;

		// Process the remaining changes (could be deletes in there)
		if (!(*loadReturn)->changes.empty()) {
			(*changeSet)->insert((*changeSet)->begin(), (*loadReturn)->changes.begin(),
								 (*loadReturn)->changes.end());
			(*loadReturn)->changes.clear();
		}

		// And then the IDs from the component objects just added
		SimpleIDSet idSet;
		for (auto compObj: (*loadReturn)->compObjs)
			idSet.insert(compObj->getId());
		for (auto compObj: (*loadReturn)->ovlCompObjs)
			idSet.insert(compObj->getId());

		PlatformInfo_Android platformInfo(env);
		(*compManager)->removeComponentObjects(&platformInfo, idSet, *(*changeSet));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LoaderReturn::deleteComponentObjects()");
	}
}
