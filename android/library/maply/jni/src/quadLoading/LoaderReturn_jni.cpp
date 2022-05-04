/*  LoaderReturn_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/20/19.
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
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		LoaderReturnClassInfo *classInfo = LoaderReturnClassInfo::getClassInfo();
		std::unique_lock<std::mutex> lock(disposeMutex);
		auto loadRet = classInfo->getObject(env,obj);
		if (loadRet)
		{
			classInfo->clearHandle(env, obj);
		}
		lock.unlock();

		if (loadRet)
		{
			(*loadRet)->cancel = true;
			delete loadRet;
		}
	}
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()
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
	MAPLY_STD_JNI_CATCH()

}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_deleteComponentObjects
		(JNIEnv *env, jobject obj, jobject renderControlObj,
		 jobject compManagerObj, jobject changeSetObj)
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
		for (const auto& compObj: (*loadReturn)->compObjs)
			idSet.insert(compObj->getId());
		for (const auto& compObj: (*loadReturn)->ovlCompObjs)
			idSet.insert(compObj->getId());

		PlatformInfo_Android platformInfo(env);
		(*compManager)->removeComponentObjects(&platformInfo, idSet, **changeSet, true);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_discardChanges
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (auto loadReturn = LoaderReturnClassInfo::get(env,obj))
        {
            ChangeSet changes = std::move((*loadReturn)->changes);
            for (auto *change : changes)
            {
                delete change;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_cancel(JNIEnv *env, jobject obj)
{
	try
	{
		if (auto loadReturn = LoaderReturnClassInfo::get(env,obj))
		{
			(*loadReturn)->cancel = true;
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LoaderReturn_isCanceled(JNIEnv *env, jobject obj)
{
	try
	{
		auto loadReturn = LoaderReturnClassInfo::get(env,obj);
		return loadReturn && *loadReturn && (*loadReturn)->cancel;
	}
	MAPLY_STD_JNI_CATCH()
	return false;
}
