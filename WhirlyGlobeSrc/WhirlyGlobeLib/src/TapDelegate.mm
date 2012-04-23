/*
 *  TapDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/3/11.
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

#import "TapDelegate.h"
#import "EAGLView.h"
#import "SceneRendererES1.h"
#import "GlobeMath.h"

using namespace WhirlyKit;

@implementation WhirlyGlobeTapDelegate

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
	}
	
	return self;
}

+ (WhirlyGlobeTapDelegate *)tapDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
	WhirlyGlobeTapDelegate *tapDelegate = [[WhirlyGlobeTapDelegate alloc] initWithGlobeView:globeView];
	[view addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapAction:)]];
	return tapDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Called for a tap
- (void)tapAction:(id)sender
{
	UITapGestureRecognizer *tap = sender;
	WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)tap.view;
	WhirlyKitSceneRendererES1 *sceneRender = glView.renderer;
//    WhirlyKit::Scene *scene = sceneRender.scene;

	// Translate that to the sphere
	// If we hit, then we'll generate a message
	Point3f hit;
	Eigen::Affine3f theTransform = [globeView calcModelMatrix];
    CGPoint touchLoc = [tap locationOfTouch:0 inView:glView];
    if ([globeView pointOnSphereFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor) hit:&hit])
    {
		WhirlyGlobeTapMessage *msg = [[WhirlyGlobeTapMessage alloc] init];
        [msg setTouchLoc:touchLoc];
        [msg setView:glView];
		[msg setWorldLoc:hit];
        Point3f localCoord = GeoCoordSystem::GeocentricishToLocal(hit);
		[msg setWhereGeo:GeoCoord(localCoord.x(),localCoord.y())];
        msg.heightAboveSurface = globeView.heightAboveGlobe;
		
		[[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeTapMsg object:msg]];
	} else
        // If we didn't hit, we generate a different message
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeTapOutsideMsg object:[NSNull null]]];
}

@end
