/*
 *  AnimateViewMomentum.h
 *  WhirlyGlobeApp
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
/** Animate View Momentum is a WhirlyGlobe Animation Delegate
    that will animate from a starting point forward in time with
    an acceleration.  Basically, we use this to simulate momentum.
    We might assign it after a tap and drag is finished.
 */
class AnimateViewMomentum : public GlobeViewAnimationDelegate
{
public:
    AnimateViewMomentum(GlobeViewRef globeView,double velocity,double acceleration,const Eigen::Vector3f &axis,bool northUp);
    
    /// Update the globe view
    virtual void updateView(GlobeView *globeView);
    
    /// Set the velocity while this is running (for auto-rotate)
    void setVelocity(double newVel) { velocity = newVel; }

protected:
    Eigen::Quaterniond rotForTime(GlobeView *globeView,WhirlyKit::TimeInterval sinceStart);
    
    double velocity,acceleration;
    bool northUp;
    Eigen::Quaterniond startQuat;
    Eigen::Vector3d axis;
    double maxTime;
    WhirlyKit::TimeInterval startDate;
};
    
typedef std::shared_ptr<AnimateViewMomentum> AnimateViewMomentumRef;

}
