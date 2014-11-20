/*
 *  GlobeTwoFingerTapDelegate.mm
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

#import "GlobeTwoFingerTapDelegate.h"
#import "EAGLView.h"
#import "GlobeAnimateHeight.h"

using namespace WhirlyKit;

@implementation WhirlyGlobeTwoFingerTapDelegate
{
    WhirlyGlobeAnimateViewHeight *animate;
    WhirlyGlobeView *globeView;
}

+ (WhirlyGlobeTwoFingerTapDelegate *)twoFingerTapDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
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
	WhirlyKitEAGLView *glView = (WhirlyKitEAGLView *)tap.view;
	WhirlyKitSceneRendererES *sceneRenderer = glView.renderer;
	
    // Just figure out where we tapped
	Point3d hit;
    Eigen::Matrix4d theTransform = [globeView calcFullMatrix];
    CGPoint touchLoc = [tap locationInView:tap.view];
    if ([globeView pointOnSphereFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&hit normalized:true])
    {
        double curH = globeView.heightAboveGlobe;
        double newH = curH * _zoomTapFactor;
        if (_minZoom < newH && newH < _maxZoom)
        {
            animate = [[WhirlyGlobeAnimateViewHeight alloc] initWithView:globeView toHeight:newH howLong:_zoomAnimationDuration delegate:self.tiltDelegate];
            globeView.delegate = animate;
        }
    }
}

@end
