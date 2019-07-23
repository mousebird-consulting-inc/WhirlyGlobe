/*
 *  MaplyVariableTarget.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/18/18.
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

#import <UIKit/UIKit.h>
#import "visual_objects/MaplyTexture.h"
#import "rendering/MaplyRenderTarget.h"
#import "control/MaplyRenderController.h"

typedef NS_ENUM(NSInteger, MaplyVariableType) {
    // Rendering 4 component images to the target
    VariableTypeVisual
};

/**
    A variable target manages two pass rendering for one type of variable.
 
    Set up the variable target
  */
@interface MaplyVariableTarget : NSObject

/// Initialize with the variable type and view controller
- (nonnull instancetype)initWithType:(MaplyVariableType)type viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/// Render target created for this variable target
@property (nonatomic,readonly,strong,nonnull) MaplyRenderTarget *renderTarget;

/// Scale the screen by this amount for the render target
- (void)setScale:(double)scale;

/// Color of the rectangle used to draw the render target
@property (nonatomic,strong,nonnull) UIColor *color;

/// Draw priority of the rectangle we'll use to draw the render target to the screen
@property (nonatomic,assign) int drawPriority;

/// Shader used to draw the render target to the screen.
/// Leave this empty and we'll provide our own
@property (nonatomic,strong,nullable) MaplyShader *shader;

/// By default we'll build a rectangle to display the target
@property (nonatomic,assign) bool buildRectangle;

/// Rectangle created to show the variable target (if that's set)
@property (nonatomic,readonly,nullable) MaplyComponentObject *rectObj;

/// The texture we're rendering to (as part of the render target)
@property (nonatomic,readonly,strong,nullable) MaplyTexture *renderTex;

/// Passing in another variable target will let us assign that target to the
/// rectangle used to render this variable target's data.  This is used if
/// you need the contents of more than one target in a shader.
- (void)addVariableTarget:(MaplyVariableTarget * __nonnull)target;

/// Stop rendering to the target and release everything
- (void)shutdown;

@end
