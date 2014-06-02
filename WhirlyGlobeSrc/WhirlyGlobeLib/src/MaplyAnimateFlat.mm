/*
 *  MaplyAnimateFlat.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/24/13.
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

#import "MaplyAnimateFlat.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyAnimateFlat
{
    CFTimeInterval startDate,endDate;
    Point2f windowSize,destWindowSize;
    Point2f contentOffset,destContentOffset;
    MaplyFlatView __weak *flatView;
}

- (id)initWithView:(MaplyFlatView *)inFlatView destWindow:(WhirlyKit::Point2f)inWindowSize destContentOffset:(WhirlyKit::Point2f)inContentOffset howLong:(float)howLong
{
    self = [super init];
    if (!self)
        return nil;
    
    flatView = inFlatView;
    startDate = CFAbsoluteTimeGetCurrent();
    endDate = startDate + howLong;
    windowSize = flatView.windowSize;
    destWindowSize = inWindowSize;
    contentOffset = flatView.contentOffset;
    destContentOffset = inContentOffset;
    
    return self;
}

- (void)updateView:(MaplyView *)mapView
{
    if (startDate == 0.0)
        return;
 
    CFTimeInterval now = CFAbsoluteTimeGetCurrent();
    float span = endDate - startDate;
    float remain = endDate - now;
    
    // All done, snap to end
    if (remain < 0)
    {
        [flatView setWindowSize:destWindowSize contentOffset:destContentOffset];
        startDate = 0;
        endDate = 0;
        [mapView cancelAnimation];
    } else {
        // Interpolate in the middle
        float t = (span-remain)/span;
        Point2f curWindowSize,curContentOffset;
        curWindowSize = windowSize + (destWindowSize-windowSize)*t;
        curContentOffset = contentOffset + (destContentOffset-contentOffset)*t;
        [flatView setWindowSize:curWindowSize contentOffset:curContentOffset];
    }
}

@end
