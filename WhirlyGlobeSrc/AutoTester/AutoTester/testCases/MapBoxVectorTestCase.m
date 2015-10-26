//
//  MapBoxVectorTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 26/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "MapBoxVectorTestCase.h"

@implementation MapBoxVectorTestCase


- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"MapBox Vector";
		self.captureDelay = 5;
	}

	return self;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
	return true;
}

@end
