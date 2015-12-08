//
//  MaplyTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "MaplyTestCase.h"
#import "WhirlyGlobeComponent.h"
#import "MaplyComponent.h"


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

	dispatch_group_t lock = dispatch_group_create();

	if (self.options & MaplyTestCaseOptionGlobe) {
		dispatch_group_enter(lock);

		[self runGlobeTestWithLock:lock];
	}

	if (self.options & MaplyTestCaseOptionMap) {
		dispatch_group_enter(lock);

		[self runMapTestWithLock:lock];
	}

	dispatch_group_notify(lock,dispatch_get_main_queue(),^{
		self.running = NO;

		if (self.resultBlock) {
			self.resultBlock(self);
		}
	});

}

- (void)runGlobeTestWithLock:(dispatch_group_t)lock
{
	// create and prepare the controller
	self.globeViewController = [[WhirlyGlobeViewController alloc] init];
	[self.testView addSubview:self.globeViewController.view];
	self.globeViewController.view.frame = self.testView.bounds;
	self.globeViewController.clearColor = [UIColor blackColor];
	self.globeViewController.frameInterval = 2;

	// setup test case specifics
	if (![self setUpWithGlobe:self.globeViewController]) {
		[self tearDownWithGlobe:self.globeViewController];

		[self.globeViewController.view removeFromSuperview];
		self.globeViewController = nil;

		dispatch_group_leave(lock);

		return;
	}

	[self runTestWithGlobe:self.globeViewController result: ^(BOOL passed) {
		if (!passed || self.captureDelay == -1) {
			[self finishGlobeTestWithActualImage:nil passed:passed];
			dispatch_group_leave(lock);
		}
		else {
			__weak MaplyTestCase *weak_self = self;

			// TODO don't use main queue. The capture is blocking!
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, self.captureDelay * NSEC_PER_SEC), dispatch_get_main_queue(), ^{

				NSString *snapshot = [weak_self captureScreenFromVC:self.globeViewController];

				[weak_self finishGlobeTestWithActualImage:snapshot
												   passed:passed && (_globeResult.actualImageFile != nil)];
				dispatch_group_leave(lock);
			});
		}
	}];
}

- (void)runMapTestWithLock:(dispatch_group_t)lock
{
	// create and prepare the controller
	self.mapViewController = [[MaplyViewController alloc] initWithMapType:MaplyMapTypeFlat];
	[self.testView addSubview:self.mapViewController.view];
	self.mapViewController.view.frame = self.testView.bounds;
	self.mapViewController.clearColor = [UIColor blackColor];
	self.mapViewController.frameInterval = 2;

	// setup test case specifics
	if (![self setUpWithMap:self.mapViewController]) {
		[self tearDownWithMap:self.mapViewController];

		[self.mapViewController.view removeFromSuperview];
		self.mapViewController = nil;

		dispatch_group_leave(lock);

		return;
	}

	[self runTestWithMap:self.mapViewController result: ^(BOOL passed) {
		if (!passed || self.captureDelay == -1) {
			[self finishMapTestWithActualImage:nil passed:passed];
			dispatch_group_leave(lock);
		}
		else {
			__weak MaplyTestCase *weak_self = self;

			// TODO don't use main queue. The capture is blocking!
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, self.captureDelay * NSEC_PER_SEC), dispatch_get_main_queue(), ^{

				NSString *snapshot = [weak_self captureScreenFromVC:self.mapViewController];

				[weak_self finishMapTestWithActualImage:snapshot
												 passed:passed && (_mapResult.actualImageFile != nil)];
				dispatch_group_leave(lock);
			});
		}
	}];
}


- (void)finishGlobeTestWithActualImage:(NSString *)actualImage passed:(BOOL)passed
{
	_globeResult = [[MaplyTestResult alloc] initWithTestName:self.name
													baseline:[self baselineGlobeImageFile]
													  actual:actualImage
													  passed:passed];

	[self tearDownWithGlobe:self.globeViewController];

	[self.globeViewController.view removeFromSuperview];
	self.globeViewController = nil;
}


- (void)finishMapTestWithActualImage:(NSString *)actualImage passed:(BOOL)passed
{
	_mapResult = [[MaplyTestResult alloc] initWithTestName:self.name
												  baseline:[self baselineMapImageFile]
													actual:actualImage
													passed:passed];

	[self tearDownWithMap:self.mapViewController];

	[self.mapViewController.view removeFromSuperview];
	self.mapViewController = nil;
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	return NO;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
	return NO;
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController *)globeVC
{
}

- (void)tearDownWithMap:(MaplyViewController *)mapVC
{
}

- (NSString *)baselineGlobeImageFile
{
	NSString *name = [NSString stringWithFormat:@"%@-globe", [self testClassName]];
	return [[NSBundle mainBundle] pathForResource:name ofType:@"png"];
}

- (NSString *)baselineMapImageFile
{
	NSString *name = [NSString stringWithFormat:@"%@-map", [self testClassName]];
	return [[NSBundle mainBundle] pathForResource:name ofType:@"png"];
}

- (NSString *)testClassName
{
	NSString *className = NSStringFromClass(self.class);
	if ([className hasPrefix:@"AutoTester."]) {
		className = [className componentsSeparatedByString:@"."][1];
	}

	return className;
}

- (void)runTestWithGlobe:(WhirlyGlobeViewController *)globeVC result:(void (^)(BOOL passed))block
{
	block(YES);
}

- (void)runTestWithMap:(MaplyViewController *)globeVC result:(void (^)(BOOL passed))block
{
	block(YES);
}

- (NSString *)captureScreenFromVC:(MaplyBaseViewController *)vc
{
	UIImage *screenshot = vc.snapshot;

	if (screenshot) {
		NSString *dir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
		//NSLog(@"------>%@", dir);
		NSData *imgData = UIImagePNGRepresentation(screenshot);

		NSString *guid = [[NSProcessInfo processInfo] globallyUniqueString] ;
		NSString *fileName = [NSString stringWithFormat:@"%@-%@", [self testClassName], guid];
		dir = [dir stringByAppendingPathComponent:@"results"];
		fileName = [dir stringByAppendingPathComponent:fileName];

		[imgData writeToFile:fileName atomically:YES];

		return fileName;
	}

	NSLog(@"An error happened capturing the screen");
	return nil;
}


@end
