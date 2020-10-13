/*
 *  MaplyRotateDelegate.h
 *  WhirlyGlobeLib
 *
 *  Created by rghosh0 around 9/26/13.
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
#import "MapView_iOS.h"


namespace Maply
{
    /// The state of our rotation
    typedef enum {RotNone,RotFree} RotationType;
}

@interface MaplyRotateDelegate : NSObject <UIGestureRecognizerDelegate>

//The gesture recognizer
@property (nonatomic,retain) UIGestureRecognizer* gestureRecognizer;

/// The minimum angle (degrees) that must be subtended before rotation begins
@property(nonatomic,assign) float rotateThreshold;

+ (MaplyRotateDelegate *)rotateDelegateForView:(UIView *)view mapView:(Maply::MapView_iOS *)mapView;

@end
