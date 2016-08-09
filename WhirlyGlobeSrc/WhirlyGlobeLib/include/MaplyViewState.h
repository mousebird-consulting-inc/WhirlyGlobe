/*
 *  MayerLayerViewWatcher.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/14/12.
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

#import "GlobeView.h"
#import "ViewState.h"
#import "MaplyView.h"

namespace Maply
{
    
/** View State related to the map view.
  */
class MapViewState : public WhirlyKit::ViewState
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    MapViewState(MapView *mapView,WhirlyKit::SceneRendererES *renderer);

	/// Height above globe at this view state
    double heightAboveSurface;

	bool pointOnPlaneFromScreen(WhirlyKit::Point2d pt, Eigen::Matrix4d transform, WhirlyKit::Point2f frameSize, WhirlyKit::Point3d &hit, bool clip);
};

// Generate MapViewState objects as required
class MapViewStateFactory : public WhirlyKit::ViewStateFactory
{
public:
    virtual WhirlyKit::ViewState *makeViewState(WhirlyKit::View *view,WhirlyKit::SceneRendererES *renderer)
    {
        return new Maply::MapViewState((Maply::MapView *)view,renderer);
    }
};
    
}
