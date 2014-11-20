/*
 *  GlobeDoubleTapDragDelegate.mm
 *
 *
 *  Created by Steve Gifford on 2/7/14.
 *  Copyright 2011-2014 mousebird consulting
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
#import "GlobeMath.h"
#import "GlobeDoubleTapDragDelegate.h"
#import "GlobeView.h"
#import "EAGLView.h"

using namespace WhirlyKit;

@implementation WhirlyGlobeDoubleTapDragDelegate
{
    WhirlyGlobeView * __weak globeView;
    CGPoint screenPt;
    float startZ;
}

+ (WhirlyGlobeDoubleTapDragDelegate *)doubleTapDragDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;
{
    WhirlyGlobeDoubleTapDragDelegate *pressDelegate = [[WhirlyGlobeDoubleTapDragDelegate alloc] init];
    pressDelegate->globeView = globeView;
    UILongPressGestureRecognizer *pressRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:pressDelegate action:@selector(pressGesture:)];
    pressRecognizer.numberOfTapsRequired = 1;
    pressRecognizer.minimumPressDuration = 0.1;
    pressRecognizer.delegate = pressDelegate;
    pressDelegate.gestureRecognizer = pressRecognizer;
	[view addGestureRecognizer:pressRecognizer];
	return pressDelegate;
}

// Nothing else can be running
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return FALSE;
}

// Called for double tap actions
- (void)pressGesture:(id)sender
{
    UILongPressGestureRecognizer *press = sender;
    WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)press.view;
	WhirlyKitSceneRendererES *sceneRenderer = glView.renderer;
    
	switch (press.state)
    {
        case UIGestureRecognizerStateBegan:
            screenPt = [press locationInView:glView];
            startZ = globeView.heightAboveGlobe;
            [globeView cancelAnimation];
            [[NSNotificationCenter defaultCenter] postNotificationName:kGlobeDoubleTapDragDidStart object:globeView];
            break;
        case UIGestureRecognizerStateCancelled:
            [[NSNotificationCenter defaultCenter] postNotificationName:kGlobeDoubleTapDragDidEnd object:globeView];
            break;
        case UIGestureRecognizerStateChanged:
        {
            CGPoint curPt = [press locationInView:glView];
            float diffY = screenPt.y-curPt.y;
            float height = sceneRenderer.framebufferHeight / glView.contentScaleFactor;
            float scale = powf(2.0,2*diffY/(height/2));
            float newZ = startZ * scale;
            if (_minZoom < newZ && newZ < _maxZoom)
            {
                globeView.heightAboveGlobe = newZ;
                if (_tiltDelegate)
                    globeView.tilt = [_tiltDelegate tiltFromHeight:newZ];
            }
        }
            break;
        case UIGestureRecognizerStateEnded:
            [[NSNotificationCenter defaultCenter] postNotificationName:kGlobeDoubleTapDragDidEnd object:globeView];
        break;
        default:
            break;
    }
}

@end
