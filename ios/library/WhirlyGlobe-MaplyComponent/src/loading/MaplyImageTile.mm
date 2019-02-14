/*
 *  MaplyImageTile.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/7/13.
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

#import "MaplyImageTile.h"
#import "MaplyImageTile_private.h"
#import "WhirlyGlobe.h"

@implementation MaplyImageTile

- (instancetype)initWithRawImage:(NSData *)data width:(int)width height:(int)height
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    if ([data length] != width*height*4)
        return nil;
    
    self = [super init];
    _type = MaplyImgTypeRawImage;
    
    imageStuff = data;
    _components = 4;
    _width = width;
    _height = height;
    
    return self;
}

- (instancetype)initWithRawImage:(NSData *)data width:(int)width height:(int)height components:(int)comp
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    if ([data length] != width*height*comp)
        return nil;
    
    self = [super init];
    _type = MaplyImgTypeRawImage;
    
    imageStuff = data;
    _components = comp;
    _width = width;
    _height = height;
    
    return self;
}

- (instancetype)initWithImage:(UIImage *)image
{
    if (![image isKindOfClass:[UIImage class]])
        return nil;
    
    self = [super init];
    _type = MaplyImgTypeImage;
    imageStuff = image;
    _width = _height = -1;
    _components = 4;

    return self;
}

- (instancetype)initWithPNGorJPEGData:(NSData *)data
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    
    self = [super init];
    _type = MaplyImgTypeData;
    imageStuff = data;
    _width = _height = -1;
    _components = 4;

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

- (instancetype)initWithRandomData:(id)theObj
{
    self = nil;
    
    if ([theObj isKindOfClass:[MaplyImageTile class]])
        return theObj;
    
    if (!theObj)
        return self;
    
    if ([theObj isKindOfClass:[UIImage class]])
        self = [[MaplyImageTile alloc] initWithImage:theObj];
    else if ([theObj isKindOfClass:[NSData class]])
        self = [[MaplyImageTile alloc] initWithPNGorJPEGData:theObj];
    
    return self;
}

- (WhirlyKitLoadedTile *)wkTile:(int)borderTexel convertToRaw:(bool)convertToRaw
{
    WhirlyKitLoadedTile *loadTile = [[WhirlyKitLoadedTile alloc] init];
    
    // Work through the various layers
    WhirlyKitLoadedImage *loadImage = nil;
    switch (_type)
    {
        case MaplyImgTypeImage:
            loadImage = [WhirlyKitLoadedImage LoadedImageWithUIImage:imageStuff];
            break;
        case MaplyImgTypeData:
            loadImage = [WhirlyKitLoadedImage LoadedImageWithNSDataAsPNGorJPG:imageStuff];
            break;
        case MaplyImgTypeRawImage:
            loadImage = [[WhirlyKitLoadedImage alloc] init];
            loadImage.imageData = imageStuff;
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
    } else {
        // They have to at least be square
        if (loadImage.width != loadImage.height)
        {
            int maxSize = std::max(loadImage.width,loadImage.height);
            maxSize = WhirlyKit::NextPowOf2(maxSize);
            loadImage.width = maxSize;
            loadImage.height = maxSize;
        }
    }

    // This pulls the pixels out of their weird little compressed formats
    // Since we're on our own thread here (probably) this may save time
    if (convertToRaw)
        [loadImage convertToRawData:borderTexel];
    [loadTile.images addObject:loadImage];
    
    return loadTile;
}

- (NSData *) asNSData
{
    if ([imageStuff isKindOfClass:[NSData class]])
        return imageStuff;

    return nil;
}

@end
