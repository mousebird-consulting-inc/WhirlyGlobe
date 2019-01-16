//
//  AerisWeatherTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 3/11/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "AerisWeatherTestCase.h"
#import "MaplyAerisTiles.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "AutoTester-Swift.h"

@implementation AerisWeatherTestCase {
    NSString *aerisID;      // The ID for the Aeris account.
    NSString *aerisKey;     // The secret key for the Aeris account.
    NSString *layerCode;    // The code that Aeris uses to identify the layer.
    float importanceScale;
    MaplyAerisLayerInfo *layerInfo;
    MaplyAerisTileSet *layerTileSet;
    MaplyQuadImageFrameLoader *frameLoader;
    MaplyQuadImageFrameAnimator *animator;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Aeris Weather";
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        aerisID = @"2kDDnD7Q1XFfFm4CwH17C";
        aerisKey = @"FQmadjUccN3CnB4KG6kKeurUpxHSKM0xbCd6TlVi";
        layerCode = @"sat-global";
        importanceScale = 1.0/8.0; // 1/8 works well for the weather imagery layers.
        
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
    
    // Construct the MaplyAerisTiles object to access information on available layers.
    MaplyAerisTiles *aerisTiles = [[MaplyAerisTiles alloc] initWithAerisID:aerisID secretKey:aerisKey];
    
    // Retrieve the layer information.
    NSDictionary *layerInfoDict = [aerisTiles layerInfo];
    layerInfo = layerInfoDict[layerCode];

    // Construct the MaplyAerisTileSet object, identifying the desired layer and number of frames.
    layerTileSet = [[MaplyAerisTileSet alloc] initWithAerisID:aerisID secretKey:aerisKey layerInfo:layerInfo tileSetCount:8];
    
    // Start the MaplyAerisTileSet fetch, providing custom code in the success block for adding the imagery to the map or globe.
    [layerTileSet startFetchWithSuccess:^(NSArray *tileSources) {
        if ([tileSources count] == 0)
            return;
        
        MaplyRemoteTileInfoNew *firstSource = [tileSources objectAtIndex:0];
        
        // Parameters describing how we want a globe broken down
        MaplySamplingParams *sampleParams = [[MaplySamplingParams alloc] init];
        sampleParams.coordSys = [[MaplySphericalMercator alloc] initWebStandard];
        sampleParams.coverPoles = true;
        sampleParams.edgeMatching = true;
        sampleParams.minZoom = firstSource.minZoom;
        sampleParams.maxZoom = firstSource.maxZoom;
        sampleParams.singleLevel = true;
        
        self->frameLoader = [[MaplyQuadImageFrameLoader alloc] initWithParams:sampleParams tileInfos:tileSources viewC:vc];
        self->animator = [[MaplyQuadImageFrameAnimator alloc] initWithFrameLoader:self->frameLoader viewC:vc];
        self->animator.period = 5.0;
    } failure:^(NSError *error) {
    }];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBLightTestCase *baseView = [[CartoDBLightTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBLightTestCase *baseView = [[CartoDBLightTestCase alloc] init];
    [baseView setUpWithMap:mapVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) time:0.0];
}

@end

