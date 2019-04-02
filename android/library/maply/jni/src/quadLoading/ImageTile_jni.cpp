/*
 *  ImageTile_jni.cpp
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
#import "com_mousebird_maply_ImageTile.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> ImageTileClassInfo *ImageTileClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ImageTile_nativeInit
  (JNIEnv *env, jclass cls)
{
	ImageTileClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ImageTile_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
	    ImageTile_AndroidRef *image = new ImageTile_AndroidRef(new ImageTile_Android());
		ImageTileClassInfo::getClassInfo()->setHandle(env,obj,image);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageTile::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ImageTile_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		ImageTileClassInfo *classInfo = ImageTileClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            ImageTile_AndroidRef *image = classInfo->getObject(env,obj);
            if (!image)
                return;
            delete image;
            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageTile::dispose()");
	}
}

JNIEXPORT jobject JNICALL MakeImageTile(JNIEnv *env,WhirlyKit::ImageTile_AndroidRef imgTile)
{
    ImageTileClassInfo *classInfo = ImageTileClassInfo::getClassInfo(env,"com/mousebird/maply/ImageTile");
    return classInfo->makeWrapperObject(env,new ImageTile_AndroidRef(imgTile));
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ImageTile_setBitmap
  (JNIEnv *env, jobject obj, jobject bitmapObj)
{
	try
	{
		ImageTile_AndroidRef *imageTile = ImageTileClassInfo::getClassInfo()->getObject(env,obj);
		if (!imageTile)
		    return;

		(*imageTile)->setBitmap(env,bitmapObj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageTile::setBitmap()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ImageTile_setBorderSize
  (JNIEnv *env, jobject obj, jint borderSize)
{
	try
	{
		ImageTile_AndroidRef *imageTile = ImageTileClassInfo::getClassInfo()->getObject(env,obj);
		if (!imageTile)
		    return;

        (*imageTile)->borderSize = borderSize;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageTile::setBorderSize()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ImageTile_preprocessTexture
  (JNIEnv *env, jobject obj)
{
	try
	{
		ImageTile_AndroidRef *imageTile = ImageTileClassInfo::getClassInfo()->getObject(env,obj);
		if (!imageTile)
		    return;

		(*imageTile)->prebuildTexture();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageTile::preprocessTexture()");
	}
}
