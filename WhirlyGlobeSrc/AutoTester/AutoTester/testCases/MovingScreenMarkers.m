//
//  MovingScreenMarkers.m
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "MovingScreenMarkers.h"

@implementation MovingScreenMarkers

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Moving Screen Markers";
		self.captureDelay = 2;
	}
	
	return self;
}

@end
