/*
 *  AnimateTranslation.mm
 *  WhirlyGlobeLib
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

#import "AnimateTranslation.h"

using namespace WhirlyKit;
using namespace Eigen;

@implementation AnimateViewTranslation

@synthesize startDate;
@synthesize endDate;
@synthesize startLoc;
@synthesize endLoc;

- (id)initWithView:(WhirlyMapView *)globeView translate:(Point3f &)newLoc howLong:(float)howLong
{
    self = [super init];
    
    if (self)
    {
        self.startDate = [NSDate date];
        self.endDate = [startDate dateByAddingTimeInterval:howLong];
        startLoc = globeView.loc;
        endLoc = newLoc;
    }
    
    return self;
}


- (void)updateView:(WhirlyMapView *)mapView
{
    if (!self.startDate)
        return;
    
    NSDate *now = [NSDate date];
    float span = (float)[endDate timeIntervalSinceDate:startDate];
    float remain = (float)[endDate timeIntervalSinceDate:now];
    
    // All done, snap to end
    if (remain < 0)
    {
        [mapView setLoc:endLoc];
        self.startDate = nil;
        self.endDate = nil;
    } else {
        // Interpolate in the middle
        float t = (span-remain)/span;
        Point3f midLoc = startLoc + (endLoc-startLoc)*t;
        [mapView setLoc:midLoc];
    }
}

@end