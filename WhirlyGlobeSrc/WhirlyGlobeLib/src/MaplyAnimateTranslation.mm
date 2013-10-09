/*
 *  MaplyAnimateTranslation.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/20/12.
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

#import "MaplyAnimateTranslation.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation MaplyAnimateViewTranslation

- (id)initWithView:(MaplyView *)globeView translate:(Point3f &)newLoc howLong:(float)howLong
{
    self = [super init];
    
    if (self)
    {
        _startDate = CFAbsoluteTimeGetCurrent();
        _endDate = _startDate + howLong;
        _startLoc = globeView.loc;
        _endLoc = Point3d(newLoc.x(),newLoc.y(),newLoc.z());
    }
    
    return self;
}


- (void)updateView:(MaplyView *)mapView
{
    if (_startDate == 0.0)
        return;

    CFTimeInterval now = CFAbsoluteTimeGetCurrent();
    float span = _endDate - _startDate;
    float remain = _endDate - now;
    
    // All done, snap to end
    if (remain < 0)
    {
        [mapView setLoc:_endLoc];
        _startDate = 0;
        _endDate = 0;
        [mapView cancelAnimation];
    } else {
        // Interpolate in the middle
        float t = (span-remain)/span;
        Point3d midLoc = _startLoc + (_endLoc-_startLoc)*t;
        [mapView setLoc:midLoc];
    }
}

@end
