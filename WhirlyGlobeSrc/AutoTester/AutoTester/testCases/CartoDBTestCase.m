//
//  CartoDBTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 6/12/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "CartoDBTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyRemoteTileSource.h"
#import "WhirlyGlobeViewController.h"
#import "CartoDBLayer.h"

@implementation CartoDBTestCase


- (instancetype)init
{
	if (self = [super init]) {
		self.captureDelay = 7;
		self.name = @"CartoDB";
	}
	return self;
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	[self setupBaseLayer: globeVC];
	[self setupCartoDBLayer: globeVC];
	globeVC.height = 0.0001;
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-73.99,40.75)
							 time:1.0];

	return true;
}


- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
	[self setupBaseLayer: mapVC];
	[self setupCartoDBLayer: mapVC];
	mapVC.height = 0.0002;
	[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-73.99,40.75)
						  time:1.0];
	return true;
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

- (void) setupCartoDBLayer:(MaplyBaseViewController*) baseLayer
{	
	NSString *search = @"SELECT the_geom,address,ownername,numfloors FROM mn_mappluto_13v1 WHERE the_geom && ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326) LIMIT 2000;";
	
	CartoDBLayer *cartoLayer = [[CartoDBLayer alloc] initWithSearch:search];
	cartoLayer.minZoom = 15;
	cartoLayer.maxZoom = 15;
	MaplySphericalMercator *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
	MaplyQuadPagingLayer *quadLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:coordSys delegate:cartoLayer];
	[baseLayer addLayer:quadLayer];
}


@end
