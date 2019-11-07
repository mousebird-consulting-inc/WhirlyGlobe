/*
*  MaplyGlobeRenderController.h
*  WhirlyGlobeComponent
*
*  Created by Steve Gifford on 10/23/10.
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
#import "control/MaplyRenderController.h"
#import "loading/MaplyRemoteTileFetcher.h"

/**
    Animation State used by the WhirlyGlobeViewControllerAnimationDelegate.
    
    You fill out one of these when you're implementing the animation delegate.  Return it and the view controller will set the respective values to match.
  */
@interface WhirlyGlobeViewControllerAnimationState : NSObject

/// Heading is calculated from due north
/// If not set or set to MAXFLOAT, this is ignored
@property (nonatomic) double heading;

/// Height above the globe
@property (nonatomic) double height;

/// Tilt as used in the view controller
/// If not set or set to MAXFLOAT, we calculate tilt the regular way
@property (nonatomic) double tilt;

/// Roll as used in the view controller
@property (nonatomic) double roll;

/// Position to move to on the globe
@property (nonatomic) MaplyCoordinateD pos;

/// If set, this is a point on the screen where pos should be.
/// By default this is (-1,-1) meaning the screen position is just the middle.  Otherwise, this is where the position should wind up on the screen, if it can.
@property (nonatomic) CGPoint screenPos;

/// If set, the globe will be centered at this point on the screen
@property (nonatomic) CGPoint globeCenter;

/**
    Interpolate a new state between the given states A and B.
    
    This does a simple interpolation (lat/lon, not great circle) between the two animation states.
  */
+ (nonnull WhirlyGlobeViewControllerAnimationState *)Interpolate:(double)t from:(WhirlyGlobeViewControllerAnimationState *__nonnull)stateA to:(WhirlyGlobeViewControllerAnimationState *__nonnull)stateB;

@end

/**
   The Globe Render Controller is a standalone renderer for the globe.
      This is separate from the WhirlyGlobeViewController, but performs a similar function for
    offline rendering.
 */
@interface WhirlyGlobeRenderController : MaplyRenderController

/// Initialize with the size of the target rendering buffer
- (instancetype __nullable) initWithSize:(CGSize)screenSize mode:(MaplyRenderType)renderType;

/// Initialize as an offline renderer of a given target size with default renderer (Metal)
- (instancetype __nullable)initWithSize:(CGSize)size;

/** Set this if you're doing frame by frame animation.
    It will move particles along and run any animations you may have going.
 **/
@property (nonatomic,assign) NSTimeInterval currentTime;

/**
    Set the viewing state all at once
    
    This sets the position, tilt, height, screen position and heading all at once.
  */
- (void)setViewState:(WhirlyGlobeViewControllerAnimationState *__nonnull)viewState;

/**
    Make a WhirlyGlobeViewControllerAnimationState object from the current view state.
    
    This returns the current view parameters in a single WhirlyGlobeViewControllerAnimationState.
  */
- (nullable WhirlyGlobeViewControllerAnimationState *)getViewState;

/**
    Takes a snapshot of the current OpenGL view and returns it.
  */
- (UIImage *__nullable)snapshot;

/**
     This version of snapshot just returns the raw NSData from the "screen".
 */
- (NSData *__nullable)snapshotData;


/**
    If set, keep north facing upward on the screen as the user moves around.
    
    Off by default.
  */
@property(nonatomic,assign) bool keepNorthUp;

@end
