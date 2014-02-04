/*
 *  MaplyPinchDelegateMap.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
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

#import "EAGLView.h"
#import "MaplyPinchDelegate.h"
#import "SceneRendererES.h"
#import "MaplyZoomGestureDelegate_private.h"

using namespace WhirlyKit;

@implementation MaplyPinchDelegate
{
    /// If we're zooming, where we started
    float startZ;
}

+ (MaplyPinchDelegate *)pinchDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
    MaplyPinchDelegate *pinchDelegate = [[MaplyPinchDelegate alloc] initWithMapView:mapView];
    UIPinchGestureRecognizer *pinchRecog = [[UIPinchGestureRecognizer alloc] initWithTarget:pinchDelegate action:@selector(pinchGesture:)];
    pinchRecog.delegate = pinchDelegate;
	[view addGestureRecognizer:pinchRecog];
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
			// Store the starting Z for comparison
			startZ = mapView.loc.z();
            [mapView cancelAnimation];
			break;
		case UIGestureRecognizerStateChanged:
        {
            Point3d curLoc = mapView.loc;
            double newZ = startZ/pinch.scale;
            if (self.minZoom >= self.maxZoom || (self.minZoom < newZ && newZ < self.maxZoom))
            {
                [mapView setLoc:Point3d(curLoc.x(),curLoc.y(),newZ)];
                if (![self withinBounds:mapView.loc view:glView renderer:sceneRenderer])
                    [mapView setLoc:curLoc];
            }
        }
			break;
        default:
            break;
	}
}

@end
