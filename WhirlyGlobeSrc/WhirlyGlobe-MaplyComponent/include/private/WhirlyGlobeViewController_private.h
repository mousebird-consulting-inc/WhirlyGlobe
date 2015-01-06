/*
 *  WhirlyGlobeViewController_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 10/26/12.
 *  Copyright 2011-2013 mousebird consulting
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
#import <WhirlyGlobe.h>
#import "WhirlyGlobeViewController.h"
#import "MaplyBaseViewController_private.h"
#import "WGSphericalEarthWithTexGroup_private.h"
#import "GlobeDoubleTapDelegate.h"
#import "GlobeTwoFingerTapDelegate.h"
#import "GlobeDoubleTapDragDelegate.h"
#import "TiltDelegate.h"

/// This is the private interface to WhirlyGlobeViewController.
/// Only pull this in if you're subclassing
@interface WhirlyGlobeViewController()<WhirlyKitViewWatcherDelegate>
{
@public    
    WhirlyGlobe::GlobeScene *globeScene;
    WhirlyGlobeView *globeView;
    
    // Local interaction layer
    WGInteractionLayer *globeInteractLayer;
        
    // Gesture recognizers
    WGPinchDelegateFixed *pinchDelegate;
    PanDelegateFixed *panDelegate;
    TiltDelegate *tiltDelegate;
    WhirlyGlobeTapDelegate *tapDelegate;
    WhirlyGlobeRotateDelegate *rotateDelegate;
    AnimateViewRotation *animateRotation;    
    WhirlyGlobeDoubleTapDelegate *doubleTapDelegate;
    WhirlyGlobeTwoFingerTapDelegate *twoFingerTapDelegate;
    WhirlyGlobeDoubleTapDragDelegate *doubleTapDragDelegate;
    WGStandardTiltDelegate *tiltControlDelegate;

    // Set when we're animating the view point but we know where it's going
    bool knownAnimateEndRot;
    // The quaternion animation end point
    Eigen::Quaterniond animateEndRot;
    
    // Used for the outside animation interface
    NSObject<WhirlyGlobeViewControllerAnimationDelegate> *animationDelegate;
    NSTimeInterval animationDelegateEnd;
    Eigen::Quaterniond startQuat;
    Eigen::Vector3d startUp;
    bool delegateRespondsToViewUpdate;
}

@end
