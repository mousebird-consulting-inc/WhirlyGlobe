/*
*  LoadedTile.mm
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 9/19/13.
*  Copyright 2011-2017 mousebird consulting
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

#import "LoadedImage.h"
#import "GlobeMath.h"
#import "GlobeLayerViewWatcher.h"
#import "UIImage+Stuff.h"
#import "DynamicTextureAtlas.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitLoadedTile

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    _images = [NSMutableArray array];
    
    return self;
}

@end

@implementation WhirlyKitLoadedImage

+ (WhirlyKitLoadedImage *)LoadedImageWithUIImage:(UIImage *)image
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    loadImage.type = WKLoadedImageUIImage;
    loadImage.borderSize = 0;
    loadImage.imageData = image;
    CGImageRef cgImage = image.CGImage;
    loadImage.width = (int)CGImageGetWidth(cgImage);
    loadImage.height = (int)CGImageGetHeight(cgImage);
    
    return loadImage;
}

+ (WhirlyKitLoadedImage *)LoadedImageWithPVRTC:(NSData *)imageData size:(int)squareSize
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    loadImage.type = WKLoadedImagePVRTC4;
    loadImage.borderSize = 0;
    loadImage.imageData = imageData;
    loadImage.width = loadImage.height = squareSize;
    
    return loadImage;
}

+ (WhirlyKitLoadedImage *)LoadedImageWithNSDataAsPNGorJPG:(NSData *)imageData
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    
    // Check if it's a PKM
    int dataLen = (int)[imageData length];
    if (dataLen > 3 && !strncmp((char *)[imageData bytes], "PKM", 3))
    {
        loadImage.type = WKLoadedImageNSDataPKM;
        loadImage.borderSize = 0;
        loadImage.width = loadImage.height = sqrtf(dataLen*2-16);
        loadImage.imageData = imageData;
    } else {
        loadImage.type = WKLoadedImageNSDataAsImage;
        loadImage.borderSize = 0;
        loadImage.width = loadImage.height = 0;
        loadImage.imageData = imageData;

        UIImage *texImage = [UIImage imageWithData:(NSData *)imageData];
        if (texImage)
        {
            loadImage.imageData = texImage;
            loadImage.width = (int)CGImageGetWidth(texImage.CGImage);
            loadImage.height = (int)CGImageGetHeight(texImage.CGImage);
            loadImage.type = WKLoadedImageUIImage;
        } else
            return nil;
    }
    
    return loadImage;
}

+ (WhirlyKitLoadedImage *)PlaceholderImage
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    loadImage.type = WKLoadedImagePlaceholder;
    
    return loadImage;
}

- (WhirlyKit::Texture *)textureFromRawData:(NSData *)theData width:(int)theWidth height:(int)theHeight
{
    Texture *newTex = new Texture("Tile Quad Loader",theData,false);
    newTex->setWidth(theWidth);
    newTex->setHeight(theHeight);
    
    return newTex;
}

- (WhirlyKit::Texture *)textureFromPKMData:(NSData *)theData width:(int)theWidth height:(int)theHeight
{
    Texture *newTex = new Texture("LoadedTile");
    newTex->setPKMData(theData);
    newTex->setWidth(theWidth);
    newTex->setHeight(theHeight);
    
    // Note: Check the width/height
    
    return newTex;
}

- (WhirlyKit::Texture *)buildTexture:(int)reqBorderTexel destWidth:(int)destWidth destHeight:(int)destHeight
{
    Texture *newTex = NULL;
    
    switch (_type)
    {
        case WKLoadedImageUIImage:
        {
            destWidth = (destWidth <= 0 ? _width : destWidth);
            destHeight = (destHeight <= 0 ? _height : destHeight);
            NSData *rawData = [(UIImage *)_imageData rawDataScaleWidth:destWidth height:destHeight border:reqBorderTexel];
            newTex = [self textureFromRawData:rawData width:destWidth height:destHeight];
        }
            break;
        case WKLoadedImageNSDataAsImage:
            // These are converted to UIImages on initialization.  So it must have failed.
            break;
        case WKLoadedImageNSDataRawData:
            if ([_imageData isKindOfClass:[NSData class]])
            {
                // Note: This isn't complete
                return [self textureFromRawData:(NSData *)_imageData width:_width height:_height];
            }
            break;
        case WKLoadedImageNSDataPKM:
            if ([_imageData isKindOfClass:[NSData class]])
            {
                return [self textureFromPKMData:(NSData *)_imageData width:_width height:_height];
            }
            break;
        case WKLoadedImagePVRTC4:
            if ([_imageData isKindOfClass:[NSData class]])
            {
                newTex = new Texture("Tile Quad Loader", (NSData *)_imageData,true);
                newTex->setWidth(_width);
                newTex->setHeight(_height);
            }
            break;
        case WKLoadedImagePlaceholder:
        default:
            break;
    }
    
    return newTex;
}

- (bool)convertToRawData:(int)borderTexel
{
    switch (_type)
    {
        case WKLoadedImageUIImage:
        {
            int destWidth = _width;
            int destHeight = _height;
            // We need this to be square.  Because duh.
            if (destWidth != destHeight)
            {
                int size = std::max(destWidth,destHeight);
                destWidth = destHeight = size;
            }
            if ([_imageData isKindOfClass:[NSData class]])
            {
                _type = WKLoadedImageNSDataRawData;
                _width = destWidth;
                _height = destHeight;
            } else {
                NSData *rawData = [(UIImage *)_imageData rawDataScaleWidth:destWidth height:destHeight border:borderTexel];
                if (rawData)
                {
                    _imageData = rawData;
                    _type = WKLoadedImageNSDataRawData;
                    _width = destWidth;
                    _height = destHeight;
                }
            }
        }
            break;
        default:
            return false;
            break;
    }
    
    return true;
}

@end
