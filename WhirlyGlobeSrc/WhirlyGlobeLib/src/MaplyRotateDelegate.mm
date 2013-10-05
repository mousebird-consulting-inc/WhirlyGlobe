/*
 *  MaplyRotateDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by rghosh0 around 9/26/13.
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

#import "MaplyRotateDelegate.h"

using namespace WhirlyKit;

@implementation MaplyRotateDelegate
{
    Maply::RotationType rotType;
    MaplyView *mapView;
    double startRotationAngle;
}

using namespace Maply;

- (id)initWithMapView:(MaplyView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
        rotType = RotNone;
	}
	
	return self;
}


+ (MaplyRotateDelegate *)rotateDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
	MaplyRotateDelegate *rotateDelegate = [[MaplyRotateDelegate alloc] initWithMapView:mapView];
	UIRotationGestureRecognizer *rotationRecognizer = [[UIRotationGestureRecognizer alloc] initWithTarget:rotateDelegate action:@selector(rotateAction:)];
    rotationRecognizer.delegate = rotateDelegate;
    [view addGestureRecognizer:rotationRecognizer];
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
        return;
    }
    switch (rotate.state)
    {
        case UIGestureRecognizerStateBegan:
            [mapView cancelAnimation];
            rotType = RotFree;
            startRotationAngle = [mapView rotAngle];
            break;
        case UIGestureRecognizerStateChanged:
            [mapView cancelAnimation];
            if (rotType == RotFree)
            {
                [mapView setRotAngle:(startRotationAngle + rotate.rotation)];
	        }
            break;
        case UIGestureRecognizerStateFailed:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateEnded:
            rotType = RotNone;
            break;
        default:
            break;
    }
}

@end
