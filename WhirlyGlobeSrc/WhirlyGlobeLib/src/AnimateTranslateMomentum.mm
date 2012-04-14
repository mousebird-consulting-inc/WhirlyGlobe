/*
 *  AnimateTranslateMomentum.h
 *  WhirlyGlobeApp
 *
 *  Created by Steve Gifford on 1/20/12.
 *  Copyright 2011 mousebird consulting
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

#import "AnimateTranslateMomentum.h"

using namespace Eigen;
using namespace WhirlyKit;

@interface AnimateTranslateMomentum()
@property (nonatomic) NSDate *startDate;
@end

@implementation AnimateTranslateMomentum

@synthesize startDate;

- (id)initWithView:(WhirlyMapView *)mapView velocity:(float)inVel accel:(float)inAcc dir:(Vector3f)inDir
{
    if ((self = [super init]))
    {
        velocity = inVel;
        acceleration = inAcc;
        dir = inDir.normalized();        
        self.startDate = [NSDate date];
        org = mapView.loc;
        
        // Let's calculate the maximum time, so we know when to stop
        if (acceleration != 0.0)
        {
            maxTime = 0.0;
            if (acceleration != 0.0)
                maxTime = -velocity / acceleration;
            maxTime = std::max(0.f,maxTime);
            
            if (maxTime == 0.0)
                self.startDate = nil;
        } else
            maxTime = MAXFLOAT;
    }
    
    return self;
}


// Called by the view when it's time to update
- (void)updateView:(WhirlyMapView *)mapView
{
    if (!self.startDate)
        return;
    
	float sinceStart = -(float)[startDate timeIntervalSinceDate:[NSDate date]];
    
    if (sinceStart > maxTime)
    {
        // This will snap us to the end and then we stop
        sinceStart = maxTime;
        self.startDate = nil;
    }
    
    // Calculate the distance
    float dist = (velocity + 0.5 * acceleration * sinceStart) * sinceStart;
    Point3f newLoc = org + dir * dist;
    mapView.loc = newLoc;
}


@end