//
//  FullAnimationTest.m
//  AutoTester
//
//  Created by Ranen Ghosh on 4/7/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "FullAnimationTest.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "CartoDBTestCase.h"

@implementation FullAnimationTest {
    MaplyBaseViewController *_baseVC;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Full Animation";
        self.captureDelay = 2;
        
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
}

- (void)teardownWithBaseVC:(MaplyBaseViewController *)vc {
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
//    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5];
    
    [globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222) height:0.1 heading:45.0/180.0*M_PI time:10.0];
    
    return YES;
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)globeVC];
    
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithMap:mapVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) time:0.0];
    
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222) height:0.1  time:10.0];
    
    return YES;
}

- (void)tearDownWithMap:(MaplyViewController * _Nonnull)mapVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)mapVC];
    
}


@end

