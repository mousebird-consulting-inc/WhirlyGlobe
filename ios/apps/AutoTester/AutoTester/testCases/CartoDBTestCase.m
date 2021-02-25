//
//  CartoDBTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 6/12/15.
//  Copyright © 2015-2017 mousebird consulting.
//

#import "CartoDBTestCase.h"
#import "MaplyBaseViewController.h"
#import "WhirlyGlobeViewController.h"
#import "CartoDBInterpreter.h"
#import "AutoTester-Swift.h"

@implementation CartoDBTestCase
{
    CartoDBInterpreter *interp;
    MaplyQuadPagingLoader *loader;
    CartoDBLightTestCase *baseCase;
}

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Carto New York";
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
	}
	return self;
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	baseCase = [[CartoDBLightTestCase alloc] init];
    [baseCase setUpWithGlobe:globeVC];

	[self setupCartoDBLayer: globeVC];
    globeVC.height = 0.0001;
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-73.99,40.75)
							 time:1.0];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{
	baseCase = [[CartoDBLightTestCase alloc] init];
	[baseCase setUpWithMap:mapVC];

	[self setupCartoDBLayer: mapVC];
	mapVC.height = 0.0002;
	[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-73.99,40.75)
						  time:1.0];
}


- (void) setupCartoDBLayer:(MaplyBaseViewController*) baseViewC
{	
	NSString *search = @"SELECT the_geom,address,ownername,numfloors FROM mn_mappluto_13v1 WHERE the_geom && ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326) LIMIT 2000;";

    MaplySamplingParams *params = [[MaplySamplingParams alloc] init];
    params.minZoom = 0;
    params.maxZoom = 15;
    params.minImportance = 1024*1024;
    params.singleLevel = true;
    params.coordSys = [[MaplySphericalMercator alloc] initWebStandard];

    interp = [[CartoDBInterpreter alloc] initWithSearch:search];
    interp.minZoom = params.maxZoom;
    interp.maxZoom = params.maxZoom;

    loader = [[MaplyQuadPagingLoader alloc] initWithParams:params tileInfo:interp loadInterp:interp viewC:baseViewC];
    interp.loader = loader;
}


@end
