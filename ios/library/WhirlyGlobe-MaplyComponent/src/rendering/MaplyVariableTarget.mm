/*
 *  MaplyVariableTarget.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/18/18.
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
 *
 */

#import "MaplyVariableTarget_private.h"
#import "visual_objects/MaplyShape.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyRenderController_private.h"
#import "MaplyBaseInteractionLayer_private.h"

#import <vector>

@implementation MaplyVariableTarget
{
    bool valid;
    double scale;
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    std::vector<MaplyVariableTarget *> auxTargets;
    std::map<int,NSData *> uniBlocks;
    UIColor *_color;
}

/// Initialize with the variable type and view controller
- (instancetype)initWithType:(MaplyQuadImageFormat)type viewC:(NSObject<MaplyRenderControllerProtocol> *)inViewC
{
    self = [super init];
    
    scale = 1.0;
    valid = true;
    viewC = inViewC;
    _type = type;
    _color = [UIColor whiteColor];
    _renderTarget = [[MaplyRenderTarget alloc] init];
    _buildRectangle = true;
    _clearEveryFrame = true;
    _clearVal = 0.0;
    _zBuffer = false;

    dispatch_async(dispatch_get_main_queue(), ^{
        [self delayedSetup];
    });
    
    return self;
}

- (void)setColor:(UIColor *)color
{
    _color = color;
    if (_rectObj)
        [self setupRectangle];
}

- (UIColor *)color
{
    return _color;
}

- (void)addVariableTarget:(MaplyVariableTarget *)target
{
    auxTargets.push_back(target);
    
    if (_rectObj) {
        [self setupRectangle];
    }
}

- (void)setupRectangle
{
    NSObject<MaplyRenderControllerProtocol> *theViewC = viewC;
    
    if (_rectObj) {
        [theViewC removeObjects:@[_rectObj] mode:MaplyThreadCurrent];
        _rectObj = nil;
    }
    
    // Set up a rectangle right over the view to render the render target
    MaplyShapeRectangle *rect = [[MaplyShapeRectangle alloc] init];
    if (!rect)
    {
        return;
    }
    rect.ll = MaplyCoordinate3dDMake(-1.0, -1.0, 0.0);
    rect.ur = MaplyCoordinate3dDMake(1.0, 1.0, 0.0);
    rect.clipCoords = true;
    [rect addTexture:_renderTex];
    for (MaplyVariableTarget *auxTarget : auxTargets) {
        if (auxTarget.renderTex)
            [rect addTexture:auxTarget.renderTex];
        else
            NSLog(@"Failed to add auxiliary render target in setupRectangle for MaplyVariableTarget.");
    }
    
#if MAPLY_MINIMAL
    MaplyShader *shader = _shader ? _shader : [theViewC getShaderByName:kMaplyShaderDefaultTriNoLighting];
    
    WhirlyKit::ShapeInfo shapeInfo;
    shapeInfo.color = [_color asRGBAColor];
    shapeInfo.drawPriority = _drawPriority;
    shapeInfo.zBufferRead = _zBuffer;
    shapeInfo.zBufferWrite = false;
    shapeInfo.programID = [shader getShaderID];
    
    _rectObj = [theViewC addShapes:@[rect] info:shapeInfo desc:nil mode:MaplyThreadCurrent];
#else
    const NSString * const shaderName = _shader ? [_shader name] : kMaplyShaderDefaultTriNoLighting;
    NSDictionary *desc = @{
        kMaplyColor: _color,
        kMaplyDrawPriority: @(_drawPriority),
        kMaplyShader: shaderName,
        kMaplyZBufferRead: @(_zBuffer),
        kMaplyZBufferWrite: @(NO),
        kMaplyDrawableName: self.accessibilityLabel ? self.accessibilityLabel : @"MaplyVariableTarget-Rect",
    };
    _rectObj = [theViewC addShapes:@[rect] desc:desc mode:MaplyThreadCurrent];
#endif
    
    // Pass through the uniform blocks if they've been set up
    if (_rectObj) {
        for (const auto &block : uniBlocks) {
            [theViewC setUniformBlock:block.second
                               buffer:block.first
                           forObjects:@[_rectObj]
                                 mode:MaplyThreadCurrent];
        }
    }
}

- (void)delayedSetup
{
    if (!valid)
        return;

    const auto __strong vc = viewC;
    CGSize screenSize = [vc getFramebufferSize];

    // Can get into a race on framebuffer setup
    if (screenSize.width == 0.0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self delayedSetup];
        });
        return;
    }

    screenSize.width *= scale;
    screenSize.height *= scale;
    _texSize = screenSize;

    // Set up the render target
    NSString *name = self.accessibilityLabel ? self.accessibilityLabel : @"MaplyVariableTarget";
    NSDictionary *desc = @{
        kMaplyTexFormat: @(_type),
        kMaplyDrawableName: name,
    };
    _renderTex = [vc createTexture:desc sizeX:screenSize.width sizeY:screenSize.height mode:MaplyThreadCurrent];
    _renderTarget.texture = _renderTex;
    _renderTarget.clearEveryFrame = _clearEveryFrame;
    _renderTarget.clearVal = _clearVal;
    [vc addRenderTarget:_renderTarget];
    
    if (_buildRectangle) {
        [self setupRectangle];
    }
}

/// Scale the screen by this amount for the render target
- (void)setScale:(double)inScale
{
    scale = inScale;
}

- (void)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID
{
    uniBlocks[bufferID] = uniBlock;
    
    // Pass this through if we're running
    if (_rectObj) {
        [viewC setUniformBlock:uniBlock buffer:bufferID forObjects:@[_rectObj] mode:MaplyThreadCurrent];
    }
}

- (void)clear
{
    const auto vc = viewC;
#if !MAPLY_MINIMAL
    if (_renderTarget && [vc isKindOfClass:[MaplyBaseViewController class]])
        [(MaplyBaseViewController *)vc clearRenderTarget:_renderTarget mode:MaplyThreadCurrent];
#endif //!MAPLY_MINIMAL
}

/// Stop rendering to the target and release everything
- (void)shutdown
{
    valid = false;
    auxTargets.clear();
    
    const auto __strong vc = viewC;
    if (_rectObj) {
        [vc removeObjects:@[_rectObj] mode:MaplyThreadCurrent];
        _rectObj = nil;
    }
    if (_renderTarget) {
        [vc removeRenderTarget:_renderTarget];
#ifndef __clang_analyzer__  // override __nonnull
        _renderTarget = nil;
#endif
    }
    if (_renderTex) {
        [vc removeTextures:@[_renderTex] mode:MaplyThreadCurrent];
        _renderTex = nil;
    }
}

@end
