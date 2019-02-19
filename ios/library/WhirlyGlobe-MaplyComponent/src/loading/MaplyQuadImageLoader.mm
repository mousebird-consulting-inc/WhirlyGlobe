/*
 *  MaplyQuadImageLoader.mm
 *
 *  Created by Steve Gifford on 4/10/18.
 *  Copyright 2012-2018 Saildrone Inc
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

#import "MaplyQuadImageLoader_private.h"
#import "QuadTileBuilder.h"
#import "MaplyImageTile_private.h"
#import "MaplyRenderController_private.h"
#import "MaplyQuadSampler_private.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyRenderTarget_private.h"
#import "MaplyScreenLabel.h"
#import "MaplyRenderTarget_private.h"

using namespace WhirlyKit;

@implementation MaplyQuadImageLoaderBase

- (instancetype)init
{
    self = [super init];
    
    _zBufferRead = false;
    _zBufferWrite = true;
    _baseDrawPriority = kMaplyImageLayerDrawPriorityDefault;
    _drawPriorityPerLevel = 1;
    _color = [UIColor whiteColor];
    _imageFormat = MaplyImageIntRGBA;
    _borderTexel = 0;
    
    MaplyQuadImageLoaderBase * __weak weakSelf = self;
    
    // Start things out after a delay
    // This lets the caller mess with settings
    dispatch_async(dispatch_get_main_queue(), ^{
        [weakSelf delayedInit];
    });

    return self;
}

- (bool)delayedInit
{
    if (!valid)
        return false;
    
    if (!viewC || !viewC->renderControl || !viewC->renderControl->scene)
        return false;
    
    if (!tileFetcher) {
        tileFetcher = [viewC addTileFetcher:MaplyQuadImageLoaderFetcherName];
        loader->tileFetcher = tileFetcher;
    }
    
    samplingLayer = [viewC findSamplingLayer:params forUser:self->loader];
    // Do this again in case they changed them
    loader->setSamplingParams(params->params);
    loader->setFlipY(self.flipY);
    
    // Sort out the texture format
    switch (self.imageFormat) {
        case MaplyImageIntRGBA:
        case MaplyImage4Layer8Bit:
        default:
            loader->setTexType(GL_UNSIGNED_BYTE);
            break;
        case MaplyImageUShort565:
            loader->setTexType(GL_UNSIGNED_SHORT_5_6_5);
            break;
        case MaplyImageUShort4444:
            loader->setTexType(GL_UNSIGNED_SHORT_4_4_4_4);
            break;
        case MaplyImageUShort5551:
            loader->setTexType(GL_UNSIGNED_SHORT_5_5_5_1);
            break;
        case MaplyImageUByteRed:
        case MaplyImageUByteGreen:
        case MaplyImageUByteBlue:
        case MaplyImageUByteAlpha:
        case MaplyImageUByteRGB:
            loader->setTexType(GL_ALPHA);
            break;
    }
    
    if (loader->getShaderID() == EmptyIdentity) {
        MaplyShader *theShader = [viewC getShaderByName:kMaplyShaderDefaultTriMultiTex];
        if (theShader)
            loader->setShaderID([theShader getShaderID]);
    }
    
    return true;
}

- (void)setInterpreter:(NSObject<MaplyLoaderInterpreter> * __nonnull)interp
{
    if (loadInterp) {
        NSLog(@"Caller tried to set loader interpreter after startup in MaplyQuadImageLoader.  Ignoring.");
        return;
    }
    
    loadInterp = interp;
}

- (void)setTileFetcher:(NSObject<MaplyTileFetcher> *)inTileFetcher
{
    tileFetcher = inTileFetcher;
}

- (void)setShader:(MaplyShader *)shader
{
    if (!loader)
        return;
    
    loader->setShaderID([shader getShaderID]);
}

- (void)setRenderTarget:(MaplyRenderTarget *__nonnull)renderTarget
{
    if (!loader)
        return;
    
    loader->setRenderTarget([renderTarget renderTargetID]);
}

@end

@implementation MaplyQuadImageLoader

- (instancetype)initWithParams:(MaplySamplingParams *)params tileInfo:(NSObject<MaplyTileInfoNew> *)tileInfo viewC:(MaplyBaseViewController *)viewC
{
    return nil;
}

// Note: Implement these

- (instancetype)initWithParams:(MaplySamplingParams *)params tileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *)tileInfos viewC:(MaplyBaseViewController *)viewC
{
    return nil;
}

- (void)shutdown
{
}

@end
