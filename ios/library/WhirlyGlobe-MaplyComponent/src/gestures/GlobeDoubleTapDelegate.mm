/*  GlobeDoubleTapDelegate.mm
 *
 *  Created by Steve Gifford on 2/7/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "GlobeDoubleTapDelegate.h"
#import "GlobeDoubleTapDelegate_private.h"
#import "GlobeAnimateHeight.h"
#import "ViewWrapper.h"
#import "SceneRenderer.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeDoubleTapDelegate
{
    GlobeView_iOS *globeView;
}

+ (WhirlyGlobeDoubleTapDelegate *)doubleTapDelegateForView:(UIView *)view globeView:(GlobeView_iOS *)globeView
{
    WhirlyGlobeDoubleTapDelegate *tapDelegate = [[WhirlyGlobeDoubleTapDelegate alloc] init];
    tapDelegate->globeView = globeView;
    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapGesture:)];
    tapRecognizer.numberOfTapsRequired = 2;
    tapRecognizer.numberOfTouchesRequired = 1;
    tapRecognizer.delegate = tapDelegate;
    tapDelegate.gestureRecognizer = tapRecognizer;
    tapDelegate.zoomTapFactor = 2.0;
    tapDelegate.zoomAnimationDuration = 0.1;
	[view addGestureRecognizer:tapRecognizer];
	return tapDelegate;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return _approveAllGestures;
}

// Called for double tap actions
- (void)tapGesture:(id)sender
{
	UITapGestureRecognizer *tap = sender;
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)tap.view;
    SceneRenderer *sceneRenderer = wrapView.renderer;

    // Just figure out where we tapped
	Point3d hit;
    const Eigen::Matrix4d theTransform = globeView->calcFullMatrix();
    const CGPoint touchLoc = [tap locationInView:tap.view];
    const Point2f touchLoc2f(touchLoc.x,touchLoc.y);
    auto frameSizeScaled = sceneRenderer->getFramebufferSizeScaled();
    if (globeView->pointOnSphereFromScreen(touchLoc2f, theTransform, frameSizeScaled, hit, true))
    {
        const double curH = globeView->getHeightAboveGlobe();
        const double newH = curH / _zoomTapFactor;
        if (_minZoom < newH && newH < _maxZoom)
        {
            auto animate = std::make_shared<AnimateViewHeight>(globeView,newH,_zoomAnimationDuration);
            animate->setTiltDelegate(_tiltDelegate);
            globeView->setDelegate(std::move(animate));
        }
    }
}

@end
