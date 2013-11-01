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

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
		startZ = 0.0;
        _minHeight = globeView.minHeightAboveGlobe;
        _maxHeight = globeView.maxHeightAboveGlobe;
	}
	
	return self;
}

- (void)setMinTilt:(float)inMinTilt maxTilt:(float)inMaxTilt minHeight:(float)inMinHeight maxHeight:(float)inMaxHeight
{
    tiltZoom = true;
    minTilt = inMinTilt;
    maxTilt = inMaxTilt;
    minTiltHeight = inMinHeight;
    maxTiltHeight = inMaxHeight;
}

- (bool)getMinTilt:(float *)retMinTilt maxTilt:(float *)retMaxTilt minHeight:(float *)retMinHeight maxHeight:(float *)retMaxHeight
{
    *retMinTilt = minTilt;
    *retMaxTilt = maxTilt;
    *retMinHeight = minTiltHeight;
    *retMaxHeight = maxTiltHeight;
    
    return tiltZoom;
}

- (void)clearTiltZoom
{
    tiltZoom = false;
}

- (float)calcTilt
{
    float newTilt = 0.0;

    // Now the tilt, if we're in that mode
    if (tiltZoom)
    {
        float newHeight = globeView.heightAboveGlobe;
        if (newHeight <= minTiltHeight)
            newTilt = minTilt;
        else if (newHeight >= maxTiltHeight)
            newTilt = maxTilt;
        else {
            float t = (newHeight-minTiltHeight)/(maxTiltHeight - minTiltHeight);
            if (t != 0.0)
                newTilt = t * (maxTilt - minTilt) + minTilt;
        }
    }
    
    return newTilt;
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
            [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidStart object:globeView];
			break;
		case UIGestureRecognizerStateChanged:
        {
            // First the height
            float newHeight = startZ/pinch.scale;
            newHeight = std::min(_maxHeight,newHeight);
            newHeight = std::max(_minHeight,newHeight);
			[globeView setHeightAboveGlobe:newHeight];
            float newTilt = [self calcTilt];
            [globeView setTilt:newTilt];
        }
			break;
        default:
            break;
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateEnded:
            [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidEnd object:globeView];
            break;
	}
}

@end
