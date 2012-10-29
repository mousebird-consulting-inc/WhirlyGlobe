/*
 *  AnimateViewMomentum.mm
 *  WhirlyGlobeApp
 *
 *  Created by Steve Gifford on 5/23/11.
 *  Copyright 2011-2012 mousebird consulting
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
#import "AnimateViewMomentum.h"

using namespace Eigen;

@implementation AnimateViewMomentum

@synthesize velocity;
@synthesize acceleration;

- (id)initWithView:(WhirlyGlobeView *)globeView velocity:(float)inVel accel:(float)inAcc axis:(Vector3f)inAxis
{
    if ((self = [super init]))
    {
        velocity = inVel;
        acceleration = inAcc;
        axis = inAxis;
        startQuat = [globeView rotQuat];
        
        startDate = CFAbsoluteTimeGetCurrent();
        
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
    }
    
    return self;
}


// Called by the view when it's time to update
- (void)updateView:(WhirlyGlobeView *)globeView
{
    if (startDate == 0.0)
        return;
    
	float sinceStart = CFAbsoluteTimeGetCurrent()-startDate;
    
    if (sinceStart > maxTime)
    {
        // This will snap us to the end and then we stop
        sinceStart = maxTime;
        startDate = 0;
    }
    
    // Calculate the offset based on angle
    float totalAng = (velocity + 0.5 * acceleration * sinceStart) * sinceStart;
    Eigen::Quaternion<float> diffRot(Eigen::AngleAxisf(totalAng,axis));
    Eigen::Quaternion<float> newQuat;
    newQuat = startQuat * diffRot;    
    [globeView setRotQuat:newQuat];
}

@end
