/*  ImageLoaderReturn_jni.cpp
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
#import "Components_jni.h"
#import "com_mousebird_maply_RawPNGImageLoaderInterpreter.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit {

// Stores value mappings for raw PNGs
struct RawPNGImage {
	std::vector<int> valueMap;
};

}

typedef JavaClassInfo<WhirlyKit::RawPNGImage> RawPNGImageClassInfo;
template<> RawPNGImageClassInfo *RawPNGImageClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RawPNGImageLoaderInterpreter_nativeInit
  (JNIEnv *env, jclass cls)
{
	RawPNGImageClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RawPNGImageLoaderInterpreter_initialise
  (JNIEnv *env, jobject obj)
{
	try
	{
		RawPNGImage *rawImage = new RawPNGImage();
		RawPNGImageClassInfo::getClassInfo()->setHandle(env,obj,rawImage);
	}
	MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RawPNGImageLoaderInterpreter_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		RawPNGImageClassInfo *classInfo = RawPNGImageClassInfo::getClassInfo();
		std::lock_guard<std::mutex> lock(disposeMutex);
		RawPNGImage *inst = classInfo->getObject(env,obj);
		delete inst;
		classInfo->clearHandle(env,obj);
	}
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RawPNGImageLoaderInterpreter_dataForTileNative
  (JNIEnv *env, jobject obj, jbyteArray inImage,jobject loadReturnObj)
{
	try
	{
		RawPNGImage *rawImage = RawPNGImageClassInfo::getClassInfo()->getObject(env,obj);
		QuadLoaderReturnRef *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,loadReturnObj);
		if (!rawImage || !loadReturn)
			return;

		jbyte *bytes = env->GetByteArrayElements(inImage,NULL);
		jsize len = env->GetArrayLength(inImage);

        unsigned int width=0,height=0;
        unsigned int err = 0;
		unsigned int depth = 0;
		unsigned int components = 0;
        unsigned char *outData = RawPNGImageLoaderInterpreter(width,height,
                (const unsigned char *)bytes,len,
		        &rawImage->valueMap[0],
				&depth,
                &components,
				&err,
				/*errStr=*/nullptr);
		const int byteWidth = components * depth / 8;

		env->ReleaseByteArrayElements(inImage,bytes, 0);

		if (err != 0 && !outData)
		{
            wkLogLevel(Warn, "Failed to read PNG in MaplyRawPNGImageLoaderInterpreter for tile %d: (%d,%d)",
					   (*loadReturn)->ident.level,(*loadReturn)->ident.x,(*loadReturn)->ident.y);
        }
		else
		{
			auto wrap = std::make_shared<RawDataWrapper>(outData,width*height*byteWidth,true);
			auto imgTile = std::make_shared<ImageTile_Android>("Raw PNG", std::move(wrap));
			imgTile->width = width;
			imgTile->height = height;
			imgTile->depth = depth;
			imgTile->components = components;
			(*loadReturn)->images.push_back(std::move(imgTile));
        }
    }
	MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_RawPNGImageLoaderInterpreter_addMappingFrom
  (JNIEnv *env, jobject obj, jint inVal, jint outVal)
{
	try
	{
		RawPNGImage *rawImage = RawPNGImageClassInfo::getClassInfo()->getObject(env,obj);
		if (!rawImage)
			return;

		if (rawImage->valueMap.empty())
			rawImage->valueMap.resize(256,-1);
		if (inVal < 256)
			rawImage->valueMap[inVal] = outVal;
	}
	MAPLY_STD_JNI_CATCH()
}
