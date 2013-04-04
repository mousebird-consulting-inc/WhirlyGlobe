/*
 *  PinchDelegateFixed.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/22/12.
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

#import "PinchDelegateFixed.h"

@implementation WGPinchDelegateFixed
@synthesize minHeight;
@synthesize maxHeight;

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
		startZ = 0.0;
        minHeight = globeView.minHeightAboveGlobe;
        maxHeight = globeView.maxHeightAboveGlobe;
	}
	
	return self;
}

+ (WGPinchDelegateFixed *)pinchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
	WGPinchDelegateFixed *pinchDelegate = [[WGPinchDelegateFixed alloc] initWithGlobeView:globeView];
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
			startZ = globeView.heightAboveGlobe;
			break;
		case UIGestureRecognizerStateChanged:
        {
            float newHeight = startZ/pinch.scale;
            newHeight = std::min(maxHeight,newHeight);
            newHeight = std::max(minHeight,newHeight);
			[globeView setHeightAboveGlobe:newHeight];
        }
			break;
        default:
            break;
	}
}

@end
