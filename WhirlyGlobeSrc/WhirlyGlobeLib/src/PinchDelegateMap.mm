/*
 *  PinchDelegateMap.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
 *  Copyright 2011 mousebird consulting
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

#import "PinchDelegateMap.h"

@implementation WhirlyMapPinchDelegate

- (id)initWithMapView:(WhirlyMapView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
		startZ = 0.0;
	}
	
	return self;
}

+ (WhirlyMapPinchDelegate *)pinchDelegateForView:(UIView *)view mapView:(WhirlyMapView *)mapView
{
    WhirlyMapPinchDelegate *pinchDelegate = [[[WhirlyMapPinchDelegate alloc] initWithMapView:mapView] autorelease];
    UIPinchGestureRecognizer *pinchRecog = [[[UIPinchGestureRecognizer alloc] initWithTarget:pinchDelegate action:@selector(pinchGesture:)] autorelease];
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
			break;
		case UIGestureRecognizerStateChanged:
            mapView.loc.z() = startZ/pinch.scale;
			break;
        default:
            break;
	}
}

@end