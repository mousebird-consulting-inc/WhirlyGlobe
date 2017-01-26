//
//  TestResult.m
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "MaplyTestResult.h"

@implementation MaplyTestResult

- (instancetype)initWithTestName:(NSString *)testName
						baseline:(NSString *)baseline
						  actual:(NSString *)actual
						  passed:(BOOL)passed
{
	if (self = [super init]) {
		_testName = testName;
		_baselineImageFile = baseline;
		_actualImageFile = actual;
		_passed = passed;
	}
	return self;
}


@end
