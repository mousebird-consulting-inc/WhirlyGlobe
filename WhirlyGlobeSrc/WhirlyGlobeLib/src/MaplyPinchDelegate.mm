/*
 *  MaplyPinchDelegateMap.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
 *  Copyright 2011-2015 mousebird consulting
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

#import "EAGLView.h"
#import "MaplyPinchDelegate.h"
#import "SceneRendererES.h"
#import "MaplyZoomGestureDelegate_private.h"

using namespace WhirlyKit;

@implementation MaplyPinchDelegate
{
    /// If we're zooming, where we started
    float startZ;
    CGPoint startingMidPoint;
    Point3d startingGeoPoint;
}

+ (MaplyPinchDelegate *)pinchDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
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
	WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)pinch.view;
	WhirlyKitSceneRendererES *sceneRenderer = glView.renderer;
    
	switch (theState)
	{
		case UIGestureRecognizerStateBegan:
        {
			// Store the starting Z for comparison
            startZ = self.mapView.loc.z();
            
            //calculate center between touches, in screen and map coords
            CGPoint t0 = [pinch locationOfTouch:0 inView:pinch.view];
            CGPoint t1 = [pinch locationOfTouch:1 inView:pinch.view];
            startingMidPoint.x = (t0.x + t1.x) / 2.0;
            startingMidPoint.y = (t0.y + t1.y) / 2.0;
            Eigen::Matrix4d modelTrans = [self.mapView calcFullMatrix];
            [self.mapView pointOnPlaneFromScreen:startingMidPoint
                                       transform:&modelTrans
                                       frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)
                                             hit:&startingGeoPoint
                                            clip:true];
            
            [self.mapView cancelAnimation];
            [[NSNotificationCenter defaultCenter] postNotificationName:kZoomGestureDelegateDidStart object:self.mapView];
        }
			break;
		case UIGestureRecognizerStateChanged:
        {
            Point3d curLoc = self.mapView.loc;
            double newZ = startZ/pinch.scale;
            if (self.minZoom >= self.maxZoom || (self.minZoom < newZ && newZ < self.maxZoom))
            {
                //set new height, but dont update display yet
                Point3d newLoc(curLoc.x(), curLoc.y(), newZ);
                [self.mapView setLoc:newLoc runUpdates:NO];

                //calculatute scalepoint offset in screenspace
                Eigen::Matrix4d modelTrans = [self.mapView calcFullMatrix];
                CGPoint currentScalePointScreenLoc = [self.mapView pointOnScreenFromPlane:startingGeoPoint
                                                                               transform:&modelTrans
                                                                               frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)];
                CGPoint screenOffset = {startingMidPoint.x - currentScalePointScreenLoc.x,
                    startingMidPoint.y - currentScalePointScreenLoc.y};

                //calculate a new map center to maintain scalepoint in place on screen
                CGPoint newMapCenterPoint = {static_cast<CGFloat>((glView.frame.size.width/2.0) - screenOffset.x),
                    static_cast<CGFloat>((glView.frame.size.height/2.0) - screenOffset.y)};
                Point3d newCenterGeoPoint;
                [self.mapView pointOnPlaneFromScreen:newMapCenterPoint
                                           transform:&modelTrans
                                           frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)
                                                 hit:&newLoc
                                                clip:true];
                newLoc.z() = newZ;
                
                [self.mapView setLoc:newLoc runUpdates:YES];

                //Check if we've gone out of bounds, undo changes if needed
                if (![self withinBounds:self.mapView.loc view:glView renderer:sceneRenderer])
                    [self.mapView setLoc:curLoc];
            }
        }
			break;
        case UIGestureRecognizerStateEnded:
            [[NSNotificationCenter defaultCenter] postNotificationName:kZoomGestureDelegateDidEnd object:self.mapView];
            break;
        default:
            break;
	}
}

@end
