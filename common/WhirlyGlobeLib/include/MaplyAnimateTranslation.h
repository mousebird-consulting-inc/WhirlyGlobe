/*
 *  MaplyAnimateTranslation.h
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

// Bounds check that adjusts the center to try and compensate
bool MaplyGestureWithinBounds(const WhirlyKit::Point2dVector &bounds,const WhirlyKit::Point3d &loc,WhirlyKit::SceneRenderer *sceneRender,MapView *testMapView,WhirlyKit::Point3d *newCenter);

/// Maply translation from one location to another.
class AnimateViewTranslation : public MapViewAnimationDelegate
{
public:
    /// Kick off a translate to the given position over the given time
    /// Assign this to the globe view's delegate and it'll do the rest
    AnimateViewTranslation(MapViewRef mapView,WhirlyKit::SceneRenderer *renderer,WhirlyKit::Point3d &newLoc,float howLong);
    
    /// Set the bounding rectangle
    void setBounds(WhirlyKit::Point2d *bounds);
    
    /// Update the map view
    virtual void updateView(MapView *mapView);

    /// When to start the animation.  Can be in the past
    WhirlyKit::TimeInterval startDate;
    /// When to finish the animation.
    WhirlyKit::TimeInterval endDate;
    /// Where to start the translation.  This is probably where you are when you starting.
    WhirlyKit::Point3d startLoc;
    /// Where to end the translation.  We'll interpolate from the start to here.
    WhirlyKit::Point3d endLoc;
    /// Set if a user kicked this off (true by default)
    bool userMotion;

protected:
    WhirlyKit::SceneRenderer *renderer;
    
    bool withinBounds(const WhirlyKit::Point3d &loc,MapView * testMapView,WhirlyKit::Point3d *newCenter);
    
    /// Boundary quad that we're to stay within
    WhirlyKit::Point2dVector bounds;
    MapViewRef mapView;
};
    
typedef std::shared_ptr<AnimateViewTranslation> AnimateViewTranslationRef;
    
}
