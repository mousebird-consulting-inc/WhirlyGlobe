/*
 *  Texture_jni.cpp
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
 *
 */

#import <android/bitmap.h>
#import "Scene_jni.h"
#import "com_mousebird_maply_Texture.h"

using namespace WhirlyKit;

template<> TextureClassInfo *TextureClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_nativeInit
  (JNIEnv *env, jclass cls)
{
	TextureClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Texture *tex = new TextureGLES("jni");
		TextureClassInfo::getClassInfo()->setHandle(env,obj,tex);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		TextureClassInfo *classInfo = TextureClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Texture *tex = classInfo->getObject(env,obj);
            if (!tex)
                return;
            delete tex;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::dispose()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Texture_setBitmap
  (JNIEnv *env, jobject obj, jobject bitmapObj, jint format)
{
	try
	{
		TextureClassInfo *classInfo = TextureClassInfo::getClassInfo();
		Texture *tex = classInfo->getObject(env,obj);
		if (!tex)
			return false;

		AndroidBitmapInfo info;
		if (AndroidBitmap_getInfo(env, bitmapObj, &info) < 0)
			return false;
		if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
			return false;

		// Copy the raw data over to the texture
		void* bitmapPixels = nullptr;
		if (AndroidBitmap_lockPixels(env, bitmapObj, &bitmapPixels) != ANDROID_BITMAP_RESULT_SUCCESS)
			return false;

		try
		{
			// Make a copy
			auto rawData = new MutableRawData(bitmapPixels, info.height * info.width * 4);
			// takes ownership
			tex->setRawData(rawData, (int)info.width, (int)info.height, 8, 4);
		}
		catch (...)
		{
			// Make sure to release the lock, even in case of an exception
			AndroidBitmap_unlockPixels(env, bitmapObj);
			throw;
		}
		AndroidBitmap_unlockPixels(env, bitmapObj);

		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::setBitmap()");
	}

	return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_setSize
(JNIEnv *env, jobject obj, jint sizeX, jint sizeY)
{
    try
    {
        TextureClassInfo *classInfo = TextureClassInfo::getClassInfo();
        Texture *tex = classInfo->getObject(env,obj);
        if (!tex)
            return;

        tex->setWidth(sizeX);
        tex->setHeight(sizeY);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::setSize()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_setIsEmpty
(JNIEnv *env, jobject obj, jboolean isEmpty)
{
    try
    {
        TextureClassInfo *classInfo = TextureClassInfo::getClassInfo();
        Texture *tex = classInfo->getObject(env,obj);
        if (!tex)
            return;

        tex->setIsEmptyTexture(isEmpty);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::setIsEmpty()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_setSettings
(JNIEnv *env, jobject obj, jboolean wrapU, jboolean wrapV)
{
    TextureClassInfo *classInfo = TextureClassInfo::getClassInfo();
    Texture *tex = classInfo->getObject(env,obj);
    if (!tex)
        return;
    
    tex->setWrap(wrapU,wrapV);
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_Texture_getID
  (JNIEnv *env, jobject obj)
{
	try
	{
		TextureClassInfo *classInfo = TextureClassInfo::getClassInfo();
		Texture *tex = classInfo->getObject(env,obj);
		if (!tex)
			return EmptyIdentity;

		return tex->getId();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::getID()");
	}
    
    return EmptyIdentity;
}
