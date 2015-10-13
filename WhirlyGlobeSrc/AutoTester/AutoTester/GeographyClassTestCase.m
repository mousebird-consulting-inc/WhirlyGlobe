//
//  GeographyClassTestCase.m
//  AutoTester
//
//  Created by jmWork on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "GeographyClassTestCase.h"

@implementation GeographyClassTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Geography Class";
	}

	return self;
}


- (void)runTest
{

	if (self.resultBlock) {
		self.resultBlock(NO);
	}
}


@end
