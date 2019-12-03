//
//  GeographyClassTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015-2017 mousebird consulting.
//

#import "GeographyClassTestCase.h"
#import "WhirlyGlobeComponent.h"


@interface GeographyClassTestCase()

@property (nonatomic, strong) MaplyMBTileFetcher *fetcher;
@property (nonatomic, strong) MaplyQuadImageLoader *loader;

@end


@implementation GeographyClassTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Geography Class";
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
	}

	return self;
}

- (NSArray *)remoteResources {
	return nil;
/*
	return [NSArray arrayWithObjects:
			@"http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-demo.txt",
			@"http://www.cl.cam.ac.uk/~mgk25/ucs/examples/grid-cyrillic-1.txt",
			@"https://manuals.info.apple.com/en_US/macbook_retina_12_inch_early2016_essentials.pdf",
			nil];
*/
}

- (void) setup:(MaplyBaseViewController *)viewC
{
    // Reads from the MBTiles file
    _fetcher = [[MaplyMBTileFetcher alloc] initWithMBTiles:@"geography-class_medres"];
    if (!_fetcher)
        return;

    // Describes what the file covers and how deep
    MaplySamplingParams *sampleParams = [[MaplySamplingParams alloc] init];
    sampleParams.coordSys = _fetcher.coordSys;
    sampleParams.coverPoles = true;
    sampleParams.edgeMatching = true;
    sampleParams.minZoom = _fetcher.minZoom;
    sampleParams.maxZoom = _fetcher.maxZoom;
    sampleParams.singleLevel = true;

    // Actually loads the images
    _loader = [[MaplyQuadImageLoader alloc] initWithParams:sampleParams tileInfo:_fetcher.tileInfo viewC:viewC];
    [_loader setTileFetcher:_fetcher];
    _loader.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault;

    // Color changing test
//    _loader.color = [UIColor greenColor];
//    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
//        self.loader.color = [UIColor blueColor];
//    });
}


- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    [self setup:globeVC];

	globeVC.height = 0.8;
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    [self setup:mapVC];

	mapVC.height = 0.8;
	[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
}

- (void)stop
{
    [self.loader shutdown];
    [self.fetcher shutdown];
    self.loader = nil;
    self.fetcher = nil;
}

@end
