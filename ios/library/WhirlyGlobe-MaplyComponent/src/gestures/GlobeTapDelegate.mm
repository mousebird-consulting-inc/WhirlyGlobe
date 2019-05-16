/*
 *  TapDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/3/11.
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

#import "gestures/GlobeTapDelegate.h"
#import "SceneRenderer.h"
#import "GlobeMath.h"
#import "ViewWrapper.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeTapDelegate
{
	GlobeView_iOS *globeView;
}

- (id)initWithGlobeView:(GlobeView_iOS *)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
	}
	
	return self;
}

+ (WhirlyGlobeTapDelegate *)tapDelegateForView:(UIView *)view globeView:(GlobeView_iOS *)globeView
{
	WhirlyGlobeTapDelegate *tapDelegate = [[WhirlyGlobeTapDelegate alloc] initWithGlobeView:globeView];
    
    UITapGestureRecognizer *tapRecog = [[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapAction:)];
    tapDelegate.gestureRecognizer = tapRecog;
	[view addGestureRecognizer:tapRecog];
    
	return tapDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return otherGestureRecognizer.view == gestureRecognizer.view;
}

// Called for a tap
- (void)tapAction:(id)sender
{
	UITapGestureRecognizer *tap = sender;
    
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)tap.view;
	SceneRenderer *sceneRender = wrapView.renderer;
//    WhirlyKit::Scene *scene = sceneRender.scene;
    auto frameSizeScaled = sceneRender->getFramebufferSizeScaled();

	// Translate that to the sphere
	// If we hit, then we'll generate a message
	Point3d hit;
	Eigen::Matrix4d theTransform = globeView->calcFullMatrix();
    CGPoint touchLoc = [tap locationInView:wrapView];
    Point2f touchLoc2f(touchLoc.x,touchLoc.y);
    if (globeView->pointOnSphereFromScreen(touchLoc2f, theTransform, frameSizeScaled, hit, true))
    {
		WhirlyGlobeTapMessage *msg = [[WhirlyGlobeTapMessage alloc] init];
        [msg setTouchLoc:touchLoc];
        [msg setView:wrapView];
		[msg setWorldLocD:hit];
        Point3d localCoord = FakeGeocentricDisplayAdapter::DisplayToLocal(hit);
		[msg setWhereGeo:GeoCoord(localCoord.x(),localCoord.y())];
        msg.heightAboveSurface = globeView->getHeightAboveGlobe();
		
		[[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeTapMsg object:msg]];
    } else {
        WhirlyGlobeTapMessage *msg = [[WhirlyGlobeTapMessage alloc] init];
        [msg setTouchLoc:touchLoc];
        [msg setView:wrapView];
        // If we didn't hit, we generate a different message
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeTapOutsideMsg object:msg]];
    }
}

@end
