/*
 *  MaplyAnimateTranslateMomentum.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/20/12.
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

#import "MaplyAnimateTranslateMomentum.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyAnimateTranslateMomentum
{
    MaplyView *mapView;
    UIView *glView;
    WhirlyKitSceneRendererES * __weak sceneRenderer;

    float velocity,acceleration;
    Eigen::Vector3d dir;
    float maxTime;
    CFTimeInterval startDate;
    WhirlyKit::Point3d org;
    std::vector<WhirlyKit::Point2f> bounds;
}

- (id)initWithView:(MaplyView *)inMapView velocity:(float)inVel accel:(float)inAcc dir:(Vector3f)inDir bounds:(std::vector<WhirlyKit::Point2f> &)inBounds view:(UIView *)inView renderer:(WhirlyKitSceneRendererES *)inSceneRenderer
{
    if ((self = [super init]))
    {
        velocity = inVel;
        acceleration = inAcc;
        dir = Vector3fToVector3d(inDir.normalized());
        startDate = CFAbsoluteTimeGetCurrent();
        mapView = inMapView;
        org = mapView.loc;
        glView = inView;
        sceneRenderer = inSceneRenderer;
        
        // Let's calculate the maximum time, so we know when to stop
        if (acceleration != 0.0)
        {
            maxTime = 0.0;
            if (acceleration != 0.0)
                maxTime = -velocity / acceleration;
            maxTime = std::max(0.f,maxTime);
            
            if (maxTime == 0.0)
                startDate = 0;
        } else
            maxTime = MAXFLOAT;
        
        bounds = inBounds;
    }
    
    return self;
}

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

// Called by the view when it's time to update
- (void)updateView:(MaplyView *)theMapView
{
    if (startDate == 0.0)
        return;
    
	float sinceStart = CFAbsoluteTimeGetCurrent() - startDate;
    
    if (sinceStart > maxTime)
    {
        // This will snap us to the end and then we stop
        sinceStart = maxTime;
        startDate = 0;
        [mapView cancelAnimation];
    }
    
    // Calculate the distance
    Point3d oldLoc = mapView.loc;
    double dist = (velocity + 0.5 * acceleration * sinceStart) * sinceStart;
    Point3d newLoc = org + dir * dist;
    [theMapView setLoc:newLoc runUpdates:false];

    // We'll do a hard stop if we're not within the bounds
    // Note: We're trying this location out, then backing off if it failed.
    if (![self withinBounds:newLoc view:glView renderer:sceneRenderer])
    {
        // How about if we leave the x alone?
        Point3d testLoc = Point3d(oldLoc.x(),newLoc.y(),newLoc.z());
        [mapView setLoc:testLoc runUpdates:false];
        if (![self withinBounds:testLoc view:glView renderer:sceneRenderer])
        {
            // How about leaving y alone?
            testLoc = Point3d(newLoc.x(),oldLoc.y(),newLoc.z());
            [mapView setLoc:testLoc runUpdates:false];
            if (![self withinBounds:testLoc view:glView renderer:sceneRenderer])
                [mapView setLoc:oldLoc runUpdates:false];
        }
    }
    
    if (![self withinBounds:newLoc view:glView renderer:sceneRenderer])
        [theMapView setLoc:oldLoc runUpdates:false];
    
    [theMapView runViewUpdates];
}


@end
