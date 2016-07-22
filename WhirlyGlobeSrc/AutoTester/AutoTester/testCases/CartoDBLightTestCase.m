//
//  CartoDBLightTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 3/15/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "WhirlyGlobeComponent.h"
#import "CartoDBLightTestCase.h"

@implementation CartoDBLightTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"CartoDB Light";
        self.captureDelay = 5;
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
    }
    
    return self;
}

- (void) setupBaseLayer:(MaplyBaseViewController *) baseLayer
{
    NSString * baseCacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString * cartodbTilesCacheDir = [NSString stringWithFormat:@"%@/cartodbtiles/", baseCacheDir];
    int maxZoom = 22;
    MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:@"http://light_all.basemaps.cartocdn.com/light_all/" ext:@"png" minZoom:0 maxZoom:maxZoom];
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
