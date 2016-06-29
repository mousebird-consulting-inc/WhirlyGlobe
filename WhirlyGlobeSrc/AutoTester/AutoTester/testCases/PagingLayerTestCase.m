//
//  PagingLayerTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 6/28/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "PagingLayerTestCase.h"
#import "MapquestSatelliteTestCase.h"

@implementation PagingLayerTestCase
{
    MaplyPagingVectorTestTileSource *tileSource;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.captureDelay = 7;
        self.name = @"Paging Layer";
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
    }
    return self;
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    MapquestSatelliteTestCase *baseLayer = [[MapquestSatelliteTestCase alloc] init];
    [baseLayer setUpWithGlobe:globeVC];
    
    [self setupPagingLayer: globeVC];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    MapquestSatelliteTestCase *baseLayer = [[MapquestSatelliteTestCase alloc] init];
    [baseLayer setUpWithMap:mapVC];
    
    [self setupPagingLayer: mapVC];
}

- (void) setupPagingLayer:(MaplyBaseViewController*) baseLayer
{
    MaplySphericalMercator *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    tileSource = [[MaplyPagingVectorTestTileSource alloc] initWithCoordSys:coordSys minZoom:4 maxZoom:22];
    MaplyQuadPagingLayer *quadLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:coordSys delegate:tileSource];
    quadLayer.singleLevelLoading = true;
    quadLayer.useTargetZoomLevel = true;
    quadLayer.importance = 128*128;
    [baseLayer addLayer:quadLayer];
}

@end
