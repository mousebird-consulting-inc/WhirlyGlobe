/*
 *  GlobeLayerViewWatcher.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
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
#import "LayerViewWatcher.h"

namespace WhirlyKit
{
    class SceneRendererES;
}

namespace WhirlyGlobe
{
/** View State related to the Globe view.  This adds
    more parameters relating to the globe.
  */
class GlobeViewState : public WhirlyKit::ViewState
{
public:
    GlobeViewState(WhirlyGlobe::GlobeView *globeView,WhirlyKit::SceneRendererES *renderer);
    virtual ~GlobeViewState();

    /// Rotation, etc, at this view state
    Eigen::Quaterniond rotQuat;

    /// Height above globe at this view state
    double heightAboveGlobe;

    /// Return where up (0,0,1) is after model rotation
    Eigen::Vector3d currentUp();

    /** Given a location on the screen and the screen size, figure out where we touched the sphere
     Returns true if we hit and where
     Returns false if not and the closest point on the sphere
     */
    bool pointOnSphereFromScreen(WhirlyKit::Point2f pt,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize,WhirlyKit::Point3d *hit);
};

}

// Note: Porting
///** The Globe Layer View watcher is a subclass of the layer view
//    watcher that takes globe specific parameters into account.
//  */
//@interface WhirlyGlobeLayerViewWatcher : WhirlyKitLayerViewWatcher
//
///// Initialize with the globe view to watch and the layer thread
//- (id)initWithView:(WhirlyGlobe::GlobeView *)view thread:(WhirlyKitLayerThread *)inLayerThread;
//
//@end
