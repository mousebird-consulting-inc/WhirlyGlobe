/*
 *  MaplyPinchDelegateMap.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
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

#import "gestures/MaplyPinchDelegate.h"
#import "SceneRenderer.h"
#import "MaplyZoomGestureDelegate_private.h"
#import "MaplyAnimateTranslation.h"
#import "ViewWrapper.h"

using namespace WhirlyKit;
using namespace Maply;

@implementation MaplyPinchDelegate
{
    /// If we're zooming, where we started
    float startZ;
    Point2f startingMidPoint;
    Point3d startingGeoPoint;
}

+ (MaplyPinchDelegate *)pinchDelegateForView:(UIView *)view mapView:(MapView_iOSRef)mapView
{
    MaplyPinchDelegate *pinchDelegate = [[MaplyPinchDelegate alloc] initWithMapView:mapView];
    pinchDelegate.gestureRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:pinchDelegate action:@selector(pinchGesture:)];
    pinchDelegate.gestureRecognizer.delegate = pinchDelegate;
	[view addGestureRecognizer:pinchDelegate.gestureRecognizer];
	return pinchDelegate;
}

// Called for pinch actions
- (void)pinchGesture:(id)sender
{
	UIPinchGestureRecognizer *pinch = sender;
	UIGestureRecognizerState theState = pinch.state;
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)pinch.view;
    SceneRenderer *sceneRenderer = wrapView.renderer;

	switch (theState)
	{
		case UIGestureRecognizerStateBegan:
        {
			// Store the starting Z for comparison
            startZ = self.mapView->getLoc().z();
            
            //calculate center between touches, in screen and map coords
            CGPoint t0 = [pinch locationOfTouch:0 inView:pinch.view];
            CGPoint t1 = [pinch locationOfTouch:1 inView:pinch.view];
            startingMidPoint.x() = (t0.x + t1.x) / 2.0;
            startingMidPoint.y() = (t0.y + t1.y) / 2.0;
            Eigen::Matrix4d modelTrans = self.mapView->calcFullMatrix();
            Point2f frameSize = sceneRenderer->getFramebufferSizeScaled();
            self.mapView->pointOnPlaneFromScreen(startingMidPoint, &modelTrans, frameSize, &startingGeoPoint, true);

            self.mapView->cancelAnimation();
            [[NSNotificationCenter defaultCenter] postNotificationName:kZoomGestureDelegateDidStart object:self.mapView->tag];
        }
			break;
		case UIGestureRecognizerStateChanged:
        {
            Point3d curLoc = self.mapView->getLoc();
            double newZ = startZ/pinch.scale;
            if (self.minZoom >= self.maxZoom || (self.minZoom < newZ && newZ < self.maxZoom))
            {
                MapView testMapView(*(self.mapView));

                Point3d newLoc(curLoc.x(), curLoc.y(), newZ);
                
                testMapView.setLoc(newLoc,false);

                // calculate scalepoint offset in screenspace
                Eigen::Matrix4d modelTrans = testMapView.calcFullMatrix();
                auto frameSizeScaled = sceneRenderer->getFramebufferSizeScaled();
                Point2f currentScalePointScreenLoc = testMapView.pointOnScreenFromPlane(startingGeoPoint, &modelTrans, frameSizeScaled);
                Point2f screenOffset(startingMidPoint.x() - currentScalePointScreenLoc.x(),
                    startingMidPoint.y() - currentScalePointScreenLoc.y());

                //calculate a new map center to maintain scalepoint in place on screen
                Point2f newMapCenterPoint((wrapView.frame.size.width/2.0) - screenOffset.x(),
                    (wrapView.frame.size.height/2.0) - screenOffset.y());
                Point3d newCenterGeoPoint;
                testMapView.pointOnPlaneFromScreen(newMapCenterPoint, &modelTrans, frameSizeScaled, &newLoc, true);
                newLoc.z() = newZ;

                testMapView.setLoc(newLoc, false);
                Point3d newCenter;
                if (MaplyGestureWithinBounds(bounds,newLoc,sceneRenderer,&testMapView,&newCenter))
                {
                    self.mapView->setLoc(newCenter, true);
                }
            }
        }
			break;
        case UIGestureRecognizerStateEnded:
            [[NSNotificationCenter defaultCenter] postNotificationName:kZoomGestureDelegateDidEnd object:self.mapView->tag];
            break;
        default:
            break;
	}
}

@end
