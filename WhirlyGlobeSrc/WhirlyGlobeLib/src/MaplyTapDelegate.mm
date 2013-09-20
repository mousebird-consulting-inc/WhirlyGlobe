/*
 *  MaplyTapDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/20/12.
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

#import "MaplyTapDelegate.h"
#import "EAGLView.h"
#import "SceneRendererES.h"
#import "MaplyView.h"
#import "GlobeMath.h"

using namespace WhirlyKit;

@implementation MaplyTapDelegate
{
    MaplyView *mapView;
}

- (id)initWithMapView:(MaplyView *)inView
{
    self = [super init];
    if (self)
    {
        mapView = inView;
    }
    
    return self;
}

+ (MaplyTapDelegate *)tapDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
	MaplyTapDelegate *tapDelegate = [[MaplyTapDelegate alloc] initWithMapView:mapView];
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
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;
    CoordSystemDisplayAdapter *coordAdapter = mapView.coordAdapter;
//    WhirlyKit::Scene *scene = sceneRender.scene;
    
    // Just figure out where we tapped
	Point3d hit;
    Eigen::Matrix4d theTransform = [mapView calcFullMatrix];
    CGPoint touchLoc = [tap locationInView:tap.view];
    if ([mapView pointOnPlaneFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor) hit:&hit clip:true])
    {
        MaplyTapMessage *msg = [[MaplyTapMessage alloc] init];
        [msg setTouchLoc:touchLoc];
        [msg setView:tap.view];
		[msg setWorldLoc:Point3f(hit.x(),hit.y(),hit.z())];
        Point3d localPt = coordAdapter->displayToLocal(hit);
		[msg setWhereGeo:coordAdapter->getCoordSystem()->localToGeographic(localPt)];
        msg.heightAboveSurface = hit.z();
		
		[[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:MaplyTapMsg object:msg]];
    } else {
        // Not expecting this case
    }
}

@end
