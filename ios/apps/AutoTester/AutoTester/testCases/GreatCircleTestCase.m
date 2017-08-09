//
//  GreatCircleTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 3/11/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "GreatCircleTestCase.h"
#import "CartoDBTestCase.h"

@implementation GreatCircleTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Great Circles";
        self.captureDelay = 20;
        self.implementations = MaplyTestCaseOptionGlobe | MaplyTestCaseOptionMap;

    }
    
    return self;
}

-(NSArray *)addLongRoute:(MaplyBaseViewController *)viewC globe:(bool)isGlobe
{
    NSMutableArray *compObjs = [NSMutableArray array];
    
    if (true) {
        MaplyCoordinate x = MaplyCoordinateMakeWithDegrees(2.548, 49.010);
        MaplyCoordinate y = MaplyCoordinateMakeWithDegrees(151.177, -33.946);
        MaplyCoordinate z[] = { x, y };
        MaplyVectorObject *v0 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        [v0 subdivideToGlobeGreatCircle:0.001];
        
        [compObjs addObject:[viewC addWideVectors:@[v0] desc:@{
                                                               kMaplyColor: [UIColor redColor],
                                                               kMaplyEnable: @(YES),
                                                               kMaplyVecWidth: @(6.0),
                                                               }]];
    }

    if (true) {
        MaplyCoordinate x = MaplyCoordinateMakeWithDegrees(150.0, 0.0);
        MaplyCoordinate y = MaplyCoordinateMakeWithDegrees(-150, 0.0);
        MaplyCoordinate z[] = { x, y };
        MaplyVectorObject *v1 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        if (isGlobe)
            [v1 subdivideToGlobeGreatCircle:0.001];
        else {
            [v1 subdivideToFlatGreatCircle:0.001];
        }

        [compObjs addObject:[viewC addWideVectors:@[v1] desc:@{
                                                               kMaplyColor: [UIColor blueColor],
                                                               kMaplyEnable: @(YES),
                                                               kMaplyVecWidth: @(6.0),
                                                               }]];
    }

    if (true) {
        MaplyCoordinate a = MaplyCoordinateMakeWithDegrees(-176.4624, -44.3040);
//        MaplyCoordinate b = MaplyCoordinateMakeWithDegrees(180.0, -44.3040);
        MaplyCoordinate c = MaplyCoordinateMakeWithDegrees(171.2303, 44.3040);
//        MaplyCoordinate x = MaplyCoordinateMake(2.98844504, -0.773229361);
//        MaplyCoordinate y = MaplyCoordinateMake(-3.07975984, -0.764628767);
        MaplyCoordinate z[] = {a, c };
        MaplyVectorObject *v2 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        if (isGlobe)
            [v2 subdivideToGlobeGreatCircle:0.001];
        else {
            [v2 subdivideToFlatGreatCircle:0.001];
        }

        [compObjs addObject:[viewC addWideVectors:@[v2] desc:@{
                                                               kMaplyColor: [UIColor greenColor],
                                                               kMaplyEnable: @(YES),
                                                               kMaplyVecWidth: @(6.0),
                                                               }]];
    }

    // Stop at -180/+180
    if (false) {
        MaplyCoordinate a = MaplyCoordinateMakeWithDegrees(-176.4624, -44.3040);
        MaplyCoordinate b1 = MaplyCoordinateMakeWithDegrees(-180.0, 0.0);
        MaplyCoordinate b2 = MaplyCoordinateMakeWithDegrees(180.0, 0.0);
        MaplyCoordinate c = MaplyCoordinateMakeWithDegrees(171.2303, 44.3040);
        //        MaplyCoordinate x = MaplyCoordinateMake(2.98844504, -0.773229361);
        //        MaplyCoordinate y = MaplyCoordinateMake(-3.07975984, -0.764628767);
        MaplyCoordinate line1[] = {a, b1 };
        MaplyCoordinate line2[] = {b2, c };
        MaplyVectorObject *v0 = [[MaplyVectorObject alloc] initWithLineString:line1 numCoords:2 attributes:nil];
        MaplyVectorObject *v1 = [[MaplyVectorObject alloc] initWithLineString:line2 numCoords:2 attributes:nil];
        if (isGlobe)
        {
            [v0 subdivideToGlobeGreatCircle:0.001];
            [v1 subdivideToGlobeGreatCircle:0.001];
        } else {
            [v0 subdivideToFlatGreatCircle:0.001];
            [v1 subdivideToFlatGreatCircle:0.001];
        }
        
        [compObjs addObject:[viewC addWideVectors:@[v0] desc:@{
                                                               kMaplyColor: [UIColor orangeColor],
                                                               kMaplyEnable: @(YES),
                                                               kMaplyVecWidth: @(6.0),
                                                               }]];
        [compObjs addObject:[viewC addWideVectors:@[v1] desc:@{
                                                               kMaplyColor: [UIColor yellowColor],
                                                               kMaplyEnable: @(YES),
                                                               kMaplyVecWidth: @(6.0),
                                                               }]];
    }
    
    if (true) {
        MaplyCoordinate a = MaplyCoordinateMakeWithDegrees(-179.686999,-24.950296);
        MaplyCoordinate c = MaplyCoordinateMakeWithDegrees(179.950328,-22.180086);
        MaplyCoordinate z[] = {a, c };
        MaplyVectorObject *v2 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        if (isGlobe)
            [v2 subdivideToGlobeGreatCircle:0.001];
        else
            [v2 subdivideToFlatGreatCircle:0.001];

        [compObjs addObject:[viewC addVectors:@[v2] desc:@{
                                                           kMaplyColor: [UIColor purpleColor],
                                                           kMaplyEnable: @(YES),
                                                           kMaplyVecWidth: @(6.0),
                                                           }]];
    }

    if (true) {
        MaplyCoordinate a = MaplyCoordinateMakeWithDegrees(-175.0,-33.092222);
        MaplyCoordinate c = MaplyCoordinateMakeWithDegrees(177.944183,-34.845333);
        MaplyCoordinate z[] = {a, c };
        MaplyVectorObject *v2 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        if (isGlobe)
            [v2 subdivideToGlobeGreatCircle:0.0001];
        else
            [v2 subdivideToFlatGreatCircle:0.0001];
        
        [compObjs addObject:[viewC addVectors:@[v2] desc:@{
                                                           kMaplyColor: [UIColor brownColor],
                                                           kMaplyEnable: @(YES),
                                                           kMaplyVecWidth: @(6.0),
                                                           }]];
    }

    if (true) {
        MaplyCoordinate a = MaplyCoordinateMakeWithDegrees(177.747519,-34.672406);
        MaplyCoordinate c = MaplyCoordinateMakeWithDegrees(-175.0,-31.833547);
        MaplyCoordinate z[] = {a, c };
        MaplyVectorObject *v2 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        if (isGlobe)
            [v2 subdivideToGlobeGreatCircle:0.0001];
        else
            [v2 subdivideToFlatGreatCircle:0.0001];
        
        [compObjs addObject:[viewC addVectors:@[v2] desc:@{
                                                           kMaplyColor: [UIColor brownColor],
                                                           kMaplyEnable: @(YES),
                                                           kMaplyVecWidth: @(6.0),
                                                           }]];
    }

    if (true) {
        MaplyCoordinate a = MaplyCoordinateMakeWithDegrees(-175.0,-31.833547);
        MaplyCoordinate c = MaplyCoordinateMakeWithDegrees(178.394161,-35.357856);
        MaplyCoordinate z[] = {a, c };
        MaplyVectorObject *v2 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        if (isGlobe)
            [v2 subdivideToGlobeGreatCircle:0.0001];
        else
            [v2 subdivideToFlatGreatCircle:0.0001];
        
        [compObjs addObject:[viewC addVectors:@[v2] desc:@{
                                                           kMaplyColor: [UIColor brownColor],
                                                           kMaplyEnable: @(YES),
                                                           kMaplyVecWidth: @(6.0),
                                                           }]];
    }

    // Line to see where the division is
    if (true) {
        MaplyCoordinate x = MaplyCoordinateMakeWithDegrees(180., -89.9);
        MaplyCoordinate y = MaplyCoordinateMakeWithDegrees(180., 89.9);
        MaplyCoordinate z[] = { x, y };
        MaplyVectorObject *v3 = [[MaplyVectorObject alloc] initWithLineString:z numCoords:2 attributes:nil];
        if (isGlobe)
            [v3 subdivideToGlobeGreatCircle:0.001];
        
        [compObjs addObject:[viewC addVectors:@[v3] desc:@{
                                                           kMaplyColor: [UIColor whiteColor],
                                                           kMaplyEnable: @(YES),
                                                           kMaplyVecWidth: @(6.0),
                                                           }]];
    }
    
    return compObjs;
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    globeVC.keepNorthUp = false;
    
    [self addLongRoute:globeVC globe:true];
    
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(2.548,49.010) height:1.0];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithMap:mapVC];
    
    [self addLongRoute:mapVC globe:false];
    
    [mapVC setPosition:MaplyCoordinateMakeWithDegrees(2.548,49.010) height:1.0];
}

@end
