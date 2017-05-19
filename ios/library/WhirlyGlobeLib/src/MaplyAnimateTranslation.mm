/*
 *  MaplyAnimateTranslation.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/20/12.
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

#import "EAGLView.h"
#import "SceneRendererES.h"
#import "MaplyAnimateTranslation.h"
#import "MaplyPanDelegate.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation MaplyAnimateViewTranslation
{
    /// Boundary quad that we're to stay within
    std::vector<WhirlyKit::Point2d> bounds;
    MaplyView __weak *globeView;
    WhirlyKitEAGLView *glView;
}

- (id)initWithView:(MaplyView *)inGlobeView view:(UIView *)inView translate:(Point3d &)newLoc howLong:(float)howLong
{
    self = [super init];
    
    if (self)
    {
        glView = (WhirlyKitEAGLView  *)inView;
        globeView = inGlobeView;
        _startDate = CFAbsoluteTimeGetCurrent();
        _endDate = _startDate + howLong;
        _startLoc = globeView.loc;
        _endLoc = newLoc;
        _userMotion = true;
    }
    
    return self;
}

- (void)setBounds:(WhirlyKit::Point2d *)inBounds
{
    bounds.clear();
    for (unsigned int ii=0;ii<4;ii++)
        bounds.push_back(Point2d(inBounds[ii].x(),inBounds[ii].y()));
}

// Bounds check on a single point
- (bool)withinBounds:(Point3d &)loc view:(UIView *)view renderer:(WhirlyKitSceneRendererES *)sceneRender mapView:(MaplyView *)testMapView newCenter:(Point3d *)newCenter
{
    return MaplyGestureWithinBounds(bounds,loc,view,sceneRender,testMapView,newCenter);
}

- (void)updateView:(MaplyView *)mapView
{
    if (_startDate == 0.0)
        return;

    CFTimeInterval now = CFAbsoluteTimeGetCurrent();
    float span = _endDate - _startDate;
    float remain = _endDate - now;
    
    Point3d newLoc;
    
    // All done, snap to end
    if (remain < 0)
    {
        newLoc = _endLoc;
        _startDate = 0;
        _endDate = 0;
        [mapView cancelAnimation];
    } else {
        // Interpolate in the middle
        float t = (span-remain)/span;
        Point3d midLoc = _startLoc + (_endLoc-_startLoc)*t;
        newLoc = midLoc;
    }
    
    // Test the prospective point first
    Point3d newCenter;
    MaplyView *testMapView = [[MaplyView alloc] initWithView:mapView];
    if ([self withinBounds:newLoc view:glView renderer:glView.renderer mapView:testMapView newCenter:&newCenter])
    {
        [mapView setLoc:newCenter];
    }
}

@end
