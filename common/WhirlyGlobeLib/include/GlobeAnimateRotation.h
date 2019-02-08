/*
 *  AnimateRotation.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/23/11.
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
#import "GlobeView.h"

namespace WhirlyGlobe
{

/** Animate View Rotation is WhirlyGlobe Animation Delegate
    that will animate rotation from one point to another over
    time.
 */
class AnimateViewRotation : public GlobeViewAnimationDelegate
{
public:
    /// Kick off a rotate to the given position over the given time
    /// Assign this to the globe view's delegate and it'll do the rest
    AnimateViewRotation(GlobeView *globeView,const Eigen::Quaterniond &newRot,WhirlyKit::TimeInterval howLong);

    /// Update the globe view
    virtual void updateView(GlobeView *globeView);

protected:
    /// When to start the animation.  Can be in the past
    WhirlyKit::TimeInterval startDate;
    /// When to finish the animation.
    WhirlyKit::TimeInterval endDate;
    /// Where to start rotating.  This is probably where you are when you start
    Eigen::Quaterniond startRot;
    /// Where to end the rotation.  We'll interpolate from the start to here
    Eigen::Quaterniond endRot;
};

}
