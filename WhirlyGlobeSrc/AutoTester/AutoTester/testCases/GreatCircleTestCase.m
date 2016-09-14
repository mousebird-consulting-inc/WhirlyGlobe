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
        self.implementations = MaplyTestCaseOptionGlobe;

    }
    
    return self;
}

-(MaplyComponentObject *)addLongRoute:(MaplyBaseViewController *)viewC
{
    NSDictionary *vecDesc = @{
                              kMaplyColor: [UIColor redColor],
                              kMaplyEnable: @(YES),
                              kMaplyVecWidth: @(6.0),
//                              kMaplyVecHeight: @(0.0),
                              kMaplyDrawPriority: @(10000),
//                              kMaplySubdivType: kMaplySubdivGreatCircle,
//                              kMaplySubdivEpsilon: @(0.001)
                              };
    
    MaplyVectorObject *v0,*v1,*v2;
    {
        MaplyCoordinate x = MaplyCoordinateMakeWithDegrees(2.548, 49.010);
        MaplyCoordinate y = MaplyCoordinateMakeWithDegrees(151.177, -33.946);
        MaplyCoordinate z[] = { x, y };
        v0 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        [v0 subdivideToGlobeGreatCircle:0.001];
    }

    {
        MaplyCoordinate x = MaplyCoordinateMakeWithDegrees(150.0, 0.0);
        MaplyCoordinate y = MaplyCoordinateMakeWithDegrees(-150, 0.0);
        MaplyCoordinate z[] = { x, y };
        v1 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        [v1 subdivideToGlobeGreatCircle:0.001];
    }

    {
//        MaplyCoordinate x = MaplyCoordinateMakeWithDegrees(160.0, -25.0);
//        MaplyCoordinate y = MaplyCoordinateMakeWithDegrees(180., -24.0);
        MaplyCoordinate x = MaplyCoordinateMake(2.98844504, -0.773229361);
        MaplyCoordinate y = MaplyCoordinateMake(-3.07975984, -0.764628767);
        MaplyCoordinate z[] = { x, y };
        v2 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        [v2 subdivideToGlobeGreatCircle:0.0001];
    }

    
//    return [viewC addWideVectors:@[ v0, v1, v2 ] desc:vecDesc];
//    return [viewC addWideVectors:@[ v2 ] desc:vecDesc];
    return [viewC addVectors:@[ v0, v1, v2 ] desc:vecDesc];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    StamenWatercolorRemote *baseView = [[StamenWatercolorRemote alloc] init];
    [baseView setUpWithGlobe:globeVC];
    
    [self addLongRoute:globeVC];
    
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(2.548,49.010) height:1.0];
}

@end
