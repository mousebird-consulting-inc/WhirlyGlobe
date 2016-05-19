//
//  MapquestSatellite.m
//  AutoTester
//
//  Created by Steve Gifford on 3/15/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "WhirlyGlobeComponent.h"
#import "MapquestSatelliteTestCase.h"

@implementation MapquestSatelliteTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Mapquest Satellite";
        self.captureDelay = 5;
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
    }
    
    return self;
}

- (void) setupBaseLayer:(MaplyBaseViewController *) baseLayer
{
    NSString * baseCacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString * cartodbTilesCacheDir = [NSString stringWithFormat:@"%@/cartodbtiles/", baseCacheDir];
    int maxZoom = 18;
    MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/sat/" ext:@"png" minZoom:0 maxZoom:maxZoom];
    tileSource.cacheDir = cartodbTilesCacheDir;
    MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
    if ([baseLayer isKindOfClass:[WhirlyGlobeViewController class]]) {
        layer.handleEdges = true;
        layer.coverPoles = true;
    }
    else {
        layer.handleEdges = false;
        layer.coverPoles = false;
    }
    layer.requireElev = false;
    layer.waitLoad = false;
    layer.singleLevelLoading = false;
    layer.drawPriority = 0;
    [baseLayer addLayer:layer];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    [self setupBaseLayer:mapVC];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    [self setupBaseLayer:globeVC];
}

@end
