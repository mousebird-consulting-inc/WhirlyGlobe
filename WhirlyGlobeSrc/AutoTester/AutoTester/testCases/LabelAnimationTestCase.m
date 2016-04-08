//
//  LabelAnimationTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 4/7/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "LabelAnimationTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "CartoDBTestCase.h"
#import "MaplyScreenLabel.h"


@implementation LabelAnimationTestCase {
    NSTimer *_labelAnimationTimer;
    NSMutableDictionary *_trafficLabels;
    MaplyBaseViewController *_baseVC;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Label Animation";
        self.captureDelay = 2;
        
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
    _baseVC = vc;
    _trafficLabels = [NSMutableDictionary dictionary];

    // Create a bunch of labels periodically
    _labelAnimationTimer = [NSTimer scheduledTimerWithTimeInterval:1.25 target:self selector:@selector(labelAnimationCallback) userInfo:nil repeats:NO];
    
}

- (void)teardownWithBaseVC:(MaplyBaseViewController *)vc {
    if (_labelAnimationTimer) {
        [_labelAnimationTimer invalidate];
        _labelAnimationTimer = nil;
    }
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
    
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
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) time:0.0];
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
    
    return YES;
}

- (void)tearDownWithMap:(MaplyViewController * _Nonnull)mapVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)mapVC];
    
}

- (void) labelAnimationCallback
{
    NSLog(@"anim callback");
    MaplyComponentObject *trafficLabelCompObj;
    
    NSArray *keys = [_trafficLabels allKeys];
    for (NSObject *key in keys) {
        trafficLabelCompObj = _trafficLabels[key];
        if (!trafficLabelCompObj)
            continue;
        [_baseVC removeObject:trafficLabelCompObj];
        [_trafficLabels removeObjectForKey:key];
    }
    
    NSDictionary *labelsDesc = @{kMaplyMinVis: @(0.0), kMaplyMaxVis: @(1.0), kMaplyFade: @(0.3), kMaplyJustify : @"left", kMaplyDrawPriority: @(50)};
    MaplyScreenLabel *label;
    for (int i=0; i<50; i++) {
        label = [[MaplyScreenLabel alloc] init];
        
        label.loc = MaplyCoordinateMakeWithDegrees(
                                                   -100.0 + 0.25 * ((float)arc4random()/0x100000000),
                                                   40.0 + 0.25 * ((float)arc4random()/0x100000000));
        label.rotation = 0.0;
        label.layoutImportance = 1.0;
        label.text = @"ABCDE";
        label.layoutPlacement = kMaplyLayoutRight;
        label.userObject = nil;
        label.color = [UIColor whiteColor];
        
        
        trafficLabelCompObj = [_baseVC addScreenLabels:[NSArray arrayWithObjects:label, nil] desc:labelsDesc];
        _trafficLabels[@(i)] = trafficLabelCompObj;
        
    }
    
    
    _labelAnimationTimer = [NSTimer scheduledTimerWithTimeInterval:1.25 target:self selector:@selector(labelAnimationCallback) userInfo:nil repeats:NO];
}


@end
