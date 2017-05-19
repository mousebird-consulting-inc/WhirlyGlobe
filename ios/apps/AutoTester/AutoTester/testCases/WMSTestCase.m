//
//  WMSTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 4/7/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "WMSTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"
#import "CartoDBTestCase.h"

#import "MaplyWMSTileSource.h"
#import "DDXML.h"

@implementation WMSTestCase {
    MaplyBaseViewController *_baseVC;
    MaplyQuadImageTilesLayer *_imageLayer;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"WMS Test";
        self.captureDelay = 2;
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
    _baseVC = vc;
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
    [self fetchWMSLayer:@"http://129.206.228.72/cached/osm" layer:@"osm_auto:all" style:nil cacheDir:cacheDir];
}

- (void)teardownWithBaseVC:(MaplyBaseViewController *)vc {
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

// Try to fetch the given WMS layer
- (void)fetchWMSLayer:(NSString *)baseURL layer:(NSString *)layerName style:(NSString *)styleName cacheDir:(NSString *)thisCacheDir
{
    NSString *capabilitiesURL = [MaplyWMSCapabilities CapabilitiesURLFor:baseURL];
    
    NSURLSession *session = [NSURLSession sharedSession];
    NSURLSessionDataTask *task = [session dataTaskWithURL:[NSURL URLWithString:capabilitiesURL] completionHandler:
    ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        NSError *xmlError;
        DDXMLDocument *xmlDoc;
        if (!error) {
            xmlDoc = [[DDXMLDocument alloc]initWithData:data options:(NSUInteger)0 error:&xmlError];
        }
        if (!error && !xmlError) {
            [self startWMSLayerBaseURL:baseURL xml:xmlDoc layer:layerName style:styleName cacheDir:thisCacheDir];
        } else
            NSLog(@"Unable to fetch WMS layer:\n%@", error);
    }];
    [task resume];
    
    
    
//    AFHTTPRequestOperation *operation = [[AFHTTPRequestOperation alloc] initWithRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:capabilitiesURL]]];
//    operation.responseSerializer = [AFXMLParserResponseSerializer serializer];
//    [operation setCompletionBlockWithSuccess:^(AFHTTPRequestOperation *operation, id responseObject) {
//        [self startWMSLayerBaseURL:baseURL xml:responseObject layer:layerName style:styleName cacheDir:thisCacheDir ovlName:ovlName];
//    } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
//        // Sometimes this works anyway
//        //        if (![self startWMSLayerBaseURL:baseURL xml:XMLDocument layer:layerName style:styleName cacheDir:thisCacheDir ovlName:ovlName])
//        //            NSLog(@"Failed to get capabilities from WMS server: %@",capabilitiesURL);
//    }];
//    
//    [operation start];
}

// Try to start the layer, given the capabilities
- (bool)startWMSLayerBaseURL:(NSString *)baseURL xml:(DDXMLDocument *)XMLDocument layer:(NSString *)layerName style:(NSString *)styleName cacheDir:(NSString *)thisCacheDir
{
    // See what the service can provide
    MaplyWMSCapabilities *cap = [[MaplyWMSCapabilities alloc] initWithXML:XMLDocument];
    MaplyWMSLayer *layer = [cap findLayer:layerName];
    MaplyCoordinateSystem *coordSys = [layer buildCoordSystem];
    MaplyWMSStyle *style = [layer findStyle:styleName];
    if (!layer)
    {
        NSLog(@"Couldn't find layer %@ in WMS response.",layerName);
        return false;
    } else if (!coordSys)
    {
        NSLog(@"No coordinate system we recognize in WMS response.");
        return false;
    } else if (styleName && !style)
    {
        NSLog(@"No style named %@ in WMS response.",styleName);
        return false;
    }
    
    if (layer && coordSys)
    {
        MaplyWMSTileSource *tileSource = [[MaplyWMSTileSource alloc] initWithBaseURL:baseURL capabilities:cap layer:layer style:style coordSys:coordSys minZoom:0 maxZoom:16 tileSize:256];
        tileSource.cacheDir = thisCacheDir;
        tileSource.transparent = true;
        _imageLayer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:coordSys tileSource:tileSource];
        _imageLayer.coverPoles = false;
        _imageLayer.handleEdges = true;
        _imageLayer.requireElev = false;
        _imageLayer.waitLoad = false;
        [_baseVC addLayer:_imageLayer];
        
    }
    
    return true;
}


@end
