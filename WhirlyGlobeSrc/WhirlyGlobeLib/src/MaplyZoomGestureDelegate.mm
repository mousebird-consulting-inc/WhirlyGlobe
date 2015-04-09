/*
 *  MaplyZoomGestureDelegate.mm
 *
 *
 *  Created by Jesse Crocker on 2/4/14.
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

#import "MaplyZoomGestureDelegate.h"

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
    
    Eigen::Matrix4d fullMatrix = [_mapView calcFullMatrix];
    
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
        [_mapView pointOnPlaneFromScreen:corners[ii] transform:&fullMatrix
                              frameSize:Point2f(sceneRender.framebufferWidth/view.contentScaleFactor,sceneRender.framebufferHeight/view.contentScaleFactor)
                                    hit:&planePts[ii] clip:false];
        isValid &= PointInPolygon(Point2f(planePts[ii].x(),planePts[ii].y()), bounds);
        //        NSLog(@"plane hit = (%f,%f), isValid = %s",planePts[ii].x(),planePts[ii].y(),(isValid ? "yes" : "no"));
    }
    
    return isValid;
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
        if (_minZoom >= _maxZoom || (_minZoom < newZ && newZ < _maxZoom))
        {
            [_mapView setLoc:Point3d(hit.x(),hit.y(),newZ)];
            if (![self withinBounds:_mapView.loc view:glView renderer:sceneRenderer])
                [_mapView setLoc:curLoc];
        }
    } else {
        // Not expecting this case
    }
}

@end
