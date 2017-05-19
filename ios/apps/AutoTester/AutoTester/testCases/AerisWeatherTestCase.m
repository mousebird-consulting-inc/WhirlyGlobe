//
//  AerisWeatherTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 3/11/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "AerisWeatherTestCase.h"
#import "CartoDBTestCase.h"
#import "MaplyAerisTiles.h"
#import "MaplyQuadImageTilesLayer.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyMultiplexTileSource.h"

@implementation AerisWeatherTestCase {
    NSString *aerisID;      // The ID for the Aeris account.
    NSString *aerisKey;     // The secret key for the Aeris account.
    NSString *layerCode;    // The code that Aeris uses to identify the layer.
    float importanceScale;
    __block MaplyQuadImageTilesLayer *aerisLayer;
    MaplyAerisLayerInfo *layerInfo;
    MaplyAerisTileSet *layerTileSet;
    
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Aeris Weather";
        self.captureDelay = 2;
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
        
        MaplyMultiplexTileSource *multiSource = [[MaplyMultiplexTileSource alloc] initWithSources:tileSources];
       
        aerisLayer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:multiSource.coordSys tileSource:multiSource];
        aerisLayer.imageDepth = (int)[tileSources count];
        aerisLayer.imageFormat = MaplyImageUShort5551;
        aerisLayer.drawPriority = 1000;
        aerisLayer.importanceScale = importanceScale;
        aerisLayer.animationPeriod = 5.0;
        
        [vc addLayer:aerisLayer];
        
    } failure:^(NSError *error) {
    }];
}

- (void)teardownWithBaseVC:(MaplyBaseViewController *)vc {
    if (aerisLayer)
        [vc removeLayer:aerisLayer];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5];
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)globeVC];
    
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithMap:mapVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) time:0.0];
}

- (void)tearDownWithMap:(MaplyViewController * _Nonnull)mapVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)mapVC];
}

@end

