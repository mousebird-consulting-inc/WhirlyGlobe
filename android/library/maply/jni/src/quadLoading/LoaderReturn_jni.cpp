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
  (JNIEnv *env, jobject obj, jobject loaderObj)
{
	try
	{
		QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,loaderObj);
	    QuadLoaderReturn *load = new QuadLoaderReturn((*loader)->getGeneration());
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
            QuadLoaderReturn *loader = classInfo->getObject(env,obj);
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
        QuadLoaderReturn *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
        if (!loadReturn)
            return;
        loadReturn->ident.x = tileX;
        loadReturn->ident.y = tileY;
        loadReturn->ident.level = tileLevel;
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
        QuadLoaderReturn *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
        if (!loadReturn)
            return;
        loadReturn->frame->frameIndex = frame;
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
		QuadLoaderReturn *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
		    return NULL;
		std::vector<int> rets(3);
		rets[0] = loadReturn->ident.x;
		rets[1] = loadReturn->ident.y;
		rets[2] = loadReturn->ident.level;

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
		QuadLoaderReturn *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
		    return -1;
		return loadReturn->frame->frameIndex;
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
		QuadLoaderReturn *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeObj);
		if (!loadReturn || !changeSet)
			return;

		loadReturn->changes.insert(loadReturn->changes.end(),changeSet->begin(),changeSet->end());
		changeSet->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LoaderReturn::mergeChanges()");
	}
}