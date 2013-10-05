//
//  AnimationTest.m
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 7/31/13.
//  Copyright (c) 2013 mousebird consulting. All rights reserved.
//

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

//- (void)updateForFrame:(WhirlyKit::RendererFrameInfo *)frameInfo
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
