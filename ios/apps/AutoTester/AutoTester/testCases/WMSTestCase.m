//
//  WMSTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 4/7/16.
//  Copyright © 2016-2017 mousebird consulting.
//

#import "WMSTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"

#import "MaplyWMSTileSource.h"
#import "DDXML.h"
#import "AutoTester-Swift.h"

@implementation WMSTestCase {
    MaplyBaseViewController *_baseVC;
    MaplyQuadImageLoader *imageLoader;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"WMS Test";
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
    _baseVC = vc;
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
    [self fetchWMSLayer:@"http://129.206.228.72/cached/osm" layer:@"osm_auto:all" style:nil cacheDir:cacheDir];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBLightTestCase *baseView = [[CartoDBLightTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    globeVC.height = 0.25;
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
    [globeVC setPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBLightTestCase *baseView = [[CartoDBLightTestCase alloc] init];
    [baseView setUpWithMap:mapVC];
    mapVC.height = 0.25;
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) time:0.0];
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

        // Parameters describing how we want a globe broken down
        MaplySamplingParams *sampleParams = [[MaplySamplingParams alloc] init];
        sampleParams.coordSys = coordSys;
        sampleParams.coverPoles = false;
        sampleParams.edgeMatching = false;
        sampleParams.minZoom = tileSource.minZoom;
        sampleParams.maxZoom = tileSource.maxZoom;
        sampleParams.singleLevel = true;
        
        imageLoader = [[MaplyQuadImageLoader alloc] initWithParams:sampleParams tileInfos:@[tileSource] viewC:_baseVC];
        imageLoader.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault+1000;
    }
    
    return true;
}


@end
