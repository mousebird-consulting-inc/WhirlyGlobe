/*
 *  ImageTile_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/20/19.
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

#import "ImageTile_Android.h"
#import <android/bitmap.h>

namespace WhirlyKit
{

ImageTile_Android::ImageTile_Android()
: type(MaplyImgTypeNone), tex(NULL)
{
}

ImageTile_Android::~ImageTile_Android()
{
    if (tex)
        delete tex;
    tex = NULL;
}

void ImageTile_Android::clearTexture()
{
    tex = NULL;
}

void ImageTile_Android::setBitmap(JNIEnv *env,jobject bitmapObj)
{
    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, bitmapObj, &info) < 0)
    {
        return;
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Only dealing with 8888 bitmaps in QuadImageTileLayer");
        return;
    }
    // Copy the raw data over to the texture
    void* bitmapPixels;
    if (AndroidBitmap_lockPixels(env, bitmapObj, &bitmapPixels) < 0)
    {
        return;
    }

    if (info.height > 0 && info.width > 0)
    {
        uint32_t* src = (uint32_t*) bitmapPixels;
        rawData = RawDataRef(new MutableRawData(bitmapPixels,info.height*info.width*4));
    }

    type = MaplyImgTypeRawImage;
    borderSize = 0;
    width = info.width; targetWidth = width;
    height = info.height; targetHeight = height;
    components = 4;

    AndroidBitmap_unlockPixels(env, bitmapObj);
}

Texture *ImageTile_Android::buildTexture()
{
    if (tex)
        return tex;

    if (type == MaplyImgTypeNone)
        return NULL;

    int destWidth = targetWidth;
    int destHeight = targetHeight;
    if (destWidth <= 0)
        destWidth = width;
    if (destHeight <= 0)
        destHeight = height;

    // We need this to be square.  Because duh.
    if (destWidth != destHeight)
    {
        int size = std::max(destWidth,destHeight);
        destWidth = destHeight = size;
    }
    switch (type) {
        case MaplyImgTypeNone:
            break;
        case MaplyImgTypeDataPKM:
            tex = new TextureGLES("ImageTile_Android");
            tex->setPKMData(rawData);
            tex->setWidth(destWidth);
            tex->setHeight(destHeight);
            break;
        case MaplyImgTypeDataPVRTC4:
            tex = new TextureGLES("ImageTile_Android", rawData,true);
            tex->setWidth(destWidth);
            tex->setHeight(destHeight);
            break;
        case MaplyImgTypeRawImage:
            tex = new TextureGLES("ImageTile_Android",rawData,false);
            tex->setWidth(destWidth);
            tex->setHeight(destHeight);
            break;
    }

    rawData = RawDataRef(NULL);

    return tex;

}

Texture *ImageTile_Android::prebuildTexture()
{
    if (tex)
        return tex;

    tex = buildTexture();

    return tex;
}

}
