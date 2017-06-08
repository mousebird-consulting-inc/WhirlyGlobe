//
//  LIDARTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 1/24/17.
//  Copyright © 2017 mousebird consulting. All rights reserved.
//

#import "LIDARTestCase.h"
#import "WhirlyGlobeComponent.h"

@implementation LIDARTestCase
{
    MaplyShader *pointShaderRamp,*pointShaderColor;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.captureDelay = 7;
        self.name = @"LIDAR Stadium";
        self.implementations = MaplyTestCaseImplementationGlobe;
    }
    return self;
}

// Look for a specific file in the bundle or in the doc dir
- (NSString *)findFile:(NSString *)base ext:(NSString *)ext
{
    NSString *docDir = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    NSString *dbPath = [[docDir stringByAppendingPathComponent:base] stringByAppendingPathExtension:ext];
    if (![[NSFileManager defaultManager] fileExistsAtPath:dbPath])
    {
        dbPath = [[NSBundle mainBundle] pathForResource:base ofType:ext];
        if (![[NSFileManager defaultManager] fileExistsAtPath:dbPath])
            dbPath = nil;
    }
    
    return dbPath;
}

// Generate a standard color ramp
- (UIImage *)generateColorRamp
{
    MaplyColorRampGenerator *rampGen = [[MaplyColorRampGenerator alloc] init];
    [rampGen addHexColor:0x5e03e1];
    [rampGen addHexColor:0x2900fb];
    [rampGen addHexColor:0x0053f8];
    [rampGen addHexColor:0x02fdef];
    [rampGen addHexColor:0x00fe4f];
    [rampGen addHexColor:0x33ff00];
    [rampGen addHexColor:0xefff01];
    [rampGen addHexColor:0xfdb600];
    [rampGen addHexColor:0xff6301];
    [rampGen addHexColor:0xf01a0a];
    
    return [rampGen makeImage:CGSizeMake(256.0,1.0)];
}

- (UIImage *)generateGrayRamp
{
    MaplyColorRampGenerator *rampGen = [[MaplyColorRampGenerator alloc] init];
    [rampGen addHexColor:0x000000];
    [rampGen addHexColor:0xffffff];
    
    return [rampGen makeImage:CGSizeMake(256.0,1.0)];
}

// Maximum number of points we'd like to display
static int MaxDisplayedPoints = 3000000;

- (void)addBaseLayer:(WhirlyGlobeViewController *)globeVC
{
    // Add a base layer
    NSString * baseCacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString * cacheDir = [NSString stringWithFormat:@"%@/cartodb_light/", baseCacheDir];
    int maxZoom = 22;
    MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:@"http://light_all.basemaps.cartocdn.com/light_all/" ext:@"png" minZoom:0 maxZoom:maxZoom];
    tileSource.cacheDir = cacheDir;
    MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
    layer.handleEdges = true;
    layer.coverPoles = true;
    layer.drawPriority = 0;
    layer.color = [UIColor colorWithWhite:0.5 alpha:1.0];
    [globeVC addLayer:layer];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    // Give us a tilt
    [globeVC setTiltMinHeight:0.001 maxHeight:0.01 minTilt:1.21771169 maxTilt:0.0];
    
    globeVC.tiltGesture = true;

    // Base layer from CartoDB
    [self addBaseLayer:globeVC];
    
    // Shader Shaders for color and ramp versions
    pointShaderColor = MaplyLAZBuildPointShader(globeVC);
    pointShaderRamp = MaplyLAZBuildRampPointShader(globeVC,[self generateColorRamp]);
    
    // Add the database
    NSString *path = [[NSBundle mainBundle] pathForResource:@"stadium-utm-quad-data" ofType:@"sqlite"];
    [self addLaz:globeVC path:path rampShader:pointShaderRamp regularShader:pointShaderColor desc:@{kMaplyLAZReaderColorScale: @(255.0)}];
}

- (void)addLaz:(WhirlyGlobeViewController *)globeVC path:(NSString *)dbPath rampShader:(MaplyShader *)rampShader regularShader:(MaplyShader *)regShader desc:(NSDictionary *)desc
{
    // Set up the paging logic
    //        quadDelegate = [[LAZQuadReader alloc] initWithDB:lazPath indexFile:indexPath];
    MaplyCoordinate3dD ll,ur;
    MaplyLAZQuadReader *quadDelegate = [[MaplyLAZQuadReader alloc] initWithDB:dbPath desc:desc viewC:globeVC];
    if (quadDelegate.hasColor)
        quadDelegate.shader = regShader;
    else
        quadDelegate.shader = rampShader;
    [quadDelegate getBoundsLL:&ll ur:&ur];
    
    // Start location
    WhirlyGlobeViewControllerAnimationState *viewState = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    viewState.heading = -3.118891;
    viewState.height = 0.00004537477798294276;
    viewState.tilt   = 0.988057;
//    viewState.roll = M_PI/8;
    MaplyCoordinate center = [[quadDelegate coordSys] localToGeo:[quadDelegate getCenter]];
    viewState.pos = MaplyCoordinateDMake(center.x,center.y);
    [globeVC setViewState:viewState];
    
    MaplyQuadPagingLayer *lazLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:quadDelegate.coordSys delegate:quadDelegate];
    // It takes no real time to fetch from the database.
    // All the threading is in projecting coordinates
    lazLayer.numSimultaneousFetches = 4;
    lazLayer.maxTiles = [quadDelegate getNumTilesFromMaxPoints:MaxDisplayedPoints];
    lazLayer.importance = 128*128;
    lazLayer.minTileHeight = ll.z;
    lazLayer.maxTileHeight = ur.z;
    lazLayer.useParentTileBounds = false;
    lazLayer.singleLevelLoading = false;
    [globeVC addLayer:lazLayer];
}

@end
