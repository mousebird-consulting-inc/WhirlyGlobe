/*
 *  MaplyAnimateTranslation.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/20/12.
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

#import "SceneRenderer.h"
#import "MaplyAnimateTranslation.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace Maply {
    
bool MaplyGestureWithinBounds(const Point2dVector &bounds,const Point3d &loc,SceneRenderer *sceneRender,MapView *testMapView,Point3d *newCenter)
{
    if (newCenter)
        *newCenter = loc;
    
    if (bounds.empty())
        return true;
    
    // The corners of the view should be within the bounds
    Point2f corners[4];
    corners[0] = Point2f(0,0);
    corners[1] = Point2f(sceneRender->framebufferWidth, 0.0);
    corners[2] = Point2f(sceneRender->framebufferWidth, sceneRender->framebufferHeight);
    corners[3] = Point2f(0.0, sceneRender->framebufferHeight);
    
    bool isValid = false;
    Point2d locOffset(0,0);
    Point2f frameSize(sceneRender->framebufferWidth,sceneRender->framebufferHeight);
    for (unsigned tests=0;tests<4;tests++)
    {
        Point3d newLoc = loc+Point3d(locOffset.x(),locOffset.y(),0.0);
        testMapView->setLoc(newLoc,false);
        Eigen::Matrix4d fullMatrix = testMapView->calcFullMatrix();
        
        bool checkOkay = true;
        for (unsigned int ii=0;ii<4;ii++)
        {
            Point3d planePt;
            testMapView->pointOnPlaneFromScreen(corners[ii], &fullMatrix, frameSize, &planePt, false);
            if (!PointInPolygon(Point2d(planePt.x(),planePt.y()), bounds))
            {
                Point2d closePt;
                ClosestPointToPolygon(bounds, Point2d(planePt.x(),planePt.y()), &closePt);
                Point2d thisOffset = 1.001 * (closePt - Point2d(planePt.x(),planePt.y()));
                // Try to move around, inward
                locOffset += thisOffset;
                checkOkay = false;
                
                break;
            }
        }
        
        if (checkOkay)
        {
            isValid = true;
            if (newCenter)
                *newCenter = newLoc;
            break;
        }
    }
    
    return isValid;
}
    
AnimateViewTranslation::AnimateViewTranslation(MapViewRef inMapView,WhirlyKit::SceneRenderer *inRenderer,Point3d &newLoc,float howLong)
{
    mapView = inMapView;
    renderer = inRenderer;
    startDate = TimeGetCurrent();
    endDate = startDate + howLong;
    startLoc = mapView->getLoc();
    endLoc = newLoc;
    userMotion = true;
}
    
void AnimateViewTranslation::setBounds(Point2d *inBounds)
{
    bounds.clear();
    for (unsigned int ii=0;ii<4;ii++)
        bounds.push_back(Point2d(inBounds[ii].x(),inBounds[ii].y()));
}

// Bounds check on a single point
bool AnimateViewTranslation::withinBounds(const Point3d &loc,MapView * testMapView,Point3d *newCenter)
{
    return MaplyGestureWithinBounds(bounds,loc,renderer,testMapView,newCenter);
}

void AnimateViewTranslation::updateView(MapView *mapView)
{
    if (startDate == 0.0)
        return;

    TimeInterval now = TimeGetCurrent();
    TimeInterval span = endDate - startDate;
    TimeInterval remain = endDate - now;
    
    Point3d newLoc;
    
    // All done, snap to end
    if (remain < 0)
    {
        newLoc = endLoc;
        startDate = 0;
        endDate = 0;
        mapView->cancelAnimation();
    } else {
        // Interpolate in the middle
        float t = (span-remain)/span;
        Point3d midLoc = startLoc + (endLoc-startLoc)*t;
        newLoc = midLoc;
    }
    
    // Test the prospective point first
    Point3d newCenter;
    MapView testMapView(*mapView);
    if (withinBounds(newLoc, &testMapView, &newCenter)) {
        mapView->setLoc(newCenter);
    }
}

}
