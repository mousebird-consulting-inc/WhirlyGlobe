//
//  CesiumElevationTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 24/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "CesiumElevationTestCase.h"
#import "MaplyPagingElevationTestTileSource.h"
#import "MaplyRemoteTileElevationSource.h"
#import "MaplyCoordinateSystem.h"
#import "WhirlyGlobeComponent.h"
#import "GeographyClassTestCase.h"



@implementation CesiumElevationTestCase


- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Celsium Elevation";
		self.captureDelay = 5;
	}

	return self;
}


- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	GeographyClassTestCase *gctc = [[GeographyClassTestCase alloc] init];
	[gctc setUpWithGlobe:globeVC];

	[globeVC setTiltMinHeight:0.001 maxHeight:0.04 minTilt:1.40 maxTilt:0.0];
	globeVC.frameInterval = 2;  // 30fps

	globeVC.clearColor = [UIColor colorWithWhite:0.5 alpha:1.0];

	// Cesium as an elevation source
	MaplyRemoteTileElevationCesiumSource *cesiumElev = [[MaplyRemoteTileElevationCesiumSource alloc] initWithBaseURL:@"http://cesiumjs.org/stk-terrain/tilesets/world/tiles/" ext:@"terrain" minZoom:0 maxZoom:16];

	NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];

	cesiumElev.cacheDir = [NSString stringWithFormat:@"%@/cesiumElev/",cacheDir];

	globeVC.elevDelegate = cesiumElev;

	// Don't forget to turn on the z buffer permanently
	[globeVC setHints:@{kMaplyRenderHintZBuffer: @(YES)}];

	// Set up their odd tiling system
	MaplyCesiumCoordSystem *cesiumCoordSys = [[MaplyCesiumCoordSystem alloc] init];
	MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:cesiumCoordSys minZoom:1 maxZoom:16 depth:1];
	tileSource.useDelay = false;
	tileSource.transparentMode = false;
	tileSource.pixelsPerSide = 128;
	MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
	layer.requireElev = true;
	layer.maxTiles = 128;
	layer.handleEdges = true;
	layer.numSimultaneousFetches = 8;
	[globeVC addLayer:layer];
	layer.drawPriority = 0;//BaseEarthPriority;

	//Animate slowly into position
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:5.0];

	//Animate to another position
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(151.211111, -33.859972) time:1.0];

	return true;
}

@end
