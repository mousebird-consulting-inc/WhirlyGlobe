/*
 *  AnimateViewMomentum.mm
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

#import <algorithm>
#import "GlobeAnimateViewMomentum.h"
#import "Platform.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyGlobe
{

AnimateViewMomentum::AnimateViewMomentum(GlobeViewRef globeView,double inVel,double inAcc,const Eigen::Vector3f &inAxis,bool inNorthUp)
{
    velocity = inVel;
    acceleration = inAcc;
    northUp = inNorthUp;
    axis = Vector3d(inAxis.x(),inAxis.y(),inAxis.z());
    startQuat = globeView->getRotQuat();
    
    startDate = TimeGetCurrent();
    
    // Let's calculate the maximum time, so we know when to stop
    if (acceleration != 0.0)
    {
        maxTime = 0.0;
        if (acceleration != 0.0)
            maxTime = -velocity / acceleration;
        maxTime = std::max(0.0,maxTime);
        
        if (maxTime == 0.0)
            startDate = 0;
    } else
        maxTime = MAXFLOAT;
}

Quaterniond AnimateViewMomentum::rotForTime(GlobeView *globeView,TimeInterval sinceStart)
{
    // Calculate the offset based on angle
    float totalAng = (velocity + 0.5 * acceleration * sinceStart) * sinceStart;
    Eigen::Quaterniond diffRot(Eigen::AngleAxisd(totalAng,axis));
    Eigen::Quaterniond newQuat;
    newQuat = startQuat * diffRot;

    if (northUp)
    {
        // We'd like to keep the north pole pointed up
        // So we look at where the north pole is going
        Vector3d northPole = (newQuat * Vector3d(0,0,1)).normalized();
        if (northPole.y() != 0.0)
        {
            // We need to know where up (facing the user) will be
            //  so we can rotate around that
            Vector3d newUp = globeView->prospectiveUp(newQuat);
            
            // Then rotate it back on to the YZ axis
            // This will keep it upward
            float ang = atan(northPole.x()/northPole.y());
            
            // However, the pole might be down now
            // If so, rotate it back up
            if (northPole.y() < 0.0)
                ang += M_PI;
            Eigen::AngleAxisd upRot(ang,newUp);
            
            newQuat = (newQuat * upRot).normalized();
        }
    }

    return newQuat;
}

// Called by the view when it's time to update
void AnimateViewMomentum::updateView(GlobeView *globeView)
{
    if (startDate == 0.0)
        return;
    
	float sinceStart = TimeGetCurrent()-startDate;
    
    if (sinceStart > maxTime)
    {
        // This will snap us to the end and then we stop
        sinceStart = maxTime;
        startDate = 0;
    }
    
    Quaterniond newQuat = rotForTime(globeView,sinceStart);
    globeView->setRotQuat(newQuat);
    
    // Make sure to cancel the animation *after* we set the new rotation (duh)
    if (startDate == 0)
        globeView->cancelAnimation();
}

}
