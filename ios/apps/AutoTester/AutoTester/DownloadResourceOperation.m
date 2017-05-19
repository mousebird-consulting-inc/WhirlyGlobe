//
//  DownloadResourceOperation.m
//  AutoTester
//
//  Created by jmnavarro on 13/5/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "DownloadResourceOperation.h"

@interface DownloadResourceOperation()

@property (nonatomic, strong) NSURL *url;
@property (nonatomic, strong) MaplyTestCase *test;

@end


@implementation DownloadResourceOperation

- (id) initWithUrl:(NSURL *)url test:(MaplyTestCase*)test
{
	if (self = [super init]) {
		self.url = url;
		self.test = test;
	}
	return self;
}

- (void)hideIndicator
{
	if (self.test.pendingDownload == 0){
		if (self.test.state != MaplyTestCaseStateError)
			self.test.state = MaplyTestCaseStateReady;
		if (self.test.updateProgress != nil){
			self.test.updateProgress(false);
		}
	}
}
- (void)main
{
	if (self.url == nil || self.test == nil) {
		return;
	}
	if (self.cancelled) {
		return;
	}
	
	NSFileManager *fileManager = [NSFileManager defaultManager];

	NSString *fileName = [[self.url absoluteString] lastPathComponent];
	
	if (fileName == nil) {
		self.test.state = MaplyTestCaseStateError;
		self.test.pendingDownload--;
		[self hideIndicator];
		NSLog(@"Remote resource's URL needs to include the trailing file name");
		return;
	}
	
	NSString *dir = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)[0];
	dir = [dir stringByAppendingPathComponent:@"resources"];
	if (![fileManager fileExistsAtPath:dir]){
		[fileManager createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
	}
	fileName = [dir stringByAppendingPathComponent:fileName];
	
	if ([fileManager fileExistsAtPath:fileName]){
		self.test.pendingDownload--;
		[self hideIndicator];
		NSLog(@"File already downloaded: %@", fileName);
		return;
	}
	
	NSData *data = [NSData dataWithContentsOfURL:self.url];

	if (data.length > 0) {
		if([fileManager createFileAtPath:fileName contents:data attributes:nil]){
			NSLog(@"Remote resource saved at %@", fileName);
		}
		else {
			NSLog(@"ERROR: Can't save remote resources bytes: %s", strerror(errno));
		}


		self.test.pendingDownload--;
	}
	else {
		self.test.state = MaplyTestCaseStateError;
		self.test.pendingDownload--;
	}
	[self hideIndicator];
}

@end
