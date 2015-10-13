//
//  GeographyClassTestCase.m
//  AutoTester
//
//  Created by jmWork on 13/10/15.
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


- (void)setUp
{
	self.globeViewController = [[WhirlyGlobeViewController alloc] init];
	[self.testView addSubview:self.globeViewController.view];
	self.globeViewController.view.frame = self.testView.bounds;

	self.globeViewController.clearColor = [UIColor blackColor];

	self.globeViewController.frameInterval = 2;

	// set up the data source
	MaplyMBTileSource *tileSource =
		[[MaplyMBTileSource alloc] initWithMBTiles:@"geography-class_medres"];

	// set up the layer
	MaplyQuadImageTilesLayer *layer =
		[[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys
												   tileSource:tileSource];
	layer.handleEdges = true;
	layer.coverPoles = true;
	layer.requireElev = false;
	layer.waitLoad = false;
	layer.drawPriority = 0;
	layer.singleLevelLoading = false;
	[self.globeViewController addLayer:layer];

	self.globeViewController.height = 0.8;
	[self.globeViewController animateToPosition:
	 	MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
}

@end
