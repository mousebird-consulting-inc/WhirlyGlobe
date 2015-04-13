/*
 *  MaplyUpdateLayer.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/13/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MaplyComponentObject.h"
#import "MaplyViewControllerLayer.h"
#import "MaplyCoordinate.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyTileSource.h"
#import "MaplyBaseViewController.h"

@class MaplyUpdateLayer;

/** @brief An encapsulation of where the viewer is and what they're looking at.
    @details This wraps information about where the viewer is currently looking from and at.
  */
@interface MaplyViewerState : NSObject

/// @brief Position of the viewer
- (MaplyCoordinate3d) eyePos;

@end


/** @brief The update delegate is called if the viewer moves, but not too often.
    @details Use this delegate to generate features around the viewer when they move.  You can control how far they have to move (in display coordinates) and how often you'll receive updates.
  */
@protocol MaplyUpdateDelegate

/** @brief Called when the MaplyUpdateLayer is initialized.
    @details This is called after things are set up.  You'll be on the layer thread here.
  */
- (void)start:(MaplyUpdateLayer *)layer;

/** @brief Called when the viewer moves.
    @details You'll be called on the layer thread when the viewer moves more than your moveDist, subject to calls no more frequent than the minTime.
  */
- (void)viewerMovedTo:(MaplyViewerState *)viewState layer:(MaplyUpdateLayer *)layer;

/** @brief Called when the update layer is shutting down.
    @details Clean up your own data here.
  */
- (void)shutdown:(MaplyUpdateLayer *)layer;

@end

/** @brief This layer will call a delegate as the user moves around, but constrained to distance and time.
    @details This layer is responsible for calling a delegate you provide as the user moves their viewpoint around.  You'll be called if they move from than a certain amount, but not more often than the minimum time.
  */
@interface MaplyUpdateLayer : MaplyViewControllerLayer

/// @brief The minimum distance that will trigger a delegate call.  Distance is in display units (radius of the earth = 1.0).
@property (nonatomic,readonly) double moveDist;

/// @brief The delegate will be called no more often than this amount (in seconds).
@property (nonatomic,readonly) double minTime;

/** @brief Initalize the update layer with a delegate and parameters.
    @param delegate The delegate that will be called every time the user moves, subject to the values.
    @param moveDist The minimum distance that will trigger a delegate call.  Distance is in display units (radius of the earth = 1.0).
    @param minTime The delegate will be called no more often than this amount (in seconds).
  */
- (id)initWithDelegate:(NSObject<MaplyUpdateDelegate> *)delegate moveDist:(double)moveDist minTime:(double)minTime;

@end
