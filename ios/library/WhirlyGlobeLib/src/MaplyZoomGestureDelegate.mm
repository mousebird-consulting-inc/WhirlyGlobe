/*
 *  MaplyZoomGestureDelegate.mm
 *
 *
 *  Created by Jesse Crocker on 2/4/14.
 *  Copyright 2011-2017 mousebird consulting
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

#import "MaplyZoomGestureDelegate.h"
#import "MaplyPanDelegate.h"

#import "EAGLView.h"
#import "SceneRendererES.h"

using namespace WhirlyKit;

@implementation MaplyZoomGestureDelegate

- (instancetype)initWithMapView:(MaplyView *)inView
{
	if ((self = [super init]))
	{
		_mapView = inView;
        _minZoom = _maxZoom = -1.0;
	}
	
	return self;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

- (void)setBounds:(WhirlyKit::Point2d *)inBounds
{
    bounds.clear();
    for (unsigned int ii=0;ii<4;ii++)
        bounds.push_back(inBounds[ii]);
}

// Bounds check on a single point
- (bool)withinBounds:(Point3d &)loc view:(UIView *)view renderer:(WhirlyKitSceneRendererES *)sceneRender mapView:(MaplyView *)testMapView newCenter:(Point3d *)newCenter
{
    return MaplyGestureWithinBounds(bounds,loc,view,sceneRender,testMapView,newCenter);
}

// Called for double tap actions
- (void)tapGesture:(id)sender
{
    UITapGestureRecognizer *tap = sender;
    WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)tap.view;
    WhirlyKitSceneRendererES *sceneRenderer = glView.renderer;
	
    Point3d curLoc = _mapView.loc;
//    NSLog(@"curLoc x:%f y:%f z:%f", curLoc.x(), curLoc.y(), curLoc.z());
    // Just figure out where we tapped
	Point3d hit;
    Eigen::Matrix4d theTransform = [_mapView calcFullMatrix];
    CGPoint touchLoc = [tap locationInView:tap.view];
    if ([_mapView pointOnPlaneFromScreen:touchLoc transform:&theTransform frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&hit clip:true])
    {
        double newZ = curLoc.z() - (curLoc.z() - _minZoom)/2.0;
        Point2d newCenter;
        if (_minZoom >= _maxZoom || (_minZoom < newZ && newZ < _maxZoom))
        {
            MaplyView *testMapView = [[MaplyView alloc] initWithView:_mapView];
            [testMapView setLoc:Point3d(hit.x(),hit.y(),newZ)];
            Point3d newCenter;
            if ([self withinBounds:_mapView.loc view:glView renderer:sceneRenderer mapView:testMapView newCenter:&newCenter])
            {
                [_mapView setLoc:newCenter];
            }
        }
    } else {
        // Not expecting this case
    }
}

@end
