/*
 *  MaplyRotateDelegate.mm
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

#import "gestures/MaplyRotateDelegate.h"

using namespace WhirlyKit;
using namespace Maply;

@implementation MaplyRotateDelegate
{
    Maply::RotationType rotType;
    MapView_iOS *mapView;
    double startRotationAngle;
    bool rotationStarted;       // has the threshold been reached yet
    double rotationBaseline;    // the rotation at which the threshold was reached
}

using namespace Maply;

- (id)initWithMapView:(MapView_iOS *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
        rotType = RotNone;
        _rotateThreshold = 0.0f;
        _gestureRecognizer = nil;
        rotationStarted = false;
	}
	
	return self;
}


+ (MaplyRotateDelegate *)rotateDelegateForView:(UIView *)view mapView:(MapView_iOS *)mapView
{
	auto rotateDelegate = [[MaplyRotateDelegate alloc] initWithMapView:mapView];
    rotateDelegate.gestureRecognizer = [[UIRotationGestureRecognizer alloc] initWithTarget:rotateDelegate action:@selector(rotateAction:)];
    rotateDelegate.gestureRecognizer.delegate = rotateDelegate;
    [view addGestureRecognizer:rotateDelegate.gestureRecognizer];
	return rotateDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

- (void)rotateAction:(id)sender
{
    UIRotationGestureRecognizer *rotate = sender;
    
    // Turn off rotation if we fall below two fingers
    if ([rotate numberOfTouches] < 2)
    {
        rotationStarted = false;
        return;
    }
    switch (rotate.state)
    {
        case UIGestureRecognizerStateBegan:
            mapView->cancelAnimation();
            rotType = RotFree;
            startRotationAngle = mapView->getRotAngle();
            break;
        case UIGestureRecognizerStateChanged:
            mapView->cancelAnimation();
            if (rotType == RotFree)
            {
                if (!rotationStarted && std::fabs(rotate.rotation) > DegToRad(_rotateThreshold)) {
                    // They have rotated enough, start rotating the map.
                    rotationStarted = true;
                    // Rotate based on this position, not where they started the gesture.
                    rotationBaseline = rotate.rotation;
                }
                if (rotationStarted) {
                    mapView->setRotAngle(startRotationAngle + rotate.rotation - rotationBaseline, true);
                }
	        }
            break;
        case UIGestureRecognizerStateFailed:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateEnded:
            rotType = RotNone;
            rotationStarted = false;
            break;
        default:
            break;
    }
}

@end
