//
//  MaplyTestCase.m
//  AutoTester
//
//  Created by jmWork on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "MaplyTestCase.h"
#import "WhirlyGlobeComponent.h"
#import "MaplyComponent.h"

@interface MaplyTestCase()


@end

@implementation MaplyTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.selected = YES;
	}
	return self;
}

- (void)start
{
	self.running = YES;

	[self setUp];
	[self runTestWithBlock: ^(BOOL passed) {
		if (!passed || self.captureDelay == -1) {
			_result = [[MaplyTestResult alloc] init];
			_result.testName = self.name;
			_result.actualImageFile = nil;
			_result.baselineImageFile = [self baselineImageFile];
			_result.passed = passed;

			[self finishTest];
		}
		else {
			// TODO don't use main queue. The capture is blocking!
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, self.captureDelay * NSEC_PER_SEC), dispatch_get_main_queue(), ^{

				_result = [[MaplyTestResult alloc] init];
				_result.testName = self.name;
				_result.actualImageFile = [self captureScreen];
				_result.baselineImageFile = [self baselineImageFile];
				_result.passed = passed && (_result.actualImageFile != nil);

				[self finishTest];
			});
		}
	}];
}

- (void)finishTest
{
	[self.globeViewController.view removeFromSuperview];
	self.globeViewController = nil;

	[self.mapViewController.view removeFromSuperview];
	self.mapViewController = nil;

	[self tearDown];

	self.running = NO;

	if (self.resultBlock) {
		self.resultBlock(self);
	}
}

- (void)setUp
{
}

- (void)tearDown
{
}

- (NSString *)baselineImageFile
{
	return [[NSBundle mainBundle] pathForResource:NSStringFromClass(self.class) ofType:@"png"];
}

- (void)runTestWithBlock:(void (^)(BOOL passed))block
{
	block(YES);
}

- (NSString *)captureScreen
{
	UIImage *screenshot;

	if (self.globeViewController) {
		screenshot = self.globeViewController.snapshot;
	}
	else {
		screenshot = self.mapViewController.snapshot;
	}

	if (screenshot) {
		NSString *dir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
		NSData *imgData = UIImagePNGRepresentation(screenshot);

		NSString *guid = [[NSProcessInfo processInfo] globallyUniqueString] ;
		NSString *className = NSStringFromClass(self.class);

		NSString *fileName = [NSString stringWithFormat:@"%@-%@", className, guid];
		fileName = [dir stringByAppendingPathComponent:fileName];

		[imgData writeToFile:fileName atomically:YES];

		return fileName;
	}

	NSLog(@"An error happened capturing the screen");
	return nil;
}


@end
