/*
 *  MaplyViewController_private.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012 mousebird consulting
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

#import "MaplyViewController.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyInteractionLayer_private.h"

@interface MaplyViewController()
{
    /// Custom map scene
    Maply::MapScene *mapScene;

@protected
    /// Coordinate system and display adapter
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
    /// Maply view
    MaplyView *mapView;
    // Flat view for 2D mode
    MaplyFlatView * flatView;
    // Scroll view for tethered mode
    UIScrollView * __weak scrollView;

@private    
    /// Our own interaction layer for adding and removing things
    MaplyInteractionLayer *mapInteractLayer;
    
    /// Gesture recognizers
    MaplyTapDelegate *tapDelegate;
    MaplyPanDelegate *panDelegate;
    MaplyPinchDelegate *pinchDelegate;
    MaplyRotateDelegate *rotateDelegate;

    /// Bounding box for the viewer
    MaplyCoordinate boundLL,boundUR;

    /// Current view animation (kept around so it's not released)
    NSObject *curAnimation;
}

- (void)setupFlatView;

@end
