/*
 *  GlobeTwoFingerTapDelegate.mm
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

#import "gestures/GlobeTwoFingerTapDelegate.h"
#import "GlobeAnimateHeight.h"
#import "ViewWrapper.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeTwoFingerTapDelegate
{
    WhirlyGlobe::GlobeView_iOS *globeView;
}

+ (WhirlyGlobeTwoFingerTapDelegate *)twoFingerTapDelegateForView:(UIView *)view globeView:(GlobeView_iOS *)globeView
{
    WhirlyGlobeTwoFingerTapDelegate *tapDelegate = [[WhirlyGlobeTwoFingerTapDelegate alloc] init];
    tapDelegate->globeView = globeView;
    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapGesture:)];
    tapRecognizer.numberOfTapsRequired = 1;
    tapRecognizer.numberOfTouchesRequired = 2;
    tapRecognizer.delegate = tapDelegate;
    tapDelegate.gestureRecognizer = tapRecognizer;
    tapDelegate.zoomTapFactor = 2.0;
    tapDelegate.zoomAnimationDuration = 0.1;
	[view addGestureRecognizer:tapRecognizer];
	return tapDelegate;
}

// Called for double tap actions
- (void)tapGesture:(id)sender
{
	UITapGestureRecognizer *tap = sender;
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)tap.view;
    SceneRenderer *sceneRenderer = wrapView.renderer;
    auto frameSizeScaled = sceneRenderer->getFramebufferSizeScaled();
	
    // Just figure out where we tapped
	Point3d hit;
    Eigen::Matrix4d theTransform = globeView->calcFullMatrix();
    CGPoint touchLoc = [tap locationInView:tap.view];
    Point2f touchLoc2f(touchLoc.x,touchLoc.y);
    if (globeView->pointOnSphereFromScreen(touchLoc2f, theTransform, frameSizeScaled, hit, true))
    {
        double curH = globeView->getHeightAboveGlobe();
        double newH = curH * _zoomTapFactor;
        newH = std::min(newH,(double)_maxZoom);
        if (_minZoom < newH && newH <= _maxZoom)
        {
            AnimateViewHeight *anim = new AnimateViewHeight(globeView,newH,_zoomAnimationDuration);
            anim->setTiltDelegate(self.tiltDelegate);
            globeView->setDelegate(GlobeViewAnimationDelegateRef(anim));
        }
    }
}

@end
