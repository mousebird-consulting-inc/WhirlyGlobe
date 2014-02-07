/*
 *  MaplyTwoFingerTapDelegate.m
 *
 *
 *  Created by Jesse Crocker on 2/4/14.
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

#import "MaplyTwoFingerTapDelegate.h"
#import "MaplyZoomGestureDelegate_private.h"

using namespace WhirlyKit;

@implementation MaplyTwoFingerTapDelegate

+ (MaplyTwoFingerTapDelegate *)twoFingerTapDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
    MaplyTwoFingerTapDelegate *tapDelegate = [[MaplyTwoFingerTapDelegate alloc] initWithMapView:mapView];
    UITapGestureRecognizer *tapGecognizer = [[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapGesture:)];
    tapGecognizer.numberOfTapsRequired = 1;
    tapGecognizer.numberOfTouchesRequired = 2;
    tapGecognizer.delegate = tapDelegate;
	[view addGestureRecognizer:tapGecognizer];
	return tapDelegate;
}

// Called for double tap actions
- (void)tapGesture:(id)sender
{
    UITapGestureRecognizer *tap = sender;
    WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)tap.view;
    WhirlyKitSceneRendererES *sceneRenderer = glView.renderer;
	
    Point3d curLoc = mapView.loc;
    // Just figure out where we tapped
	Point3d hit;
    Eigen::Matrix4d theTransform = [mapView calcFullMatrix];
    CGPoint touchLoc = [tap locationInView:tap.view];
    if ([mapView pointOnPlaneFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&hit clip:true])
    {
        double newZ = curLoc.z() + (curLoc.z() - self.minZoom)/2.0;
        if (self.minZoom >= self.maxZoom || (self.minZoom < newZ && newZ < self.maxZoom))
        {
            [mapView setLoc:Point3d(hit.x(),hit.y(),newZ)];
            if (![self withinBounds:mapView.loc view:glView renderer:sceneRenderer])
                [mapView setLoc:curLoc];
        }
    } else {
        // Not expecting this case
    }
}

@end
