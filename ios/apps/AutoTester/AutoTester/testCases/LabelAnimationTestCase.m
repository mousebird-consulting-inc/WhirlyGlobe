//
//  LabelAnimationTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 4/7/16.
//  Copyright © 2016-2017 mousebird consulting.
//

#import "LabelAnimationTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyScreenLabel.h"
#import "AutoTester-Swift.h"

@implementation LabelAnimationTestCase {
    NSTimer *_labelAnimationTimer;
    NSMutableDictionary *_trafficLabels;
    MaplyBaseViewController *_baseVC;
    CartoDBLightTestCase *baseCase;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Label Animation";
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        
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

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    baseCase = [[CartoDBLightTestCase alloc] init];
    [baseCase setUpWithGlobe:globeVC];
    globeVC.height = 0.25;
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(-100.0, 40.0) height:0.05];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)globeVC];
    
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    baseCase = [[CartoDBLightTestCase alloc] init];
    [baseCase setUpWithMap:mapVC];
    mapVC.height = 0.05;
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-100.0, 40.0) time:0.0];
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
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
    
    NSDictionary *labelsDesc = @{kMaplyMinVis: @(0.0),
                                 kMaplyMaxVis: @(1.0),
                                 kMaplyFade: @(0.3),
                                 kMaplyTextColor: [UIColor redColor],
                                 kMaplyJustify : @"left",
                                 kMaplyDrawPriority: @(50)};
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
        label.color = [UIColor redColor];
        
        
        trafficLabelCompObj = [_baseVC addScreenLabels:[NSArray arrayWithObjects:label, nil] desc:labelsDesc];
        _trafficLabels[@(i)] = trafficLabelCompObj;
        
    }
    
    
    _labelAnimationTimer = [NSTimer scheduledTimerWithTimeInterval:10.0 target:self selector:@selector(labelAnimationCallback) userInfo:nil repeats:NO];
}


@end
