//
//  VectorsTestCase.m
//  AutoTester
//
//  Created by Tim Sylvester on 31 Dec. 2020.
//  Copyright © 2020 mousebird consulting.
//

#import "VectorsTestCase.h"
#import "ChangeVectorsTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "AutoTester-Swift.h"

#include <stdlib.h>

@interface ChangeVectorsTestCase()
@end

@implementation ChangeVectorsTestCase {
    NSTimer *_animationTimer;
    MaplyComponentObject *_vecObj;
    MaplyComponentObject *_wideVecObj;
}

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Change Vectors";
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
	}
	
	return self;
}

- (void)subdivide:(MaplyVectorObject *)obj withVC:(MaplyBaseViewController *)vc epsilon:(float)epsilon {
    const bool isGlobe = [vc isKindOfClass:[WhirlyGlobeViewController class]];
    if (isGlobe) [obj subdivideToGlobeGreatCircle:epsilon];
    else         [obj subdivideToFlatGreatCircle:epsilon];
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
    _animationTimer = [NSTimer scheduledTimerWithTimeInterval:2.0
                                                       target:self
                                                     selector:@selector(animationCallback)
                                                     userInfo:nil
                                                      repeats:YES];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)vc
{
    self.baseCase = [[VectorsTestCase alloc]init];
    [self.baseCase setUpWithGlobe:vc];
    [self setupWithBaseVC:vc];
}

- (void)setUpWithMap:(MaplyViewController *)vc
{
    self.baseCase = [[VectorsTestCase alloc]init];
    [self.baseCase setUpWithMap:vc];
    [self setupWithBaseVC:vc];
}

+ (nonnull UIColor *)randomColor {
    return [UIColor colorWithRed:(arc4random()%256)/255.0f
                           green:(arc4random()%256)/255.0f
                            blue:(arc4random()%256)/255.0f
                           alpha:1.0f];
}

- (void) animationCallback
{
    if (!_vecObj)
    {
        const MaplyCoordinate pts[] = { MaplyCoordinateMakeWithDegrees(50, -65), MaplyCoordinateMakeWithDegrees(150, -65) };
        const auto vec = [[MaplyVectorObject alloc] initWithLineString:pts numCoords:2 attributes:nil];
        [self subdivide:vec withVC:self.baseViewController epsilon:0.0001];
        _vecObj = [self.baseViewController addVectors:@[vec] desc:@{
            kMaplyEnable: @(YES),
            kMaplyColor: [UIColor magentaColor]
        }];
    }
    if (!_wideVecObj)
    {
        const MaplyCoordinate wpts[] = { MaplyCoordinateMakeWithDegrees(50, -66), MaplyCoordinateMakeWithDegrees(150, -66) };
        const auto wvec = [[MaplyVectorObject alloc] initWithLineString:wpts numCoords:2 attributes:nil];
        [self subdivide:wvec withVC:self.baseViewController epsilon:0.0001];
        _wideVecObj = [self.baseViewController addWideVectors:@[wvec] desc:@{
            kMaplyEnable: @(YES),
            kMaplyColor: [UIColor redColor],
            kMaplyVecWidth: @(5.0)
        }];
    }

    if (_vecObj)
    {
        [self.baseViewController changeVector:_vecObj desc:@{
            kMaplyEnable: @((arc4random()%10) ? YES : NO),
            kMaplyColor: [ChangeVectorsTestCase randomColor],
            kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault)
        }];
    }
    if (_wideVecObj)
    {
        [self.baseViewController changeVector:_wideVecObj desc:@{
            kMaplyEnable: @((arc4random()%10) ? YES : NO),
            kMaplyColor: [ChangeVectorsTestCase randomColor],
            kMaplyVecWidth: @((arc4random()%33) / 4.0f),
            kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault)
        }];
    }
}

- (void) stop
{
    if (_animationTimer) {
        [_animationTimer invalidate];
        _animationTimer = nil;
    }

    [self.baseCase stop];
}

@end
