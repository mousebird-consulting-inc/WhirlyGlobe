/*
 *  LoaderReturn_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/20/19.
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
#import "Scene_jni.h"
#import "Components_jni.h"
#import "com_mousebird_maply_LoaderReturn.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> LoaderReturnClassInfo *LoaderReturnClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_nativeInit
  (JNIEnv *env, jclass cls)
{
	LoaderReturnClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
	    QuadLoaderReturnRef *load = new QuadLoaderReturnRef(new QuadLoaderReturn(0));
		(*load)->frame = QuadFrameInfoRef(new QuadFrameInfo());
		(*load)->frame->frameIndex = 0;
		LoaderReturnClassInfo::getClassInfo()->setHandle(env,obj,load);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		LoaderReturnClassInfo *classInfo = LoaderReturnClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            QuadLoaderReturnRef *loader = classInfo->getObject(env,obj);
            if (!loader)
                return;
            delete loader;
            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_setTileID
        (JNIEnv *env, jobject obj, jint tileX, jint tileY, jint tileLevel)
{
    try
    {
        QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
        if (!loadReturn)
            return;
		(*loadReturn)->ident.x = tileX;
		(*loadReturn)->ident.y = tileY;
		(*loadReturn)->ident.level = tileLevel;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::setTileID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_setFrame
		(JNIEnv *env, jobject obj, jlong frameID, jint frameIndex)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
			return;
		(*loadReturn)->frame = QuadFrameInfoRef(new QuadFrameInfo());
		(*loadReturn)->frame->setId(frameID);
		(*loadReturn)->frame->frameIndex = frameIndex;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::setTileID()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_setFrame
        (JNIEnv *env, jobject obj, jint frame)
{
    try
    {
        QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
        if (!loadReturn)
            return;
        if ((*loadReturn)->frame)
			(*loadReturn)->frame->frameIndex = frame;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::setFrame()");
    }
}

JNIEXPORT jintArray JNICALL Java_com_mousebird_maply_LoaderReturn_getTileIDNative
  (JNIEnv *env, jobject obj)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
		    return NULL;
		std::vector<int> rets(3);
		rets[0] = (*loadReturn)->ident.x;
		rets[1] = (*loadReturn)->ident.y;
		rets[2] = (*loadReturn)->ident.level;

		return BuildIntArray(env,rets);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::getTileIDNative()");
	}

	return NULL;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LoaderReturn_getFrame
  (JNIEnv *env, jobject obj)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
		    return -1;
		return (*loadReturn)->frame->frameIndex;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::getFrame()");
	}

	return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_mergeChanges
		(JNIEnv *env, jobject obj, jobject changeObj)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
		if (!loadReturn || !changeSet)
			return;

		(*loadReturn)->changes.insert((*loadReturn)->changes.end(),(*changeSet)->begin(),(*changeSet)->end());
		(*changeSet)->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::mergeChanges()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_setGeneration
		(JNIEnv *env, jobject obj, jint generation)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
			return;
		(*loadReturn)->generation = generation;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::setGeneration()");
	}
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_LoaderReturn_getGeneration
		(JNIEnv *env, jobject obj)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
			return 0;
		return (*loadReturn)->generation;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::getGeneration()");
	}
	return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_addComponentObjects
		(JNIEnv *env, jobject obj, jobjectArray compObjs, jboolean isOverlay)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn || !compObjs)
			return;

		// Work through the component object array
		ComponentObjectRefClassInfo *compObjClassInfo = ComponentObjectRefClassInfo::getClassInfo();
		JavaObjectArrayHelper compObjHelp(env,compObjs);
		if (compObjHelp.numObjects() == 0)
			return;
		while (jobject compObjObj = compObjHelp.getNextObject()) {
			ComponentObjectRef *compObj = compObjClassInfo->getObject(env,compObjObj);
			if (isOverlay)
				(*loadReturn)->ovlCompObjs.push_back(*compObj);
			else
				(*loadReturn)->compObjs.push_back(*compObj);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::addComponentObjects()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LoaderReturn_clearComponentObjectsNative
		(JNIEnv *env, jobject obj, jboolean isOverlay)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
			return;

		if (isOverlay)
			(*loadReturn)->ovlCompObjs.clear();
		else
			(*loadReturn)->compObjs.clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::clearCompObjs()");
	}

}