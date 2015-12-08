//
//  ScreenMarkersTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 30/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "ScreenMarkersTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyMarker.h"
#import "VectorsTestCase.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyViewController.h"

@implementation ScreenMarkersTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Screen Markers";
		self.captureDelay = 4;
	}
	return self;
}


- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	VectorsTestCase * baseView = [[VectorsTestCase alloc]init];
	[baseView setUpWithGlobe:globeVC];
	[self insertMarker:baseView.compList theView:(MaplyBaseViewController*)globeVC];
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];

	return true;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
	VectorsTestCase * baseView = [[VectorsTestCase alloc]init];
	[baseView setUpWithMap:mapVC];
	[self insertMarker:baseView.compList theView:(MaplyBaseViewController*)mapVC];
	[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];

	return true;
}

- (void) insertMarker:(NSMutableArray*) arrayComp theView: (MaplyBaseViewController*) theView
{
	CGSize size = CGSizeMake(0.05, 0.05);
	UIImage *alcohol = [UIImage imageNamed:@"alcohol-shop-24@2x"];
	NSMutableArray *markers = [NSMutableArray array];
	for (MaplyVectorObject* object in arrayComp) {
		MaplyMarker *marker = [[MaplyMarker alloc]init];
		marker.image = alcohol;
		marker.loc = object.center;
		marker.size = size;
		marker.userObject = object.userObject;
		[markers addObject:marker];
	}
	self.markersObj = [theView addMarkers:markers desc:nil];
}

@end
