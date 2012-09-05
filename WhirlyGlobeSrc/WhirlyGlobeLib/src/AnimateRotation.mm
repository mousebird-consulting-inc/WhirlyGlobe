/*
 *  AnimateRotation.mm
 *  WhirlyGlobeLib
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

#import "AnimateRotation.h"

@implementation AnimateViewRotation

@synthesize startDate,endDate;
@synthesize startRot,endRot;

- (id)initWithView:(WhirlyGlobeView *)globeView rot:(Eigen::Quaternion<float> &)newRot howLong:(float)howLong
{
    if ((self = [super init]))
    {
        startDate = CFAbsoluteTimeGetCurrent();
        endDate = CFAbsoluteTimeGetCurrent() + howLong;
        startRot = [globeView rotQuat];
        endRot = newRot;
    }
    
    return self;
}


// Called by the view when it's time to update
- (void)updateView:(WhirlyGlobeView *)globeView
{
	if (!self.startDate)
		return;
	
	CFTimeInterval now = CFAbsoluteTimeGetCurrent();
    float span = endDate-startDate;
    float remain = endDate - now;
    
	// All done.  Snap to the end
	if (remain < 0)
	{
		[globeView setRotQuat:endRot];
        startDate = 0;
        endDate = 0;
	} else {
		// Interpolate somewhere along the path
		float t = (span-remain)/span;
		[globeView setRotQuat:startRot.slerp(t,endRot)];
	}
}

@end
