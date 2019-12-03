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

#import "loading/MaplyImageTile.h"
#import "MaplyImageTile_private.h"
#import "MaplyRenderController_private.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@implementation MaplyImageTile

- (instancetype)initWithRawImage:(NSData *)data width:(int)width height:(int)height viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    if ([data length] != width*height*4)
        return nil;
    if (![viewC getRenderControl])
        return nil;
    
    self = [super init];
    
    imageTile = ImageTile_iOSRef(new ImageTile_iOS([viewC getRenderControl]->renderType));
    imageTile->type = MaplyImgTypeRawImage;
    imageTile->components = 4;
    imageTile->width = width;
    imageTile->height = height;
    imageTile->borderSize = 0;
    imageTile->imageStuff = data;
    
    return self;
}

- (instancetype)initWithRawImage:(NSData *)data width:(int)width height:(int)height components:(int)comp viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    if ([data length] != width*height*comp)
        return nil;
    
    self = [super init];
    imageTile = ImageTile_iOSRef(new ImageTile_iOS(viewC.getRenderControl->renderType));
    imageTile->type = MaplyImgTypeRawImage;
    imageTile->components = comp;
    imageTile->width = width;
    imageTile->height = height;
    imageTile->borderSize = 0;
    imageTile->imageStuff = data;

    return self;
}

- (instancetype)initWithImage:(UIImage *)image viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (![image isKindOfClass:[UIImage class]])
        return nil;
    
    self = [super init];
    imageTile = ImageTile_iOSRef(new ImageTile_iOS(viewC.getRenderControl->renderType));
    imageTile->type = MaplyImgTypeImage;
    imageTile->components = 4;
    imageTile->width = image.size.width;
    imageTile->height = image.size.height;
    imageTile->borderSize = 0;
    imageTile->imageStuff = image;

    return self;
}

- (instancetype)initWithPNGorJPEGData:(NSData *)data viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (![data isKindOfClass:[NSData class]])
        return nil;
    
    self = [super init];
    imageTile = ImageTile_iOSRef(new ImageTile_iOS(viewC.getRenderControl->renderType));
    imageTile->type = MaplyImgTypeDataUIKitRecognized;
    imageTile->components = 4;
    imageTile->width = -1;
    imageTile->height = -1;
    imageTile->borderSize = 0;
    imageTile->imageStuff = data;

    return self;
}

- (void)setTargetSize:(CGSize)targetSize
{
    if (!imageTile)
        return;
    
    imageTile->width = targetSize.width;
    imageTile->height = targetSize.height;
}

- (void)preprocessTexture
{
    if (!imageTile)
        return;
    
    imageTile->prebuildTexture();
}

@end
