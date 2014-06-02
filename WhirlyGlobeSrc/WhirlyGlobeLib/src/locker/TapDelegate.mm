/*
 *  TapDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/3/11.
 *  Copyright 2011-2013 mousebird consulting
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
#import "SceneRendererES.h"
#import "GlobeMath.h"

using namespace WhirlyKit;

@implementation WhirlyGlobeTapDelegate
{
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/TapDelegate.mm
	WhirlyGlobe::GlobeView *globeView;
=======
	WhirlyGlobeView *globeView;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/TapDelegate.mm
}

- (id)initWithGlobeView:(WhirlyGlobe::GlobeView *)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
	}
	
	return self;
}

+ (WhirlyGlobeTapDelegate *)tapDelegateForView:(UIView *)view globeView:(WhirlyGlobe::GlobeView *)globeView
{
	WhirlyGlobeTapDelegate *tapDelegate = [[WhirlyGlobeTapDelegate alloc] initWithGlobeView:globeView];
    
	[view addGestureRecognizer: [[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapAction:)]];
    
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
    
	WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)tap.view;
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/TapDelegate.mm
	WhirlyKit::SceneRendererES *sceneRender = glView.renderer;
=======
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/TapDelegate.mm
//    WhirlyKit::Scene *scene = sceneRender.scene;

	// Translate that to the sphere
	// If we hit, then we'll generate a message
	Point3d hit;
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/TapDelegate.mm
	Eigen::Matrix4d theTransform = globeView->calcFullMatrix();
    CGPoint touchLoc = [tap locationInView:glView];
    Point2f frameSize = sceneRender->getFramebufferSize();
    if (globeView->pointOnSphereFromScreen(touchLoc,&theTransform,Point2f(frameSize.x()/glView.contentScaleFactor,frameSize.y()/glView.contentScaleFactor),&hit,true))
=======
	Eigen::Matrix4d theTransform = [globeView calcFullMatrix];
    CGPoint touchLoc = [tap locationInView:glView];
    if ([globeView pointOnSphereFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor) hit:&hit normalized:true])
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/TapDelegate.mm
    {
		WhirlyGlobeTapMessage *msg = [[WhirlyGlobeTapMessage alloc] init];
        [msg setTouchLoc:touchLoc];
        [msg setView:glView];
		[msg setWorldLocD:hit];
        Point3d localCoord = FakeGeocentricDisplayAdapter::DisplayToLocal(hit);
		[msg setWhereGeo:GeoCoord(localCoord.x(),localCoord.y())];
        msg.heightAboveSurface = globeView->heightAboveSurface();
		
		[[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeTapMsg object:msg]];
	} else
        // If we didn't hit, we generate a different message
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:WhirlyGlobeTapOutsideMsg object:[NSNull null]]];
}

@end
