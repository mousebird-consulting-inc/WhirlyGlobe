//
//  GeographyClassTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "GeographyClassTestCase.h"
#import "WhirlyGlobeComponent.h"


@interface GeographyClassTestCase()

@property (nonatomic, strong) MaplyMBTileSource *tileSource;
@property (nonatomic, strong) MaplyQuadImageTilesLayer *layer;

@end


@implementation GeographyClassTestCase



- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Geography Class";
		self.captureDelay = 2;
	}

	return self;
}

- (void) setup{
	// set up the data source
	self.tileSource =
	[[MaplyMBTileSource alloc] initWithMBTiles:@"geography-class_medres"];

	// set up the layer
	self.layer = [[MaplyQuadImageTilesLayer alloc] initWithTileSource:self.tileSource];
	self.layer.handleEdges = true;
	self.layer.coverPoles = true;
	self.layer.requireElev = false;
	self.layer.waitLoad = false;
	self.layer.drawPriority = 0;
	self.layer.singleLevelLoading = false;
}


- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	[self setup];
	[globeVC addLayer:self.layer];

	globeVC.height = 0.8;
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];

	return YES;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
	[self setup];
	[mapVC addLayer:self.layer];

	mapVC.height = 0.8;
	[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];

	return YES;
}

@end
