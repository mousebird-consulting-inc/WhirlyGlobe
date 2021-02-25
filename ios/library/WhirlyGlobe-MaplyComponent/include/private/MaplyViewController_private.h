/*
 *  MaplyViewController_private.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012-2019 mousebird consulting
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
#import "gestures/MaplyTapDelegate.h"
#import "gestures/MaplyPanDelegate.h"
#import "gestures/MaplyPinchDelegate.h"
#import "gestures/MaplyRotateDelegate.h"
#import "gestures/MaplyDoubleTapDelegate.h"
#import "gestures/MaplyTwoFingerTapDelegate.h"
#import "gestures/MaplyDoubleTapDelegate.h"
#import "gestures/MaplyDoubleTapDragDelegate.h"
#import "gestures/MaplyTouchCancelAnimationDelegate.h"
#import "MapView_iOS.h"

@interface MaplyViewController()
{
@protected
    /// Coordinate system and display adapter
    WhirlyKit::CoordSystemDisplayAdapterRef coordAdapter;
    /// Maply view
    Maply::MapView_iOSRef mapView;

@private    
    /// Our own interaction layer for adding and removing things
    MaplyInteractionLayer *mapInteractLayer;
    
    /// Gesture recognizers
    MaplyTapDelegate *tapDelegate;
    MaplyPanDelegate *panDelegate;
    MaplyPinchDelegate *pinchDelegate;
    MaplyRotateDelegate *rotateDelegate;
    MaplyDoubleTapDelegate *doubleTapDelegate;
    MaplyTwoFingerTapDelegate *twoFingerTapDelegate;
    MaplyDoubleTapDragDelegate *doubleTapDragDelegate;
    MaplyTouchCancelAnimationDelegate *touchDelegate;
  
    /// Bounding box for the viewer
    MaplyCoordinate boundLL,boundUR;

    /// Current view animation (kept around so it's not released)
    Maply::MapViewAnimationDelegateRef curAnimation;
    
    // Used for the outside animation interface
    NSObject<MaplyViewControllerAnimationDelegate> *animationDelegate;
    NSTimeInterval animationDelegateEnd;

    bool delegateRespondsToViewUpdate;
}

@end
