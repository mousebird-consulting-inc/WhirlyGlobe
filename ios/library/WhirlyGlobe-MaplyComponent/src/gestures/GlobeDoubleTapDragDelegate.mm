/*
 *  GlobeDoubleTapDragDelegate.mm
 *
 *
 *  Created by Steve Gifford on 2/7/14.
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
#import "GlobeMath.h"
#import "gestures/GlobeDoubleTapDragDelegate.h"
#import "GlobeView.h"
#import "ViewWrapper.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeDoubleTapDragDelegate
{
    GlobeView_iOS *globeView;
    CGPoint screenPt;
    float startZ;
}

+ (WhirlyGlobeDoubleTapDragDelegate *)doubleTapDragDelegateForView:(UIView *)view globeView:(GlobeView_iOS *)globeView;
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
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)press.view;
    SceneRenderer *sceneRenderer = wrapView.renderer;

	switch (press.state)
    {
        case UIGestureRecognizerStateBegan:
            screenPt = [press locationInView:wrapView];
            startZ = globeView->getHeightAboveGlobe();
            globeView->cancelAnimation();
            [[NSNotificationCenter defaultCenter] postNotificationName:kGlobeDoubleTapDragDidStart object:globeView->tag];
            break;
        case UIGestureRecognizerStateCancelled:
            [[NSNotificationCenter defaultCenter] postNotificationName:kGlobeDoubleTapDragDidEnd object:globeView->tag];
            break;
        case UIGestureRecognizerStateChanged:
        {
            CGPoint curPt = [press locationInView:wrapView];
            float diffY = screenPt.y-curPt.y;
            float height = sceneRenderer->getFramebufferSizeScaled().y();
            float scale = powf(2.0,2*diffY/(height/2));
            float newZ = startZ * scale;
            if (_minZoom < newZ && newZ < _maxZoom)
            {
                globeView->setHeightAboveGlobe(newZ, false);
                if (_tiltDelegate)
                    globeView->setTilt(_tiltDelegate->tiltFromHeight(newZ));
                globeView->runViewUpdates();
            }
        }
            break;
        case UIGestureRecognizerStateEnded:
            [[NSNotificationCenter defaultCenter] postNotificationName:kGlobeDoubleTapDragDidEnd object:globeView->tag];
        break;
        default:
            break;
    }
}

@end
