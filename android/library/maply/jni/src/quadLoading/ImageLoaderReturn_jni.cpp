/*  ImageLoaderReturn_jni.cpp
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
#import "Components_jni.h"
#import "com_mousebird_maply_ImageLoaderReturn.h"

using namespace Eigen;
using namespace WhirlyKit;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ImageLoaderReturn_addImageTile
  (JNIEnv *env, jobject obj, jobject imageObj)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		ImageTileClassInfo *tileClassInfo = ImageTileClassInfo::getClassInfo();
		ImageTile_AndroidRef *imageTile = tileClassInfo->getObject(env,imageObj);
		if (!loadReturn || !imageTile)
		    return;
		(*loadReturn)->images.push_back(*imageTile);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageLoaderReturn::addImageTile()");
	}
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_ImageLoaderReturn_getImages(JNIEnv *env, jobject obj)
{
    try
    {
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
		    return NULL;

        jobjectArray ret = env->NewObjectArray((*loadReturn)->images.size(), ImageTileClassInfo::getClassInfo(env,"com/mousebird/maply/ImageTile")->getClass(), NULL);
        for (unsigned int ii=0;ii<(*loadReturn)->images.size();ii++)
        {
            jobject imageTileObj = MakeImageTile(env,std::dynamic_pointer_cast<ImageTile_Android>((*loadReturn)->images[ii]));
            env->SetObjectArrayElement(ret, ii, imageTileObj);
        }

        return ret;
    }
    catch (...)
    {
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageLoaderReturn::getImages()");
    }

    return NULL;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ImageLoaderReturn_clearImages(JNIEnv *env, jobject obj)
{
	try
	{
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
			return;

		(*loadReturn)->images.clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ImageLoaderReturn::clearImages()");
	}
}
