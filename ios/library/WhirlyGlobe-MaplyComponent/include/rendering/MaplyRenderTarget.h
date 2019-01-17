/*
 *  MaplyRenderTarget.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/13/17.
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

#import <UIKit/UIKit.h>
#import "MaplyTexture.h"

/** 
    Represents a render target (other than the screen)
    
    Individual objects can ask to be drawn somewhere other than the screen.
    This is how we do that.
    
    A render target is just a link between a render every frame and a MaplyTexture.  To get at the actual image you use the MaplyTexture.
    
    At the moment a render target can only draw the full screen, possibly at a lower resolution.
  */
@interface MaplyRenderTarget : NSObject

/** 
    The texture we'll draw into.
    
    This is the texture we'll draw into.  Use createTexture to set it up.
  */
@property (nonatomic,strong) MaplyTexture *texture;

/**
    If set, we'll clear the target textures every frame before rendering to it.
 
    If not set, we won't clear the render target texture between frames.
 
    True by default.
  */
@property (nonatomic) bool clearEveryFrame;

/**
    Clear the render target to this color every frame.
 
    Default is clear black.
  */
@property (nonatomic,strong) UIColor *clearColor;

/**
    If set, anything rendered to this render target will blend with what's there.
 
    If not set, what's rendered will replace what was there before.
    This is the way it normally works for screen rendering.

    Set to false by default.
  */
@property (nonatomic) bool blend;

@end
