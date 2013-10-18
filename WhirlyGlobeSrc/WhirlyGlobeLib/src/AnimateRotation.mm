/*
 *  AnimateRotation.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/23/11.
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

#import "AnimateRotation.h"

@implementation AnimateViewRotation


- (id)initWithView:(WhirlyGlobeView *)globeView rot:(Eigen::Quaterniond &)newRot howLong:(float)howLong
{
    if ((self = [super init]))
    {
        _startDate = CFAbsoluteTimeGetCurrent();
        _endDate = CFAbsoluteTimeGetCurrent() + howLong;
        _startRot = [globeView rotQuat];
        _endRot = newRot;
    }
    
    return self;
}


// Called by the view when it's time to update
- (void)updateView:(WhirlyGlobeView *)globeView
{
	if (!self.startDate)
		return;
	
	CFTimeInterval now = CFAbsoluteTimeGetCurrent();
        float span = _endDate-_startDate;
        float remain = _endDate - now;
    
	// All done.  Snap to the end
	if (remain < 0)
	{
		[globeView setRotQuat:_endRot];
        _startDate = 0;
        _endDate = 0;
        [globeView cancelAnimation];
	} else {
		// Interpolate somewhere along the path
		float t = (span-remain)/span;
		[globeView setRotQuat:_startRot.slerp(t,_endRot)];
	}
}

@end
