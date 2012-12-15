/*
 *  MaplyPinchDelegateMap.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import "MaplyPinchDelegate.h"

using namespace WhirlyKit;

@implementation MaplyPinchDelegate

@synthesize minZoom,maxZoom;

- (id)initWithMapView:(MaplyView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
		startZ = 0.0;
        minZoom = maxZoom = -1.0;
	}
	
	return self;
}

+ (MaplyPinchDelegate *)pinchDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
    MaplyPinchDelegate *pinchDelegate = [[MaplyPinchDelegate alloc] initWithMapView:mapView];
    UIPinchGestureRecognizer *pinchRecog = [[UIPinchGestureRecognizer alloc] initWithTarget:pinchDelegate action:@selector(pinchGesture:)];
    pinchRecog.delegate = pinchDelegate;
	[view addGestureRecognizer:pinchRecog];
	return pinchDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Called for pinch actions
- (void)pinchGesture:(id)sender
{
	UIPinchGestureRecognizer *pinch = sender;
	UIGestureRecognizerState theState = pinch.state;
	
	switch (theState)
	{
		case UIGestureRecognizerStateBegan:
			// Store the starting Z for comparison
			startZ = mapView.loc.z();
            [mapView cancelAnimation];
			break;
		case UIGestureRecognizerStateChanged:
        {
            Point3f curLoc = mapView.loc;
            float newZ = startZ/pinch.scale;
            if (minZoom >= maxZoom || (minZoom < newZ && newZ < maxZoom))
                [mapView setLoc:Point3f(curLoc.x(),curLoc.y(),newZ)];
        }
			break;
        default:
            break;
	}
}

@end
