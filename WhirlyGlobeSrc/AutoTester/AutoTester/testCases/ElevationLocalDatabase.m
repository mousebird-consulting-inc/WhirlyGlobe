//
//  ElevationLocalDatabase.m
//  AutoTester
//
//  Created by jmnavarro on 10/12/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "ElevationLocalDatabase.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyElevationDatabase.h"
#import "MaplyRemoteTileElevationSource.h"
#import "MaplyAnimationTestTileSource.h"

@implementation ElevationLocalDatabase

- (instancetype)init
{
	if (self = [super init]) {
		self.captureDelay = 3;
		self.name = @"Elevation (local)";
	}
	return self;
}


-(BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	[globeVC setTiltMinHeight:0.001 maxHeight:0.04 minTilt:1.40 maxTilt:0.0];
	globeVC.frameInterval = 2;
	MaplyElevationDatabase *elevSource = [[MaplyElevationDatabase alloc] initWithName:@"world_web_mercator"];
	globeVC.elevDelegate = elevSource;
	
	// Don't forget to turn on the z buffer permanently
	[globeVC setHints:@{
			kMaplyRenderHintZBuffer: @(YES)
		}];
	
	// Set up their odd tiling system
	MaplyCesiumCoordSystem *cesiumCoordSys = [[MaplyCesiumCoordSystem alloc] init];
	MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:cesiumCoordSys minZoom:1 maxZoom:16 depth:1];
	tileSource.useDelay = false;
	tileSource.transparentMode = false;
	tileSource.pixelsPerSide = 128;
	MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
	layer.requireElev = true;
	layer.maxTiles = 256;
	layer.handleEdges = true;
	layer.numSimultaneousFetches = 8;
	[globeVC addLayer:layer];
	layer.drawPriority = 100;
	globeVC.height = 0.5;
	//50.766733, -71.732629
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-71.732629, 50.766733) time:1.0];
	
	return true;
}

@end
