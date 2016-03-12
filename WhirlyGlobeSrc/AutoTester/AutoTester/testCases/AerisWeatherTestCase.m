//
//  AerisWeatherTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 3/11/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
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
    NSString *aerisID;
    NSString *aerisKey;
    NSString *layerCode;
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
        
        aerisID = @"2kDDnD7Q1XFfFm4CwH17C";
        aerisKey = @"FQmadjUccN3CnB4KG6kKeurUpxHSKM0xbCd6TlVi";
        layerCode = @"radar";
//        layerCode = @"satellite";
        importanceScale = 1.0/8.0;
        
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
    MaplyAerisTiles *aerisTiles = [[MaplyAerisTiles alloc] initWithAerisID:aerisID secretKey:aerisKey];
    NSDictionary *layerInfoDict = [aerisTiles layerInfo];
    layerInfo = layerInfoDict[layerCode];
    
    layerTileSet = [[MaplyAerisTileSet alloc] initWithAerisID:aerisID secretKey:aerisKey layerInfo:layerInfo tileSetCount:8];
    
    [layerTileSet startFetchWithSuccess:^(NSArray *tileSources) {
        
//        NSObject<MaplyTileSource> *tileSource = tileSources[0];
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

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5];

    return YES;
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)globeVC];
    
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBTestCase *baseView = [[CartoDBTestCase alloc] init];
    [baseView setUpWithMap:mapVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) time:0.0];

    return YES;
}

- (void)tearDownWithMap:(MaplyViewController * _Nonnull)mapVC
{
    [self teardownWithBaseVC:(MaplyBaseViewController *)mapVC];
    
}

@end

