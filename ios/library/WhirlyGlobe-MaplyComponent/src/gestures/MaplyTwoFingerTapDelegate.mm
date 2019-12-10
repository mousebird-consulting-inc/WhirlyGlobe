/*
 *  MaplyTwoFingerTapDelegate.m
 *
 *
 *  Created by Jesse Crocker on 2/4/14.
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

#import "gestures/MaplyTwoFingerTapDelegate.h"
#import "MaplyZoomGestureDelegate_private.h"
#import "MaplyAnimateTranslation.h"
#import "ViewWrapper.h"

using namespace WhirlyKit;
using namespace Maply;

@implementation MaplyTwoFingerTapDelegate

+ (MaplyTwoFingerTapDelegate *)twoFingerTapDelegateForView:(UIView *)view mapView:(Maply::MapView_iOSRef)mapView
{
    MaplyTwoFingerTapDelegate *tapDelegate = [[MaplyTwoFingerTapDelegate alloc] initWithMapView:mapView];
    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapGesture:)];
    tapRecognizer.numberOfTapsRequired = 1;
    tapRecognizer.numberOfTouchesRequired = 2;
    tapRecognizer.delegate = tapDelegate;
    tapDelegate.gestureRecognizer = tapRecognizer;
    tapDelegate.animTime = 0.1;
	[view addGestureRecognizer:tapRecognizer];
	return tapDelegate;
}

// Called for double tap actions
- (void)tapGesture:(id)sender
{
    UITapGestureRecognizer *tap = sender;
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)tap.view;
    SceneRenderer *sceneRenderer = wrapView.renderer;
	
    Point3d curLoc = self.mapView->getLoc();
    // Just figure out where we tapped
	Point3d hit;
    Eigen::Matrix4d theTransform = self.mapView->calcFullMatrix();
    CGPoint touchLoc = [tap locationInView:tap.view];
    Point2f touchLoc2f(touchLoc.x,touchLoc.y);
    if (self.mapView->pointOnPlaneFromScreen(touchLoc2f, &theTransform, Point2f(sceneRenderer->framebufferWidth/wrapView.contentScaleFactor,sceneRenderer->framebufferHeight/wrapView.contentScaleFactor), &hit, true))
    {
        double newZ = curLoc.z() + (curLoc.z() - self.minZoom)/2.0;
        if (self.minZoom >= self.maxZoom || (self.minZoom < newZ && newZ < self.maxZoom))
        {
            Point3d newLoc(hit.x(),hit.y(),newZ);
            Point3f newLoc3f(newLoc.x(),newLoc.y(),newLoc.z());
            Point3d newCenter;
            Maply::MapView testMapView(*(self.mapView));

            // Check if we're still within bounds
            if (MaplyGestureWithinBounds(bounds,newLoc,sceneRenderer,&testMapView,&newCenter)) {
                Maply::AnimateViewTranslationRef animation = AnimateViewTranslationRef(new AnimateViewTranslation(self.mapView,sceneRenderer,newCenter,_animTime));
                self.mapView->setDelegate(animation);
            }
        }
    } else {
        // Not expecting this case
    }
}

@end
