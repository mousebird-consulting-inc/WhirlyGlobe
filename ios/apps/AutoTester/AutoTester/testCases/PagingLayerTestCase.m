//
//  PagingLayerTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 6/28/16.
//  Copyright © 2016-2017 mousebird consulting.
//

#import "PagingLayerTestCase.h"
#import "AutoTester-Swift.h"

@implementation PagingLayerTestCase
{
    CartoDBLightTestCase *baseCase;
    MaplyQuadPagingLoader *loader;
    MaplyOvlDebugImageLoaderInterpreter *interp;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Paging Layer";
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
    }
    return self;
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    baseCase = [[CartoDBLightTestCase alloc] init];
    [baseCase setUpWithGlobe:globeVC];
    
    [self setupPagingLayer: globeVC];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    baseCase = [[CartoDBLightTestCase alloc] init];
    [baseCase setUpWithMap:mapVC];
    
    [self setupPagingLayer: mapVC];
}

const bool enableTest = false;

- (void) setupPagingLayer:(MaplyBaseViewController*) baseViewC
{
    // Describes how we want to break things down
    MaplySamplingParams *params = [[MaplySamplingParams alloc] init];
    // TODO: Try this at minZoom = 4 and debug
    params.minZoom = 0;
    params.maxZoom = 22;
    params.minImportance = 256*256;
    params.singleLevel = true;
    params.coordSys = [[MaplySphericalMercator alloc] initWebStandard];

    // This will put an outline around a tile and a number in the middle
    interp = [[MaplyOvlDebugImageLoaderInterpreter alloc] initWithViewC:baseViewC];
    
    // Starts loading up the quad tree
    // With no tileInfo object, it doesn't bother to fetch anything
    loader = [[MaplyQuadPagingLoader alloc] initWithParams:params tileInfo:nil loadInterp:interp viewC:baseViewC];

//    [self forceReload];
}

- (void)forceReload
{
    [loader reload];
    
    PagingLayerTestCase * __weak weakSelf = self;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3.0 * NSEC_PER_SEC)), dispatch_get_main_queue(),
    ^{
        [weakSelf forceReload];
    });
}

@end
