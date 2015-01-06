/*
 *  TiltDelegate.mm
 *
 *  Created by Stephen Gifford on 1/5/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "TiltDelegate.h"
#import "PinchDelegateFixed.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation TiltDelegate
{
    WhirlyGlobeView * __weak view;
    CGPoint startTouch;
    double startTilt;
    bool active;
    bool turnedOffPinch;
}

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
    if ((self = [super init]))
    {
        view = inView;
    }
    
    return self;
}


+ (TiltDelegate *)tiltDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
    TiltDelegate *tiltDelegate = [[TiltDelegate alloc] initWithGlobeView:globeView];
    UIPanGestureRecognizer *tiltRecog = [[UIPanGestureRecognizer alloc] initWithTarget:tiltDelegate action:@selector(panAction:)];
    tiltRecog.delegate = tiltDelegate;
    tiltDelegate.gestureRecognizer = tiltRecog;
    [view addGestureRecognizer:tiltRecog];
    return tiltDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    // We'll ask the rotation handler to defer to us if we're active
    if ([otherGestureRecognizer.delegate isKindOfClass:[WGPinchDelegateFixed class]] && active)
        return NO;
    
    return TRUE;
}

// Called for pan actions
- (void)panAction:(id)sender
{
    UIPanGestureRecognizer *pan = sender;
    WhirlyKitEAGLView *glView = (WhirlyKitEAGLView *)pan.view;
//    WhirlyKitSceneRendererES *sceneRender = glView.renderer;
    
    if (pan.state == UIGestureRecognizerStateCancelled)
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:kTiltDelegateDidEnd object:view];
        active = false;
        if (turnedOffPinch && !_pinchDelegate.gestureRecognizer.enabled)
            _pinchDelegate.gestureRecognizer.enabled = true;
        return;
    }
    
    // Cancel for more than one finger
    if ([pan numberOfTouches] != 2)
    {
        self.gestureRecognizer.enabled = false;
        self.gestureRecognizer.enabled = true;
        return;
    }
    
    CGSize frameSize = glView.frame.size;
    
    switch (pan.state)
    {
        case UIGestureRecognizerStateBegan:
        {
            [view cancelAnimation];
            startTilt = view.tilt;
            startTouch = [pan locationInView:glView];
            active = true;
            if (_pinchDelegate.gestureRecognizer.enabled)
            {
                turnedOffPinch = true;
                _pinchDelegate.gestureRecognizer.enabled = false;
            }
            
            [[NSNotificationCenter defaultCenter] postNotificationName:kTiltDelegateDidStart object:view];
        }
            break;
        case UIGestureRecognizerStateChanged:
        {
            CGPoint curTouch = [pan locationInView:glView];
            double scale = (curTouch.y-startTouch.y)/frameSize.width;
            double move = scale * M_PI/4;
            
            double newTilt = move + startTilt;
            view.tilt = newTilt;
        }
            break;
        case UIGestureRecognizerStateFailed:
            [[NSNotificationCenter defaultCenter] postNotificationName:kTiltDelegateDidEnd object:view];
            active = false;
            if (turnedOffPinch)
            {
                if (!_pinchDelegate.gestureRecognizer.enabled)
                    _pinchDelegate.gestureRecognizer.enabled = true;
                turnedOffPinch = false;
            }
            break;
        case UIGestureRecognizerStateEnded:
        {
            [[NSNotificationCenter defaultCenter] postNotificationName:kTiltDelegateDidEnd object:view];
            active = false;
            if (turnedOffPinch)
            {
                if (!_pinchDelegate.gestureRecognizer.enabled)
                    _pinchDelegate.gestureRecognizer.enabled = true;
                turnedOffPinch = false;
            }
        }
            break;
        default:
            break;
    }
}

@end
