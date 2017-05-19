//
//  CartoDBTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 6/12/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

#import "CartoDBTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyRemoteTileSource.h"
#import "WhirlyGlobeViewController.h"
#import "CartoDBLayer.h"
#import "CartoDBLightTestCase.h"

@implementation CartoDBTestCase


- (instancetype)init
{
	if (self = [super init]) {
		self.captureDelay = 7;
		self.name = @"CartoDB";
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
	}
	return self;
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	CartoDBLightTestCase *baseLayer = [[CartoDBLightTestCase alloc] init];
	[baseLayer setUpWithGlobe:globeVC];

	[self setupCartoDBLayer: globeVC];
	globeVC.height = 0.0001;
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-73.99,40.75)
							 time:1.0];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{
	CartoDBLightTestCase *baseLayer = [[CartoDBLightTestCase alloc] init];
	[baseLayer setUpWithMap:mapVC];

	[self setupCartoDBLayer: mapVC];
	mapVC.height = 0.0002;
	[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-73.99,40.75)
						  time:1.0];
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
