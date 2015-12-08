//
//  LoftedPolysTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "LoftedPolysTestCase.h"
#import "GeographyClassTestCase.h"
#import "MaplyVectorObject.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyComponentObject.h"
#import "GeographyClassTestCase.h"


@implementation LoftedPolysTestCase


- (instancetype)init
{
	if (self = [super init]) {
		self.captureDelay = 5;
		self.name = @"Lofted Polys";
	}
	return self;
}

-(void) addLoftedPolysSpain: (WhirlyGlobeViewController*) globeVC
{
	NSString *path = [[NSBundle mainBundle] pathForResource:@"ESP" ofType:@"geojson" inDirectory:nil];
	if (path){
		NSData *jsonData = [NSData dataWithContentsOfFile:path];
		if (jsonData){
			MaplyVectorObject *wgVecObj = [MaplyVectorObject VectorObjectFromGeoJSON:jsonData];
			NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
			wgVecObj.userObject = vecName;
			[globeVC addLoftedPolys:@[wgVecObj]
				key:nil
				cache:nil
				desc:@{
					kMaplyLoftedPolyHeight: @(0.1),
					kMaplyLoftedPolyOutlineColor: [UIColor redColor],
					kMaplyDrawPriority: @(kMaplyLoftedPolysDrawPriorityDefault),
					kMaplyColor: [UIColor redColor]
				}
				mode:MaplyThreadAny];
		}
	}
}

-(BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	GeographyClassTestCase *baseLayer = [[GeographyClassTestCase alloc]init];
	[baseLayer setUpWithGlobe:globeVC];
	[self addLoftedPolysSpain:globeVC];

	return true;
}

@end
