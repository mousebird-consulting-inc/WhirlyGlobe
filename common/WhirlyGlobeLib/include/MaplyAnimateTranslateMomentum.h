/*
 *  MaplyAnimateTranslateMomentum.h
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

#import "WhirlyTypes.h"
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "MaplyView.h"
#import "SceneRenderer.h"

namespace Maply {

/** Animate Translate Momentum is a Maply animation delegate
    that will animate from a starting point forward in time with
    an acceleration.  We use this to simulate momentum.  Giving it
    a negative acceleration will slow it down.
  */
class AnimateTranslateMomentum : public MapViewAnimationDelegate
{
public:
    /// Initialize with a velocity and negative acceleration (to slow down)
    AnimateTranslateMomentum(MapView *inMapView,
                             float inVel,float inAcc,const WhirlyKit::Point3f &inDir,
                             const WhirlyKit::Point2dVector &inBounds,
                             WhirlyKit::SceneRenderer *inSceneRenderer);

    /// Update the map view
    virtual void updateView(MapView *mapView);

    /// Set if a user kicked this off (true by default)
    bool userMotion;
    
protected:
    bool withinBounds(const WhirlyKit::Point3d &loc,MapView * testMapView,WhirlyKit::Point3d *newCenter);

    MapView *mapView;
    WhirlyKit::SceneRenderer *renderer;
    
    float velocity,acceleration;
    Eigen::Vector3d dir;
    float maxTime;
    WhirlyKit::TimeInterval startDate;
    WhirlyKit::Point3d org;
    WhirlyKit::Point2dVector bounds;
};

}
