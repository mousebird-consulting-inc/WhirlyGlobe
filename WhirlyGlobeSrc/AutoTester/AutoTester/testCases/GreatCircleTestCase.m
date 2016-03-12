//
//  GreatCircleTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 3/11/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "GreatCircleTestCase.h"
#import "AutoTester-swift.h"

@implementation GreatCircleTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Great Circles";
        self.captureDelay = 20;
    }
    
    return self;
}

-(MaplyComponentObject *)addLongRoute:(MaplyBaseViewController *)viewC
{
    NSDictionary *vecDesc = @{
                              kMaplyColor: [UIColor redColor],
                              kMaplyEnable: @(YES),
                              kMaplyVecWidth: @(10.0),
                              kMaplyVecHeight: @(0.0),
                              kMaplyDrawPriority: @(10000),
                              kMaplySubdivType: kMaplySubdivGreatCircle,
                              kMaplySubdivEpsilon: @(0.001)
                              };
    
    MaplyCoordinate x = MaplyCoordinateMakeWithDegrees(2.548, 49.010);
    MaplyCoordinate y = MaplyCoordinateMakeWithDegrees(151.177, -33.946);
    MaplyCoordinate z[] = { x, y };
    MaplyVectorObject *v = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
    return [viewC addWideVectors:@[ v ] desc:vecDesc];
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    StamenWatercolorRemote *baseView = [[StamenWatercolorRemote alloc] init];
    [baseView setUpWithGlobe:globeVC];
    
    [self addLongRoute:globeVC];
    
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(2.548,49.010) height:1.0];
    
    return true;
}

@end
