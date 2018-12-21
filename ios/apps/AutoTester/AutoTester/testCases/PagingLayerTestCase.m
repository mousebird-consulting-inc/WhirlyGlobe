//
//  PagingLayerTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 6/28/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "PagingLayerTestCase.h"
#import "AutoTester-Swift.h"

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
    CartoDBLightTestCase *baseLayer = [[CartoDBLightTestCase alloc] init];
    [baseLayer setUpWithGlobe:globeVC];
    
    [self setupPagingLayer: globeVC];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBLightTestCase *baseLayer = [[CartoDBLightTestCase alloc] init];
    [baseLayer setUpWithMap:mapVC];
    
    [self setupPagingLayer: mapVC];
}

const bool enableTest = false;

- (void) setupPagingLayer:(MaplyBaseViewController*) baseLayer
{
    MaplySphericalMercator *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    tileSource = [[MaplyPagingVectorTestTileSource alloc] initWithCoordSys:coordSys minZoom:4 maxZoom:22];
    MaplyQuadPagingLayer *quadLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:coordSys delegate:tileSource];
    quadLayer.singleLevelLoading = true;
    quadLayer.useTargetZoomLevel = true;
    quadLayer.importance = 256*256;
    quadLayer.useParentTileBounds = false;
    quadLayer.maxTiles = 100;
    [baseLayer addLayer:quadLayer];
    
    if (enableTest)
        [self performSelector:@selector(runDisable:) withObject:quadLayer afterDelay:5.0];
}

- (void)runEnable:(MaplyQuadPagingLayer *)quadLayer
{
    [quadLayer setEnable:true];
    
    [self performSelector:@selector(runDisable:) withObject:quadLayer afterDelay:5.0];
}

- (void)runDisable:(MaplyQuadPagingLayer *)quadLayer
{
    [quadLayer setEnable:false];
    
    [self performSelector:@selector(runEnable:) withObject:quadLayer afterDelay:5.0];
}

@end
