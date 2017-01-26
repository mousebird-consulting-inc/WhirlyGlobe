/*
 *  AnimationTest.m
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 7/31/13.
 *  Copyright 2011-2015 mousebird consulting
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

#import "AnimationTest.h"
//#import "WhirlyGlobe.h"

// Note: Should import WhirlyGlobe.h here instead of doing this
//       But we can hack it for the moment

@implementation AnimatedSphere
{
    NSTimeInterval start;
    float period;
    float radius;
    UIColor *color;
    MaplyCoordinate startPt;
    MaplyComponentObject *sphereObj;
}

- (id)initWithPeriod:(float)inPeriod radius:(float)inRadius color:(UIColor *)inColor viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithViewController:viewC];
    if (!self)
        return nil;
    
    period = inPeriod;
    radius = inRadius;
    color = inColor;
    
    start = CFAbsoluteTimeGetCurrent();
    return self;
}

- (bool)hasUpdate
{
    return true;
}

//- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo
- (void)updateForFrame:(id)frameInfo
{
    if (sphereObj)
    {
        [super.viewC removeObjects:@[sphereObj] mode:MaplyThreadCurrent];
        sphereObj = nil;
    }

    float t = (CFAbsoluteTimeGetCurrent()-start)/period;
    t -= (int)t;
    
    MaplyCoordinate center = MaplyCoordinateMakeWithDegrees(-180+t*360.0, 0.0);
    
    MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
    sphere.radius = radius;
    sphere.center = center;
    
    // Here's the trick, we must use MaplyThreadCurrent to make this happen right now
    sphereObj = [super.viewC addShapes:@[sphere] desc:@{kMaplyColor: color} mode:MaplyThreadCurrent];
}

- (void)shutdown
{
    if (sphereObj)
    {
        [super.viewC removeObjects:@[sphereObj] mode:MaplyThreadCurrent];
        sphereObj = nil;
    }
}


@end
