//
//  FindHeightTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 4/7/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "FindHeightTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "CartoDBTestCase.h"

@implementation FindHeightTestCase {
    MaplyBaseViewController *_baseVC;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Find Height";
        self.captureDelay = 2;
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
}

- (void)teardownWithBaseVC:(MaplyBaseViewController *)vc {
}

- (void)addBoundingBox:(MaplyBoundingBox)bbox baseVC:(MaplyBaseViewController *)baseVC
{
    // Add a visible bounding box
    MaplyCoordinate coords[5];
    coords[0] = MaplyCoordinateMake(bbox.ll.x,bbox.ll.y);
    coords[1] = MaplyCoordinateMake(bbox.ur.x,bbox.ll.y);
    coords[2] = MaplyCoordinateMake(bbox.ur.x,bbox.ur.y);
    coords[3] = MaplyCoordinateMake(bbox.ll.x,bbox.ur.y);
    coords[4] = MaplyCoordinateMake(bbox.ll.x,bbox.ll.y);
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:5 attributes:nil];
    [baseVC addVectors:@[vecObj] desc:@{kMaplyColor: [UIColor redColor]}];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5];

    dispatch_after( dispatch_time(DISPATCH_TIME_NOW, 5.0 * NSEC_PER_SEC), dispatch_get_main_queue(),
    ^{
       MaplyBoundingBox bbox;
       bbox.ll = MaplyCoordinateMakeWithDegrees(7.05090689853, 47.7675500593);
       bbox.ur = MaplyCoordinateMakeWithDegrees(8.06813647023, 49.0562323851);
       MaplyCoordinate center = MaplyCoordinateMakeWithDegrees((7.05090689853+8.06813647023)/2, (47.7675500593+49.0562323851)/2);
       double height = [globeVC findHeightToViewBounds:bbox pos:center];
       globeVC.height = height;
       [globeVC animateToPosition:center time:1.0];
       NSLog(@"height = %f",height);

        [self addBoundingBox:bbox baseVC:globeVC];
    });
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)globeVC];
    
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithMap:mapVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) time:0.0];
    
    dispatch_after( dispatch_time(DISPATCH_TIME_NOW, 5.0 * NSEC_PER_SEC), dispatch_get_main_queue(),
    ^{
        MaplyBoundingBox bbox;
        bbox.ll = MaplyCoordinateMakeWithDegrees(7.05090689853, 47.7675500593);
        bbox.ur = MaplyCoordinateMakeWithDegrees(8.06813647023, 49.0562323851);
        MaplyCoordinate center = MaplyCoordinateMakeWithDegrees((7.05090689853+8.06813647023)/2, (47.7675500593+49.0562323851)/2);
        double height = [mapVC findHeightToViewBounds:bbox pos:center marginX:20.0 marginY:100.0];
        mapVC.height = height;
        [mapVC animateToPosition:center time:1.0];
        NSLog(@"height = %f",height);
        
        [self addBoundingBox:bbox baseVC:mapVC];
    });
}

- (void)tearDownWithMap:(MaplyViewController * _Nonnull)mapVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)mapVC];
    
}

@end
