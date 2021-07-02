/*
 *  ImageTile_iOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/14/19.
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

#import "ImageTile_iOS.h"
#import "RawData_NSData.h"
#import "UIImage+Stuff.h"
#import "TextureMTL.h"

namespace WhirlyKit
{
    
ImageTile_iOS::ImageTile_iOS(SceneRenderer::Type renderType)
: renderType(renderType), imageStuff(nil), tex(NULL)
{
}

ImageTile_iOS::~ImageTile_iOS()
{
    imageStuff = nil;
}
    
void ImageTile_iOS::clearTexture()
{
    tex = NULL;
}

Texture *ImageTile_iOS::buildTexture()
{
    if (tex)
        return tex;
    
    int destWidth = targetWidth;
    int destHeight = targetHeight;
    if (destWidth <= 0)
        destWidth = width;
    if (destHeight <= 0)
        destHeight = height;

    // negative is okay, we'll figure it out from the image itself
    if (destWidth == 0 || destHeight == 0) {
        wkLogLevel(Error,"ImageTile_iOS 0 width or height: %s",name.c_str());
        return nullptr;
    }
    
    // We need this to be square.  Because duh.
    if (destWidth != destHeight)
    {
        int size = std::max(destWidth,destHeight);
        destWidth = destHeight = size;
    }
    switch (type) {
        case MaplyImgTypeImage:
        {
            UIImage *image = (UIImage *)imageStuff;
            if (!image) {
                return nullptr;
            }
            if (destWidth <= 0) {
                destWidth = (int)(image.size.width * image.scale);
            }
            if (destHeight <= 0) {
                destHeight = (int)(image.size.height * image.scale);
            }
            destWidth = destHeight = std::max(destWidth,destHeight);

            if (NSData *rawData = [(UIImage *)imageStuff rawDataScaleWidth:destWidth height:destHeight border:0])
            {
                tex = new TextureMTL("ImageTile_iOS",std::make_shared<RawNSDataReader>(rawData),false);
                tex->setWidth(destWidth);
                tex->setHeight(destHeight);
            }
            else
            {
                return nullptr;
            }
        }
            break;
        case MaplyImgTypeDataUIKitRecognized:
        {
            UIImage *texImage = [UIImage imageWithData:(NSData *)imageStuff];
            if (!texImage)
                return nullptr;
            if (destWidth <= 0)
                destWidth = (int)CGImageGetWidth(texImage.CGImage);
            if (destHeight <= 0)
                destHeight = (int)CGImageGetHeight(texImage.CGImage);

            if (NSData *rawData = [texImage rawDataScaleWidth:destWidth height:destHeight border:0])
            {
                tex = new TextureMTL("ImageTile_iOS",RawDataRef(new RawNSDataReader(rawData)),false);
                tex->setWidth(destWidth);
                tex->setHeight(destHeight);
            }
            else
            {
                return nullptr;
            }
        }
            break;
        case MaplyImgTypeDataPKM:
            tex = new TextureMTL("ImageTile_iOS");
            tex->setPKMData(RawDataRef(new RawNSDataReader((NSData *)imageStuff)));
            tex->setWidth(destWidth);
            tex->setHeight(destHeight);
            break;
        case MaplyImgTypeDataPVRTC4:
            tex = new TextureMTL("ImageTile_iOS", RawDataRef(new RawNSDataReader((NSData *)imageStuff)),true);
            tex->setWidth(destWidth);
            tex->setHeight(destHeight);
            break;
        case MaplyImgTypeRawImage:
            tex = new TextureMTL("ImageTile_iOS",RawDataRef(new RawNSDataReader((NSData *)imageStuff)),false);
            tex->setWidth(destWidth);
            tex->setHeight(destHeight);
            break;
    }

    return tex;
}
    
Texture *ImageTile_iOS::prebuildTexture()
{
    if (tex)
        return tex;
    
    tex = buildTexture();
    
    return tex;
}
    
}
