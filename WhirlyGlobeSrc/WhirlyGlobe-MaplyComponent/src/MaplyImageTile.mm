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
<<<<<<< HEAD
#import "UIImage+Stuff.h"
#import "MaplyRawDataWrapper.h"

using namespace WhirlyKit;

// Wrapper for a single image in its various forms
@interface MaplySingleImage : NSObject
@property (nonatomic) UIImage *image;
@property (nonatomic) NSData *data;
@property (nonatomic) Texture *texture;
@end

@implementation MaplySingleImage

- (void)dealloc
{
    if (_texture)
        delete _texture;
}

//+ (MaplySingleImage *)LoadedImageWithUIImage:(UIImage *)image
//{
//    MaplySingleImage *loadImage = [[MaplySingleImage alloc] init];
//    
//    
//    loadImage.imageData = image;
//    CGImageRef cgImage = image.CGImage;
//    loadImage.width = CGImageGetWidth(cgImage);
//    loadImage.height = CGImageGetHeight(cgImage);
//    
//    return loadImage;
//}
//
//+ (MaplySingleImage *)LoadedImageWithPVRTC:(NSData *)imageData size:(int)squareSize
//{
//    MaplySingleImage *loadImage = [[MaplySingleImage alloc] init];
//    loadImage.type = WKLoadedImagePVRTC4;
//    loadImage.borderSize = 0;
//    loadImage.imageData = imageData;
//    loadImage.width = loadImage.height = squareSize;
//    
//    return loadImage;
//}
//
//+ (MaplySingleImage *)LoadedImageWithNSDataAsPNGorJPG:(NSData *)imageData
//{
//    MaplySingleImage *loadImage = [[MaplySingleImage alloc] init];
//    loadImage.type = WKLoadedImageNSDataAsImage;
//    loadImage.borderSize = 0;
//    loadImage.imageData = imageData;
//    loadImage.width = loadImage.height = 0;
//    UIImage *texImage = [UIImage imageWithData:(NSData *)imageData];
//    if (texImage)
//    {
//        loadImage.imageData = texImage;
//        loadImage.width = CGImageGetWidth(texImage.CGImage);
//        loadImage.height = CGImageGetHeight(texImage.CGImage);
//        loadImage.type = WKLoadedImageUIImage;
//    } else
//        return nil;
//    
//    return loadImage;
//}
//
//+ (MaplySingleImage *)PlaceholderImage
//{
//    MaplySingleImage *loadImage = [[MaplySingleImage alloc] init];
//    loadImage.type = WKLoadedImagePlaceholder;
//    
//    return loadImage;
//}

@end

@implementation MaplyImageTile
{
//    loadImage.type = WKLoadedImageUIImage;
//    loadImage.borderSize = 0;
=======

@implementation MaplyImageTile
{
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
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
    
<<<<<<< HEAD
    MaplySingleImage *single = [[MaplySingleImage alloc] init];
    single.data = data;
    stuff = @[single];
=======
    stuff = @[data];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
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
<<<<<<< HEAD
    
    NSMutableArray *newStuff = [NSMutableArray array];
    for (NSData *data in dataArray)
    {
        MaplySingleImage *single = [[MaplySingleImage alloc] init];
        single.data = data;
        [newStuff addObject:single];
    }
    stuff = newStuff;
=======
    stuff = dataArray;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
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
<<<<<<< HEAD
    MaplySingleImage *single = [[MaplySingleImage alloc] init];
    single.image = image;
    stuff = @[single];
    CGImageRef cgImage = image.CGImage;
    _width = CGImageGetWidth(cgImage);
    _height = CGImageGetHeight(cgImage);
=======
    stuff = @[image];
    _width = _height = -1;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    return self;
}

- (id)initWithImageArray:(NSArray *)images
{
    for (UIImage *image in images)
        if (![image isKindOfClass:[UIImage class]])
            return nil;
<<<<<<< HEAD
    if ([images count] == 0)
        return nil;
    
    self = [super init];
    _type = MaplyImgTypeImage;
    NSMutableArray *newStuff = [NSMutableArray array];
    for (UIImage *image in images)
    {
        MaplySingleImage *single = [[MaplySingleImage alloc] init];
        single.image = image;
        [newStuff addObject:single];
    }
    stuff = newStuff;
    CGImageRef cgImage = ((UIImage *)images[0]).CGImage;
    _width = CGImageGetWidth(cgImage);
    _height = CGImageGetHeight(cgImage);
=======
    
    self = [super init];
    _type = MaplyImgTypeImage;
    stuff = images;
    _width = _height = -1;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    return self;
}

- (id)initWithPNGorJPEGData:(NSData *)data
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    
    self = [super init];
<<<<<<< HEAD
    _type = MaplyImgTypeImage;
    MaplySingleImage *single = [[MaplySingleImage alloc] init];
    single.image = [UIImage imageWithData:data];
    stuff = @[single];
    CGImageRef cgImage = single.image.CGImage;
    _width = CGImageGetWidth(cgImage);
    _height = CGImageGetHeight(cgImage);
=======
    _type = MaplyImgTypeData;
    stuff = @[data];
    _width = _height = -1;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
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
<<<<<<< HEAD
    NSMutableArray *newStuff = [NSMutableArray array];
    for (NSData *data in dataArray)
    {
        MaplySingleImage *single = [[MaplySingleImage alloc] init];
        single.image = [UIImage imageWithData:data];
        [newStuff addObject:single];
    }
    stuff = newStuff;
    CGImageRef cgImage = ((MaplySingleImage *)stuff[0]).image.CGImage;
    _width = CGImageGetWidth(cgImage);
    _height = CGImageGetHeight(cgImage);
=======
    stuff = dataArray;
    _width = _height = -1;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
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

<<<<<<< HEAD
- (CGSize)size
{
    return CGSizeMake(_width, _height);
}

=======
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
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

<<<<<<< HEAD
- (void)convertToRaw:(int)borderTexel destWidth:(int)inDestWidth destHeight:(int)inDestHeight
{
    if (_type == MaplyImgTypePlaceholder)
    {
        return;
    }

    // Work through the various layers and convert to raw data
    for (MaplySingleImage *single in stuff)
    {
        switch (_type)
        {
            case MaplyImgTypeData:
                // These are converted to UIImages on initialization.  So it must have failed.
                break;
            case MaplyImgTypeImage:
            {
                int destWidth = (inDestWidth <= 0 ? _width : inDestWidth);
                int destHeight = (inDestHeight <= 0 ? _height : inDestHeight);
                NSData *rawData = [single.image rawDataScaleWidth:destWidth height:destHeight border:borderTexel];
                single.data = rawData;
                single.image = NULL;
            }
                break;
            case MaplyImgTypeRawImage:
=======
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
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
                break;
            default:
                break;
        }
<<<<<<< HEAD
    }
    _type = MaplyImgTypeRawImage;
}

- (WhirlyKit::Texture *)buildTextureFor:(int)which border:(int)reqBorderTexel destWidth:(int)destWidth destHeight:(int)destHeight
{
    if (_type != MaplyImgTypeRawImage)
        [self convertToRaw:reqBorderTexel destWidth:destWidth destHeight:destHeight];
    
    if (_type != MaplyImgTypeRawImage)
        return NULL;
    
    NSData *data = ((MaplySingleImage *)[stuff objectAtIndex:which]).data;
    RawDataRef rawData(new RawNSDataWrapper(data));
    Texture *newTex = new Texture("Tile Quad Loader",rawData,false);
    newTex->setWidth(destWidth);
    newTex->setHeight(destHeight);
    
    return newTex;
}


=======
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

>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
@end
