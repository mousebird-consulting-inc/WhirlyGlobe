/*  Texture_jni.cpp
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
#import "Scene_jni.h"
#import "com_mousebird_maply_Texture.h"

using namespace WhirlyKit;

template<> TextureClassInfo *TextureClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_nativeInit
  (JNIEnv *env, jclass cls)
{
	TextureClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		Texture *tex = new TextureGLES("jni");
		TextureClassInfo::getClassInfo()->setHandle(env,obj,tex);
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		TextureClassInfo *classInfo = TextureClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		Texture *tex = classInfo->getObject(env,obj);
		delete tex;
		classInfo->clearHandle(env,obj);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Texture_setBitmapNative
  (JNIEnv *env, jobject obj, jobject bitmapObj, jint format)
{
	try
	{
		Texture *tex = TextureClassInfo::get(env,obj);
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
			tex->setRawData(std::move(rawData), (int)info.width, (int)info.height, 8, 4);
			tex->setFormat((TextureType)format);
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
	MAPLY_STD_JNI_CATCH()
	return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_setSize
  (JNIEnv *env, jobject obj, jint sizeX, jint sizeY)
{
    try
    {
		if (Texture *tex = TextureClassInfo::get(env, obj))
		{
			tex->setWidth(sizeX);
			tex->setHeight(sizeY);
		}
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_setIsEmpty
  (JNIEnv *env, jobject obj, jboolean isEmpty)
{
    try
    {
		if (Texture *tex = TextureClassInfo::get(env, obj))
		{
			tex->setIsEmptyTexture(isEmpty);
		}
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_setSettings
  (JNIEnv *env, jobject obj, jboolean wrapU, jboolean wrapV)
{
	try
	{
		if (Texture *tex = TextureClassInfo::get(env, obj))
		{
			tex->setWrap(wrapU, wrapV);
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_Texture_getID
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (Texture *tex = TextureClassInfo::get(env,obj))
		{
			return tex->getId();
		}
	}
	MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_setFormatNative
  (JNIEnv *env, jobject obj, jint fmt)
{
	try
	{
		if (Texture *tex = TextureClassInfo::get(env, obj))
		{
			tex->setFormat((TextureType)fmt);
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_Texture_getFormatNative
  (JNIEnv *env, jobject obj)
{
	try
	{
		if (Texture *tex = TextureClassInfo::get(env, obj))
		{
			return tex->getFormat();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Texture_setFilterTypeNative
		(JNIEnv *env, jobject obj, jint type)
{
	try
	{
		if (Texture *tex = TextureClassInfo::get(env, obj))
		{
			tex->setInterpType((TextureInterpType)type);
		}
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_Texture_getFilterTypeNative
		(JNIEnv *env, jobject obj)
{
	try
	{
		if (Texture *tex = TextureClassInfo::get(env, obj))
		{
			return tex->getInterpType();
		}
	}
	MAPLY_STD_JNI_CATCH()
	return -1;
}
