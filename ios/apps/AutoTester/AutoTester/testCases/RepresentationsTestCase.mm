//
//  RepresentationsTestCase.m
//  AutoTester
//
//  Created by Tim Sylvester on 6 Jan. 2020.
//  Copyright © 2021 mousebird consulting.
//

#import "VectorsTestCase.h"
#import "RepresentationsTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "AutoTester-Swift.h"

#include <stdlib.h>

@interface RepresentationsTestCase()
@end

@implementation RepresentationsTestCase {
    NSTimer *_animationTimer;
    NSArray<MaplyComponentObject *> *_vectorObjs;
    NSArray<MaplyComponentObject *> *_markerObjs;
    MaplyTexture *_dashedLineTex;
}

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Alternate Representations";
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
	}
	
	return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
    _animationTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                       target:self
                                                     selector:@selector(animationCallback)
                                                     userInfo:nil
                                                      repeats:NO];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)vc
{
    self.baseCase = [[VectorsTestCase alloc]init];
    [self.baseCase setUpWithGlobe:vc];
    [self setupWithBaseVC:vc];
    [vc setPosition:MaplyCoordinateMakeWithDegrees(50, -65) height:0.5];
}

- (void)setUpWithMap:(MaplyViewController *)vc
{
    self.baseCase = [[VectorsTestCase alloc]init];
    [self.baseCase setUpWithMap:vc];
    [self setupWithBaseVC:vc];
    [vc setPosition:MaplyCoordinateMakeWithDegrees(50, -65) height:0.5];
}

- (void) animationCallback
{
    if (!_dashedLineTex)
    {
        auto lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
        [lineTexBuilder setPattern:@[@(2),@(2)]];
        _dashedLineTex = [self.baseViewController addTexture:[lineTexBuilder makeImage]
                                         desc:@{kMaplyTexMinFilter: kMaplyMinFilterNearest,
                                                kMaplyTexMagFilter: kMaplyMinFilterNearest,
                                                kMaplyTexWrapX: @true,
                                                kMaplyTexWrapY: @true,
                                                kMaplyTexFormat: @(MaplyImageIntRGBA)}
                                         mode:MaplyThreadCurrent];
    }

    const char* const vecUUID = "abc—⃘�—⃘123";
    static const char* const reps[] = { "", "hilite", "subtle", "detail" };
    static int idx = 0;
    
    if (!_vectorObjs)
    {
        // First time
        
        // Set a representation before any objects are added
        // "detail" version should appear first: dashed magenta line with alternate geometry
        [self.baseViewController setRepresentation:@(reps[3]) ofUUIDs:@[@(vecUUID)]];

        const MaplyCoordinate pts1[] =
        {
            MaplyCoordinateMakeWithDegrees(50, -65),
            MaplyCoordinateMakeWithDegrees(150, -65),
        };
        const MaplyCoordinate pts2[] =
        {
            MaplyCoordinateMakeWithDegrees(50, -65),
            MaplyCoordinateMakeWithDegrees(100, -75),
            MaplyCoordinateMakeWithDegrees(150, -65),
        };

        const auto v1 = [[MaplyVectorObject alloc] initWithLineString:pts1 numCoords:sizeof(pts1)/sizeof(pts1[0]) attributes:nil];
        const auto v2 = [[MaplyVectorObject alloc] initWithLineString:pts2 numCoords:sizeof(pts2)/sizeof(pts2[0]) attributes:nil];

        _vectorObjs = @[
            [self.baseViewController addWideVectors: @[v1] desc:@{
                kMaplyEnable: @(false),
                kMaplyColor: [UIColor magentaColor],
                kMaplySubdivType: kMaplySubdivGreatCircle,
                kMaplySubdivEpsilon: @0.0001,
                kMaplyVecWidth: @3,
                kMaplyWideVecOffset: @-1.5,
                kMaplyUUID: @(vecUUID),
                kMaplySelectable: @"blah",
                //kMaplyRepresentation: @"" }   // not set
                }],
            [self.baseViewController addWideVectors: @[v1] desc:@{
                kMaplyEnable: @(false),
                kMaplyColor: [UIColor redColor],
                kMaplySubdivType: kMaplySubdivGreatCircle,
                kMaplySubdivEpsilon: @0.0001,
                kMaplyVecWidth: @10,
                kMaplyWideVecOffset: @-10,
                kMaplyUUID: @(vecUUID),
                kMaplyRepresentation: @(reps[1])
                }],
            [self.baseViewController addWideVectors: @[v1] desc:@{
                kMaplyEnable: @(false),
                kMaplyColor: [[UIColor grayColor] colorWithAlphaComponent:0.5],
                kMaplySubdivType: kMaplySubdivGreatCircle,
                kMaplySubdivEpsilon: @0.0001,
                kMaplyVecWidth: @3,
                kMaplyWideVecOffset: @-1.5,
                kMaplyUUID: @(vecUUID),
                kMaplyRepresentation: @(reps[2])
                }],
            [self.baseViewController addWideVectors: @[v2] desc:@{
                kMaplyEnable: @(false),
                kMaplyColor: [UIColor magentaColor],
                kMaplySubdivType: kMaplySubdivGreatCircle,
                kMaplySubdivEpsilon: @0.0001,
                kMaplyVecWidth: @6,
                kMaplyWideVecOffset: @-3,
                kMaplyVecTexture: _dashedLineTex,
                kMaplyUUID: @(vecUUID),
                kMaplyRepresentation: @(reps[3])
                }],
        ];
    }
    else
    {
        // Not the first time

        [self.baseViewController setRepresentation:@(reps[idx]) ofUUIDs:@[@(vecUUID)]];
        idx = (idx + 1) % (sizeof(reps) / sizeof(reps[0]));
    }

    _animationTimer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(animationCallback) userInfo:nil repeats:NO];
}

- (void) stop
{
    if (_animationTimer) {
        [_animationTimer invalidate];
        _animationTimer = nil;
    }

    _vectorObjs = nil;
    _markerObjs = nil;
    _dashedLineTex = nil;

    [self.baseCase stop];
}

@end
