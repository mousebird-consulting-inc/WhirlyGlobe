/*
 *  MaplyTapDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/20/12.
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

#import "gestures/MaplyTapDelegate.h"
#import "SceneRenderer.h"
#import "MaplyView.h"
#import "GlobeMath.h"
#import "ViewWrapper.h"

using namespace WhirlyKit;
using namespace Maply;

@implementation MaplyTapDelegate
{
    MapView_iOS *mapView;
}

- (id)initWithMapView:(MapView_iOS *)inView
{
    self = [super init];
    if (self)
    {
        mapView = inView;
    }
    
    return self;
}

+ (MaplyTapDelegate *)tapDelegateForView:(UIView *)view mapView:(MapView_iOS *)mapView
{
	MaplyTapDelegate *tapDelegate = [[MaplyTapDelegate alloc] initWithMapView:mapView];
    UITapGestureRecognizer *gestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapAction:)];
    tapDelegate.gestureRecognizer = gestureRecognizer;
	[view addGestureRecognizer:gestureRecognizer];
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
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)tap.view;
    SceneRenderer *sceneRender = wrapView.renderer;
    CoordSystemDisplayAdapter *coordAdapter = mapView->getCoordAdapter();
    
    // Just figure out where we tapped
	Point3d hit;
    Eigen::Matrix4d theTransform = mapView->calcFullMatrix();
    CGPoint touchLoc = [tap locationInView:tap.view];
    Point2f touchLoc2f(touchLoc.x,touchLoc.y);
    auto frameSizeScaled = sceneRender->getFramebufferSizeScaled();
    if (mapView->pointOnPlaneFromScreen(touchLoc2f, &theTransform, frameSizeScaled, &hit, true))
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
