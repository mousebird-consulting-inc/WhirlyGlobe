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
    [vc setPosition:MaplyCoordinateMakeWithDegrees(100, -70) height:1.0];
}

- (void)setUpWithMap:(MaplyViewController *)vc
{
    self.baseCase = [[VectorsTestCase alloc]init];
    [self.baseCase setUpWithMap:vc];
    [self setupWithBaseVC:vc];
    [vc setPosition:MaplyCoordinateMakeWithDegrees(100, -70) height:1.0];
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
    static const char* const reps[] = { "", "hilite", "�subtle", "detail" };
    static int idx = 0;
    
    if (!_vectorObjs)
    {
        // First time
        
        // Set a representation before any objects are added
        // "detail" version should appear first: dashed magenta line with alternate geometry
        [self.baseViewController setRepresentation:@(reps[3]) ofUUIDs:@[@(vecUUID)]];

        const MaplyCoordinate pts[] = { MaplyCoordinateMakeWithDegrees(50, -65), MaplyCoordinateMakeWithDegrees(150, -65) };
        const auto vec = [[MaplyVectorObject alloc] initWithLineString:pts numCoords:sizeof(pts)/sizeof(pts[0]) attributes:nil];
        _vectorObjs = @[
            [self.baseViewController addWideVectors: @[vec] desc:@{
                kMaplyEnable: @(false),
                kMaplyColor: [UIColor magentaColor],
                kMaplySubdivType: kMaplySubdivGreatCircle,
                kMaplySubdivEpsilon: @0.01,
                kMaplyVecWidth: @3,
                kMaplyWideVecOffset: @-1.5,
                kMaplyUUID: @(vecUUID),
                //kMaplyRepresentation: @""    // not set
                }],
            [self.baseViewController addWideVectors: @[vec] desc:@{
                kMaplyEnable: @(false),
                kMaplyColor: [UIColor redColor],
                kMaplySubdivType: kMaplySubdivGreatCircle,
                kMaplySubdivEpsilon: @0.001,
                kMaplyVecWidth: @10,
                kMaplyWideVecOffset: @-10,
                kMaplyUUID: @(vecUUID),
                kMaplyRepresentation: @(reps[1])
                }],
            [self.baseViewController addWideVectors: @[vec] desc:@{
                kMaplyEnable: @(false),
                kMaplyColor: [[UIColor blackColor] colorWithAlphaComponent:0.5],
                kMaplySubdivType: kMaplySubdivGreatCircle,
                kMaplySubdivEpsilon: @0.0001,
                kMaplyVecWidth: @3,
                kMaplyWideVecOffset: @-1.5,
                kMaplyUUID: @(vecUUID),
                kMaplyRepresentation: @(reps[2])
                }],
            [self.baseViewController addWideVectors: @[vec] desc:@{
                kMaplyEnable: @(false),
                kMaplyColor: [UIColor magentaColor],
                kMaplySubdivType: kMaplySubdivGreatCircle,
                kMaplySubdivEpsilon: @0.00001,
                kMaplyVecWidth: @6,
                kMaplyWideVecOffset: @-3,
                kMaplyVecTexture: _dashedLineTex,
                kMaplyUUID: @(vecUUID),
                kMaplyRepresentation: @(reps[3])
                }],
        ];

        const auto vecSubdiv = [[MaplyVectorObject alloc] initWithLineString:pts numCoords:sizeof(pts)/sizeof(pts[0]) attributes:nil];
        [vecSubdiv subdivideToGlobe:0.0001];

        const auto m1 = [MaplyScreenMarker new];
        m1.loc = [vecSubdiv center];
        m1.size = CGSizeMake(20, 20);
        m1.color = [UIColor magentaColor];
        m1.layoutImportance = MAXFLOAT;

        const auto m2 = [MaplyScreenMarker new];
        m2.loc = [vecSubdiv centroid];
        m2.size = CGSizeMake(50, 50);
        m2.color = [UIColor redColor];
        m1.rotation = M_PI_4;
        m2.layoutImportance = MAXFLOAT;

        const auto m3 = [MaplyScreenMarker new];
        m3.loc = [vecSubdiv linearMiddle:self.baseViewController.coordSystem];
        m3.size = CGSizeMake(30, 30);
        m3.color = [[UIColor blackColor] colorWithAlphaComponent:0.5];
        m3.layoutImportance = MAXFLOAT;

        _markerObjs = @[
            [self.baseViewController addScreenMarkers:@[m1] desc:@{
                kMaplyEnable: @(false),
                kMaplyUUID: @(vecUUID),
                //kMaplyRepresentation: @""    // not set
            }],
            [self.baseViewController addScreenMarkers:@[m2] desc:@{
                kMaplyEnable: @(false),
                kMaplyUUID: @(vecUUID),
                kMaplyRepresentation: @(reps[1])
            }],
            [self.baseViewController addScreenMarkers:@[m3] desc:@{
                kMaplyEnable: @(false),
                kMaplyUUID: @(vecUUID),
                kMaplyRepresentation: @(reps[2])
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
