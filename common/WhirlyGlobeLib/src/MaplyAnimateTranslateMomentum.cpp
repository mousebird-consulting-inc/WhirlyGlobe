/*  MaplyAnimateTranslateMomentum.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/20/12.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "MaplyAnimateTranslation.h"
#import "MaplyAnimateTranslateMomentum.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace Maply {

AnimateTranslateMomentum::AnimateTranslateMomentum(
        const MapViewRef &inMapView,
        float inVel, float inAcc,
        const WhirlyKit::Point3f &inDir,
        const Point2dVector &inBounds,
        SceneRenderer *inSceneRenderer) :
    renderer(inSceneRenderer),
    velocity(inVel),
    acceleration(inAcc)
{
    dir = Vector3fToVector3d(inDir.normalized());
    startDate = TimeGetCurrent();
    org = inMapView->getLoc();

    // Let's calculate the maximum time, so we know when to stop
    if (acceleration != 0.0)
    {
        maxTime = 0.0;
        if (acceleration != 0.0)
        {
            maxTime = std::max(0.0f,-velocity / acceleration);
        }
        if (maxTime == 0.0)
        {
            startDate = 0;
        }
    }

    bounds = inBounds;
}

bool AnimateTranslateMomentum::withinBounds(const Point3d &loc,MapView *testMapView,Point3d *newCenter)
{
    return MaplyGestureWithinBounds(bounds,loc,renderer,testMapView,newCenter);
}

// Called by the view when it's time to update
void AnimateTranslateMomentum::updateView(WhirlyKit::View *view)
{
    auto mapView = (MapView *)view;
    if (startDate == 0.0)
        return;
    
    auto sinceStart = TimeGetCurrent() - startDate;
    
    if (sinceStart >= maxTime)
    {
        // This will snap us to the end and then we stop
        sinceStart = maxTime;
        startDate = 0;
        mapView->cancelAnimation();
    }
    
    // Calculate the distance
    const double dist = (velocity + 0.5 * acceleration * sinceStart) * sinceStart;
    const Point3d newLoc = org + dir * dist;
    mapView->setLoc(newLoc,false);
    
    Point3d newCenter;
    
    MapView testMapView(*mapView);

    // We'll do a hard stop if we're not within the bounds
    // We're trying this location out, then backing off if it failed.
    if (withinBounds(newLoc, &testMapView, &newCenter))
    {
        mapView->setLoc(newCenter,true);
    }
    else
    {
        startDate = 0.0;
    }
}

}
