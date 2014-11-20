/*
 *  GlobeAnimateHeight.mm
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

#import "GlobeAnimateHeight.h"

@implementation WhirlyGlobeAnimateViewHeight
{
    NSTimeInterval startDate;
    NSTimeInterval endDate;
    double startHeight,endHeight;
}

- (id)initWithView:(WhirlyGlobeView *)globeView toHeight:(double)height howLong:(float)howLong delegate:(NSObject<WGTiltDelegate> *)tiltDelegate
{
    if ((self = [super init]))
    {
        startDate = CFAbsoluteTimeGetCurrent();
        endDate = startDate+howLong;
        startHeight = globeView.heightAboveGlobe;
        endHeight = height;
        _tiltDelegate = tiltDelegate;
    }
    
    return self;
}


// Called by the view when it's time to update
- (void)updateView:(WhirlyGlobeView *)globeView
{
    if (startDate == 0.0)
        return;
	
	CFTimeInterval now = CFAbsoluteTimeGetCurrent();
    float span = endDate-startDate;
    float remain = endDate - now;
    
	// All done.  Snap to the end
	if (remain < 0)
	{
        globeView.heightAboveGlobe = endHeight;
        startDate = 0;
        endDate = 0;
        [globeView cancelAnimation];
	} else {
		// Interpolate somewhere along the path
		float t = (span-remain)/span;
        globeView.heightAboveGlobe = startHeight + (endHeight-startHeight)*t;

        if (_tiltDelegate)
        {
            float newTilt = [_tiltDelegate tiltFromHeight:globeView.heightAboveGlobe];
            [globeView setTilt:newTilt];
        }
    }
}

@end
