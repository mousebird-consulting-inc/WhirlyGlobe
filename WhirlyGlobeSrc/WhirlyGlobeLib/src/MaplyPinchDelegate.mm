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

using namespace WhirlyKit;

@implementation MaplyPinchDelegate
{
    /// If we're zooming, where we started
    float startZ;
    MaplyView *mapView;
    /// Boundary quad that we're to stay within
    std::vector<WhirlyKit::Point2f> bounds;
}

- (id)initWithMapView:(MaplyView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
		startZ = 0.0;
        _minZoom = _maxZoom = -1.0;
	}
	
	return self;
}

+ (MaplyPinchDelegate *)pinchDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
    MaplyPinchDelegate *pinchDelegate = [[MaplyPinchDelegate alloc] initWithMapView:mapView];
    UIPinchGestureRecognizer *pinchRecog = [[UIPinchGestureRecognizer alloc] initWithTarget:pinchDelegate action:@selector(pinchGesture:)];
    pinchRecog.delegate = pinchDelegate;
	[view addGestureRecognizer:pinchRecog];
	return pinchDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

- (void)setBounds:(WhirlyKit::Point2f *)inBounds
{
    bounds.clear();
    for (unsigned int ii=0;ii<4;ii++)
        bounds.push_back(inBounds[ii]);
}

// Bounds check on a single point
- (bool)withinBounds:(Point3d &)loc view:(UIView *)view renderer:(WhirlyKitSceneRendererES *)sceneRender
{
    if (bounds.empty())
        return true;
    
    Eigen::Matrix4d fullMatrix = [mapView calcFullMatrix];
    
    // The corners of the view should be within the bounds
    CGPoint corners[4];
    corners[0] = CGPointMake(0,0);
    corners[1] = CGPointMake(view.frame.size.width, 0.0);
    corners[2] = CGPointMake(view.frame.size.width, view.frame.size.height);
    corners[3] = CGPointMake(0.0, view.frame.size.height);
    Point3d planePts[4];
    bool isValid = true;
    for (unsigned int ii=0;ii<4;ii++)
    {
        [mapView pointOnPlaneFromScreen:corners[ii] transform:&fullMatrix
                              frameSize:Point2f(sceneRender.framebufferWidth/view.contentScaleFactor,sceneRender.framebufferHeight/view.contentScaleFactor)
                                    hit:&planePts[ii] clip:false];
        isValid &= PointInPolygon(Point2f(planePts[ii].x(),planePts[ii].y()), bounds);
//        NSLog(@"plane hit = (%f,%f), isValid = %s",planePts[ii].x(),planePts[ii].y(),(isValid ? "yes" : "no"));
    }
    
    return isValid;
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
            if (_minZoom >= _maxZoom || (_minZoom < newZ && newZ < _maxZoom))
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
