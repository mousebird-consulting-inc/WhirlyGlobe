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
		self.state = MaplyTestCaseStateReady;
		self.interactive = false;
		self.pendingDownload = 0;
	}

	return self;
}

- (NSArray * _Nullable)remoteResources {
	return nil;
}

- (void)fetchResources {
	NSArray * _Nullable resources = [self remoteResources];

	if (resources.count == 0) {
		return;
	}

	NSFileManager *fileManager = [NSFileManager defaultManager];

	if (self.updateProgress != nil) {
		self.updateProgress(resources.count, resources.count - self.pendingDownload);
	}

	for (int ii = 0; ii < resources.count; ii++) {
		NSArray *foo = [[resources[ii] absoluteString] componentsSeparatedByString:@"/"];
		NSString *fileName = [foo lastObject];

		if (fileName != nil) {
			NSString *dir = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)[0];
			dir = [dir stringByAppendingPathComponent:@"resources"];
			fileName = [dir stringByAppendingPathComponent:fileName];

			if (![fileManager fileExistsAtPath:fileName]) {
				[self startDownloadWithURL:resources[ii] file:fileName];
			}
		}
		else {
			//TODO
		}
	}

	if (self.pendingDownload == 0) {
		if ([[NSUserDefaults standardUserDefaults] boolForKey:_name]) {
			self.state = MaplyTestCaseStateSelected;
		}
		else {
			self.state = MaplyTestCaseStateReady;
		}

		if (self.updateProgress != nil) {
			self.updateProgress(resources.count, resources.count - self.pendingDownload);
		}
	}
}

- (void)startDownloadWithURL:(NSURL *)url file:(NSString *)fileName
{
	NSURLRequest *request = [[NSURLRequest alloc]initWithURL:url];

	[NSURLConnection sendAsynchronousRequest:request
		queue:[NSOperationQueue mainQueue]
		completionHandler:^(NSURLResponse * _Nullable response, NSData * _Nullable data, NSError * _Nullable connectionError) {

			if (connectionError != nil) {
				//TODO
			}
			else if (data != nil) {
				[data writeToFile:fileName atomically:YES];

				self.pendingDownload--;

				if (self.updateProgress != nil) {
					//self.updateProgress(self.remoteResources.count, self.remoteResources.count - self.pendingDownload);
				}

				if (self.pendingDownload == 0) {
					if ([[NSUserDefaults standardUserDefaults] boolForKey:_name]) {
						self.state = MaplyTestCaseStateSelected;
					}
					else {
						self.state = MaplyTestCaseStateReady;
					}
				}
			}
	}];
}

- (void)start
{
	if (self.state != MaplyTestCaseStateDownloading && self.state != MaplyTestCaseStateError) {
		self.state = MaplyTestCaseStateRunning;

		dispatch_group_t lock = dispatch_group_create();

		if (self.options & MaplyTestCaseOptionGlobe) {
			if (!self.interactive)
				dispatch_group_enter(lock);

			[self runGlobeTestWithLock:lock];
		}

		if (self.options & MaplyTestCaseOptionMap) {
			if (!self.interactive)
				dispatch_group_enter(lock);

			[self runMapTestWithLock:lock];
		}

		if (!self.interactive)
			dispatch_group_notify(lock,dispatch_get_main_queue(),^{
				self.state = MaplyTestCaseStateReady;

				if (self.resultBlock) {
					self.resultBlock(self);
				}
			});
	}
}

- (void)runGlobeTestWithLock:(dispatch_group_t)lock
{
	// create and prepare the controller
	self.globeViewController = [[WhirlyGlobeViewController alloc] init];
	[self.testView addSubview:self.globeViewController.view];
	self.globeViewController.view.backgroundColor = [UIColor blueColor];
	self.globeViewController.view.frame = self.testView.bounds;
	self.globeViewController.clearColor = [UIColor blackColor];
	self.globeViewController.frameInterval = 2;
	// setup test case specifics
	if (![self setUpWithGlobe:self.globeViewController]) {
		[self tearDownWithGlobe:self.globeViewController];

		[self.globeViewController.view removeFromSuperview];
		self.globeViewController = nil;

		if (!self.interactive)
			dispatch_group_leave(lock);

		return;
	}

	if (!self.interactive)
	{
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
}

- (MaplyCoordinateSystem * _Nullable)customCoordSystem
{
	return nil;
}

- (void)runMapTestWithLock:(dispatch_group_t)lock
{
	// create and prepare the controller
	self.mapViewController = [[MaplyViewController alloc] initWithMapType:MaplyMapTypeFlat];
	
	MaplyCoordinateSystem *coordSys = [self customCoordSystem];
	if (coordSys)
		self.mapViewController.coordSys = coordSys;

	[self.testView addSubview:self.mapViewController.view];
	self.mapViewController.view.frame = self.testView.bounds;
	self.mapViewController.clearColor = [UIColor blackColor];
	self.mapViewController.frameInterval = 2;

	// setup test case specifics
	if (![self setUpWithMap:self.mapViewController]) {
		[self tearDownWithMap:self.mapViewController];

		[self.mapViewController.view removeFromSuperview];
		self.mapViewController = nil;

		if (!self.interactive)
			dispatch_group_leave(lock);

		return;
	}

	if (!self.interactive)
	{
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

- (void)setSelected:(BOOL)selected
{
	if (self.state == MaplyTestCaseStateReady || self.state == MaplyTestCaseStateSelected){
		self.state = MaplyTestCaseStateSelected;
		[[NSUserDefaults standardUserDefaults] setBool:selected forKey:_name];
	}
}

- (void)setName:(NSString *)name
{
	_name = name;
}


@end
