/*
 *  Texture_jni.cpp
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

#import <jni.h>
#import <android/bitmap.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_Texture.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

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
		Texture *tex = new Texture("jni");
		TextureClassInfo::getClassInfo()->setHandle(env,obj,tex);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		TextureClassInfo *classInfo = TextureClassInfo::getClassInfo();
		Texture *tex = classInfo->getObject(env,obj);
		if (!tex)
			return;
		delete tex;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::dispose()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Texture_setBitmap
  (JNIEnv *env, jobject obj, jobject bitmapObj)
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
		void* bitmapPixels;
		if (AndroidBitmap_lockPixels(env, bitmapObj, &bitmapPixels) < 0)
			return false;

		uint32_t* src = (uint32_t*) bitmapPixels;
		MutableRawData *rawData = new MutableRawData(bitmapPixels,info.height*info.width*4);
		tex->setRawData(rawData, info.width, info.height);
		AndroidBitmap_unlockPixels(env, bitmapObj);
		classInfo->setHandle(env, obj, tex);

		return true;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Texture::setBitmap()");
	}

	return false;
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
}
