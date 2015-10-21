//
//  GeographyClassTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "GeographyClassTestCase.h"
#import "WhirlyGlobeComponent.h"


@implementation GeographyClassTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Geography Class";
		self.captureDelay = 2;
	}

	return self;
}


- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	// set up the data source
	MaplyMBTileSource *tileSource =
		[[MaplyMBTileSource alloc] initWithMBTiles:@"geography-class_medres"];

	// set up the layer
	MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithTileSource:tileSource];
	layer.handleEdges = true;
	layer.coverPoles = true;
	layer.requireElev = false;
	layer.waitLoad = false;
	layer.drawPriority = 0;
	layer.singleLevelLoading = false;
	[globeVC addLayer:layer];

	globeVC.height = 0.8;
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];

	return YES;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
	// set up the data source
	MaplyMBTileSource *tileSource =
	[[MaplyMBTileSource alloc] initWithMBTiles:@"geography-class_medres"];

	// set up the layer
	MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithTileSource:tileSource];
	layer.handleEdges = true;
	layer.coverPoles = true;
	layer.requireElev = false;
	layer.waitLoad = false;
	layer.drawPriority = 0;
	layer.singleLevelLoading = false;
	[mapVC addLayer:layer];

	mapVC.height = 0.8;
	[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];

	return YES;
}

@end
