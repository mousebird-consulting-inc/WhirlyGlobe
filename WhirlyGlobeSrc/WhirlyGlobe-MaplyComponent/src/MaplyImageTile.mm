/*
 *  MaplyImageTile.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/7/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "MaplyImageTile.h"
#import "MaplyImageTile_private.h"

+ (WhirlyKitLoadedImage *)LoadedImageWithUIImage:(UIImage *)image
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    loadImage.type = WKLoadedImageUIImage;
    loadImage.borderSize = 0;
    loadImage.imageData = image;
    CGImageRef cgImage = image.CGImage;
    loadImage.width = CGImageGetWidth(cgImage);
    loadImage.height = CGImageGetHeight(cgImage);
    
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
    loadImage.type = WKLoadedImageNSDataAsImage;
    loadImage.borderSize = 0;
    loadImage.imageData = imageData;
    loadImage.width = loadImage.height = 0;
    UIImage *texImage = [UIImage imageWithData:(NSData *)imageData];
    if (texImage)
    {
        loadImage.imageData = texImage;
        loadImage.width = CGImageGetWidth(texImage.CGImage);
        loadImage.height = CGImageGetHeight(texImage.CGImage);
        loadImage.type = WKLoadedImageUIImage;
    } else
        return nil;
    
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
            NSData *rawData = [(UIImage *)_imageData rawDataScaleWidth:destWidth height:destHeight border:borderTexel];
            if (rawData)
            {
                _imageData = rawData;
                _type = WKLoadedImageNSDataRawData;
                _width = destWidth;
                _height = destHeight;
            }
        }
            break;
        default:
            return false;
            break;
    }
    
    return true;
}


@implementation MaplyImageTile
{
    int _width,_height;
    int _targetWidth,_targetHeight;
    
    NSArray *stuff;
}

- (id)initAsPlaceholder
{
    self = [super init];
    _type = MaplyImgTypePlaceholder;
    
    return self;
}

- (id)initWithRawImage:(NSData *)data width:(int)width height:(int)height
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    if ([data length] != width*height*4)
        return nil;
    
    self = [super init];
    _type = MaplyImgTypeRawImage;
    
    stuff = @[data];
    _width = width;
    _height = height;
    
    return self;
}

- (id)initWithRawImageArray:(NSArray *)dataArray width:(int)width height:(int)height
{
    for (NSData *data in dataArray)
    {
        if (![data isKindOfClass:[NSData class]])
            return nil;
        if ([data length] != width*height*4)
            return nil;
    }
    
    self = [super init];
    _type = MaplyImgTypeRawImage;
    stuff = dataArray;
    _width = width;
    _height = height;
    
    return self;
}

- (id)initWithImage:(UIImage *)image
{
    if (![image isKindOfClass:[UIImage class]])
        return nil;
    
    self = [super init];
    _type = MaplyImgTypeImage;
    stuff = @[image];
    _width = _height = -1;
    
    return self;
}

- (id)initWithImageArray:(NSArray *)images
{
    for (UIImage *image in images)
        if (![image isKindOfClass:[UIImage class]])
            return nil;
    
    self = [super init];
    _type = MaplyImgTypeImage;
    stuff = images;
    _width = _height = -1;
    
    return self;
}

- (id)initWithPNGorJPEGData:(NSData *)data
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    
    self = [super init];
    _type = MaplyImgTypeData;
    stuff = @[data];
    _width = _height = -1;
    
    return self;
}

- (id)initWithPNGorJPEGDataArray:(NSArray *)dataArray
{
    for (NSData *data in dataArray)
    {
        if (![data isKindOfClass:[NSData class]])
            return nil;
    }
    
    self = [super init];
    _type = MaplyImgTypeData;
    stuff = dataArray;
    _width = _height = -1;
    
    return self;
}

- (void)setTargetSize:(CGSize)targetSize
{
    _targetWidth = targetSize.width;
    _targetHeight = targetSize.height;
}

- (CGSize)targetSize
{
    return CGSizeMake(_targetWidth, _targetHeight);
}

- (id)initWithRandomData:(id)theObj
{
    self = nil;
    
    if (!theObj)
        return self;
    
    if ([theObj isKindOfClass:[MaplyImageTile class]])
        self = theObj;
    else {
        if ([theObj isKindOfClass:[UIImage class]])
            self = [[MaplyImageTile alloc] initWithImage:theObj];
        else if ([theObj isKindOfClass:[NSData class]])
            self = [[MaplyImageTile alloc] initWithPNGorJPEGData:theObj];
        else if ([theObj isKindOfClass:[NSArray class]])
        {
            NSArray *arr = theObj;
            if ([arr count] > 0)
            {
                id firstObj = [arr objectAtIndex:0];
                if ([firstObj isKindOfClass:[UIImage class]])
                    self = [[MaplyImageTile alloc] initWithImageArray:arr];
                else if ([firstObj isKindOfClass:[NSData class]])
                    self = [[MaplyImageTile alloc] initWithPNGorJPEGDataArray:arr];
            }
        }
    }
    
    return self;
}

- (WhirlyKitLoadedTile *)wkTile:(int)borderTexel convertToRaw:(bool)convertToRaw
{
    WhirlyKitLoadedTile *loadTile = [[WhirlyKitLoadedTile alloc] init];
    
    if (_type == MaplyImgTypePlaceholder)
    {
        [loadTile.images addObject:[WhirlyKitLoadedImage PlaceholderImage]];
        return loadTile;
    }

    // Work through the various layers
    for (id thing in stuff)
    {
        WhirlyKitLoadedImage *loadImage = nil;
        switch (_type)
        {
            case MaplyImgTypeImage:
                loadImage = [WhirlyKitLoadedImage LoadedImageWithUIImage:thing];
                break;
            case MaplyImgTypeData:
                loadImage = [WhirlyKitLoadedImage LoadedImageWithNSDataAsPNGorJPG:thing];
                break;
            case MaplyImgTypeRawImage:
                loadImage = [[WhirlyKitLoadedImage alloc] init];
                loadImage.imageData = thing;
                loadImage.width = _width;
                loadImage.height = _height;
                break;
            default:
                break;
        }
        if (!loadImage)
            return nil;
        if (_targetHeight > 0 && _targetWidth > 0)
        {
            loadImage.width = _targetWidth;
            loadImage.height = _targetHeight;
        }

        // This pulls the pixels out of their weird little compressed formats
        // Since we're on our own thread here (probably) this may save time
        if (convertToRaw)
            [loadImage convertToRawData:borderTexel];
        [loadTile.images addObject:loadImage];
    }
    
    return loadTile;
}

@end
