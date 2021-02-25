/*
 *  MaplyRenderTarget.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/13/17.
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

typedef NS_ENUM(NSUInteger,MaplyMipmapType) {
    /// Don't generate a mipmap
    MaplyMipmapNone,
    /// Generate a basic mipmap
    MaplyMipmapAverage,
    /// Generate a mipmap using Gauss blurring
    MaplyMipmapGauss
};

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
    If we're generating a mipmap for the attached texture of a render target, this controls
        how we do it. The default is none.
 */
@property (nonatomic) MaplyMipmapType mipmapType;

/**
 If set, we'll caclulate the min/max for this render target every frame.
    This is a GPU based calculation for Metal.
 */
@property (nonatomic) bool calculateMinMax;

/**
    Clear the render target to this color every frame.
 
    Default is clear black.
  */
@property (nonatomic,strong) UIColor *clearColor;


/**
    Clear the render target to this value on every frame.
 
    This is for render targets that are not purely color, such as multiple floats.
  */
@property (nonatomic,assign) float clearVal;

/**
    If set, anything rendered to this render target will blend with what's there.
 
    If not set, what's rendered will replace what was there before.
    This is the way it normally works for screen rendering.

    Set to false by default.
  */
@property (nonatomic) bool blend;

/**
 Retrieves a single data value out of the render target.  Size is the number of components * size of components.
 It's best to call this in the snapshot callback.  We know the destination isn't being written to at the moment.
  Metal only.
 */
- (NSData *)getValueAtX:(int)x y:(int)y;

/**
  Returns the whole render target snapshot in the NSData.
 It's best to call this in the snapshot callback.  We know the destination isn't being written to at the moment.
 Metal only.
 */
- (NSData *)getSnapshot;

/**
  Retreives the min/max data values if those are being calculated.
   It's best to call this in the snapshot callback.  We know the destination isn't being written to at the moment.
 Metal only.
 */
- (NSData *)getMinMaxValues;

@end
