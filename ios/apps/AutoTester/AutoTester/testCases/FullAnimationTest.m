//
//  FullAnimationTest.m
//  AutoTester
//
//  Created by Ranen Ghosh on 4/7/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
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
        self.name = @"Animating Position";
        self.captureDelay = 2;
		self.implementations = MaplyTestCaseImplementationGlobe;
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
}

- (void)teardownWithBaseVC:(MaplyBaseViewController *)vc {
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
    
//    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(0.0, 0.0)];
//    globeVC.heading = 0.0;
    globeVC.height = 0.5;
    [globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222) height:0.1 heading:45.0/180.0*M_PI time:10.0];
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)globeVC];
    
}

@end

