//
//  CesiumElevationTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 24/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "CesiumElevationTestCase.h"

@implementation CesiumElevationTestCase


- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Celsium Elevation";
		self.captureDelay = 2;
	}
    
	return self;
}


- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	return true;
}

@end
