/*
 *  TestViewController.m
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 7/23/12.
 *  Copyright 2011-2013 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <QuartzCore/QuartzCore.h>
#import "TestViewController.h"
#import "AnimationTest.h"
#import "WeatherShader.h"
#import "MaplyRemoteTileElevationSource.h"
#import "PagingTestDelegate.h"
#import "ParticleTest.h"
#ifdef NOTPODSPECWG
#import "MapzenSource.h"
#import <DDXMLDocument.h>
#endif

// Simple representation of locations and name for testing
typedef struct
{
    char name[20];
    float lat,lon;
} LocationInfo;

// Some random locations for testing.
// If we've missed your home, it's because we think you suck.
static const int NumLocations = 30;
LocationInfo locations[NumLocations] = 
{
    {"Kansas City",39.1, -94.58},
    {"Washington, DC",38.895111,-77.036667},
    {"Manila",14.583333,120.966667},
    {"Moscow",55.75, 37.616667},
    {"London",51.507222, -0.1275},
    {"Caracas",10.5, -66.916667},
    {"Lagos",6.453056, 3.395833},
    {"Sydney",-33.859972, 151.211111},
    {"Seattle",47.609722, -122.333056},
    {"Tokyo",35.689506, 139.6917},
    {"McMurdo Station",-77.85, 166.666667},
    {"Tehran",35.696111, 51.423056},
    {"Santiago",-33.45, -70.666667},
    {"Pretoria",-25.746111, 28.188056},
    {"Perth",-31.952222, 115.858889},
    {"Beijing",39.913889, 116.391667},
    {"New Delhi",28.613889, 77.208889},
    {"San Francisco",37.7793, -122.4192},
    {"Pittsburgh",40.441667, -80},
    {"Freetown",8.484444, -13.234444},
    {"Windhoek",-22.57, 17.083611},
    {"Buenos Aires",-34.6, -58.383333},
    {"Zhengzhou",34.766667, 113.65},
    {"Bergen",60.389444, 5.33},
    {"Glasgow",55.858, -4.259},
    {"Bogota",4.598056, -74.075833},
    {"Haifa",32.816667, 34.983333},
    {"Puerto Williams",-54.933333, -67.616667},
    {"Panama City",8.983333, -79.516667},
    {"Niihau",21.9, -160.166667}
};

// High performance vs. low performance devices
typedef enum {HighPerformance,LowPerformance} PerformanceMode;

// Lowest priority for base layers
static const int BaseEarthPriority = kMaplyImageLayerDrawPriorityDefault;

// Local interface for TestViewController
// We'll hide a few things here
@interface TestViewController ()//<Maply3dTouchPreviewDatasource>
{
    // The configuration view comes up when the user taps outside the globe
    ConfigViewController *configViewC;
    
    // Base layer
    NSString *baseLayerName;
    MaplyViewControllerLayer *baseLayer;
    
    // Overlay layers
    NSMutableDictionary *ovlLayers;
        
    // These represent a group of objects we've added to the globe.
    // This is how we track them for removal
    MaplyComponentObject *screenMarkersObj;
    MaplyComponentObject *markersObj;
    MaplyComponentObject *shapeCylObj;
    MaplyComponentObject *shapeSphereObj;
    MaplyComponentObject *greatCircleObj;
    MaplyComponentObject *arrowsObj;
    MaplyComponentObject *modelsObj;
    MaplyComponentObject *screenLabelsObj;
    MaplyComponentObject *labelsObj;
    MaplyComponentObject *stickersObj;
    MaplyComponentObject *latLonObj;
    NSArray *sfRoadsObjArray;
    MaplyComponentObject *arcGisObj;
    NSArray *vecObjects;
    MaplyComponentObject *megaMarkersObj;
    MaplyComponentObject *markerClusterObj;
    NSArray *megaMarkersImages;
    MaplyComponentObject *autoLabels;
    MaplyActiveObject *animSphere;
    NSMutableDictionary *loftPolyDict;
    MaplyStarsModel *stars;
    MaplyComponentObject *sunObj,*moonObj;
    MaplyAtmosphere *atmosObj;
    NSDictionary *tessValues;
    NSArray *labelTestObjs;
    
    // Paging marker test
    MaplyQuadPagingLayer *markerLayer;
    PagingTestDelegate *markerDelegate;

    // A source of elevation data, if we're in that mode
    NSObject<MaplyElevationSourceDelegate> *elevSource;
    
    // The view we're using to track a selected object
//    MaplyViewTracker *selectedViewTrack;
    
    NSDictionary *screenLabelDesc,*labelDesc,*vectorDesc;
    
    // If we're in 3D mode, how far the elevation goes
    int zoomLimit;
    bool requireElev;
    bool imageWaitLoad;
    int maxLayerTiles;

    // Label test
    NSTimer *_labelAnimationTimer;
    NSMutableDictionary *_trafficLabels;
    
    // Dashed lines used in wide vector test
    MaplyTexture *dashedLineTex,*filledLineTex;
    
    PerformanceMode perfMode;
  id <UIViewControllerPreviewing> previewingContext;

    UIScrollView *scrollView;
}

// Change what we're showing based on the Configuration
- (void)changeMapContents;
@end

@implementation TestViewController
{
    MapType startupMapType;
}

- (id)initWithMapType:(MapType)mapType
{
    self = [super init];
    if (self) {
        startupMapType = mapType;
        ovlLayers = [NSMutableDictionary dictionary];
    }
    return self;
}

- (void)dealloc
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    // This should release the globe view
    if (baseViewC)
    {
        [baseViewC.view removeFromSuperview];
        [baseViewC removeFromParentViewController];
        baseViewC = nil;
        mapViewC = nil;
        globeViewC = nil;
    }    
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // What sort of hardware are we on?
    perfMode = LowPerformance;
    if ([UIScreen mainScreen].scale > 1.0)
    {
        // Retina devices tend to be better, except for
        perfMode = HighPerformance;
    }
#if TARGET_IPHONE_SIMULATOR
    perfMode = HighPerformance;
#endif
    
    loftPolyDict = [NSMutableDictionary dictionary];
    
    // Configuration controller for turning features on and off
    configViewC = [[ConfigViewController alloc] initWithNibName:@"ConfigViewController" bundle:nil];
    configViewC.configOptions = ConfigOptionsAll;
    
    bool bngTest = false;

    // Create an empty globe or map controller
    zoomLimit = 0;
    requireElev = false;
    maxLayerTiles = 256;
    switch (startupMapType)
    {
        case MaplyGlobe:
        case MaplyGlobeWithElevation:
        case MaplyGlobeScrollView:
            globeViewC = [[WhirlyGlobeViewController alloc] init];
            globeViewC.delegate = self;
            globeViewC.inScrollView = (startupMapType == MaplyGlobeScrollView);
            baseViewC = globeViewC;
            maxLayerTiles = 128;
            // Per level tesselation control
            tessValues = @{@(-1) : @10, @0 : @20, @1 : @16};
            break;
        case Maply3DMap:
            mapViewC = [[MaplyViewController alloc] initWithMapType:MaplyMapType3D];
            mapViewC.doubleTapZoomGesture = true;
            mapViewC.twoFingerTapGesture = true;
            mapViewC.viewWrap = true;
            mapViewC.delegate = self;
            baseViewC = mapViewC;
            break;
        case Maply2DMap:
        case Maply2DScrollView:
            mapViewC = [[MaplyViewController alloc] initWithMapType:MaplyMapTypeFlat];
            mapViewC.viewWrap = true;
            mapViewC.doubleTapZoomGesture = true;
            mapViewC.twoFingerTapGesture = true;
            mapViewC.delegate = self;
            mapViewC.inScrollView = (startupMapType == Maply2DScrollView);
            baseViewC = mapViewC;
            configViewC.configOptions = ConfigOptionsFlat;
            break;
        case Maply2DBNG:
            mapViewC = [[MaplyViewController alloc] initWithMapType:MaplyMapTypeFlat];
            mapViewC.coordSys = [self buildBritishNationalGrid:true];
            mapViewC.viewWrap = false;
            mapViewC.doubleTapZoomGesture = true;
            mapViewC.twoFingerTapGesture = true;
            mapViewC.delegate = self;
            baseViewC = mapViewC;
            configViewC.configOptions = ConfigOptionsFlat;
            
            startupMapType = Maply2DMap;
            bngTest = true;
            break;
//        case MaplyScrollViewMap:
//            break;
        default:
            break;
    }

    if (startupMapType != MaplyGlobeScrollView && startupMapType != Maply2DScrollView) {
        [self.view addSubview:baseViewC.view];
        baseViewC.view.frame = self.view.bounds;
    } else {
        scrollView = [[UIScrollView alloc] initWithFrame:CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height)];

        scrollView.scrollEnabled = YES;
        scrollView.clipsToBounds = YES;
        scrollView.bounces = NO;
        scrollView.pagingEnabled = YES;
        scrollView.showsHorizontalScrollIndicator = NO;
        scrollView.showsVerticalScrollIndicator = NO;
        scrollView.delaysContentTouches = YES;
        scrollView.canCancelContentTouches = NO;

        [scrollView addSubview:baseViewC.view];

        UIView *secondView = [[UIView alloc] initWithFrame:CGRectMake(self.view.frame.size.width, 0, self.view.frame.size.width, self.view.frame.size.height)];
        secondView.backgroundColor = [UIColor redColor];

        [scrollView addSubview:secondView];

        scrollView.contentSize = CGSizeMake(self.view.frame.size.width*2, self.view.frame.size.height);

        [self.view addSubview:scrollView];
        baseViewC.view.frame = CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height);
    }

    [self addChildViewController:baseViewC];
    
    // Note: Debugging
//    if (globeViewC)
//        [globeViewC setTiltMinHeight:0.001 maxHeight:0.01 minTilt:1.21771169 maxTilt:0.0];

    // This lets us mix screen space objects with everything else
//    baseViewC.screenObjectDrawPriorityOffset = 0;

    // Note: Debugging
//    [self labelExercise];

    if (perfMode == LowPerformance)
    {
        baseViewC.frameInterval = 3; // 20fps
        baseViewC.threadPerLayer = false;
    } else {
        baseViewC.frameInterval = 2; // 30fps
        baseViewC.threadPerLayer = true;
    }
    
    // Set the background color for the globe
    if (globeViewC)
        baseViewC.clearColor = [UIColor colorWithWhite:0.8 alpha:1.0];
    else
        baseViewC.clearColor = [UIColor whiteColor];
        
    if (globeViewC)
    {
        // Limit the zoom (for sun & stars)
        float minHeight,maxHeight;
        [globeViewC getZoomLimitsMin:&minHeight max:&maxHeight];
        [globeViewC setZoomLimitsMin:minHeight max:3.0];
        
        // Start up over San Francisco
        globeViewC.height = 0.8;
        [globeViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) time:1.0];
    } else {
        mapViewC.height = 1.0;
        [mapViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) time:1.0];
    }
    
    // Note: Debugging British National Grid
    if (bngTest)
    {
        // We have to tweak the extents or we end up locked into place when we zoom out.
//        [mapViewC setViewExtentsLL:MaplyCoordinateMake(-1000000, -1000000) ur:MaplyCoordinateMake(2000000, -2000000)];

        // Add a marker near London
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.image = [UIImage imageNamed:@"map_pin"];
        marker.loc = MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222);
        marker.size = CGSizeMake(40, 40);
        marker.selectable = true;
        marker.userObject = @"London";
        [baseViewC addScreenMarkers:@[marker] desc:nil mode:MaplyThreadAny];
        
        MaplyCoordinate localLondon = [mapViewC.coordSys geoToLocal:MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222)];
        
        [mapViewC setPosition:localLondon height:2.0];
    }
    
    // Note: Debugging
//    if (globeViewC)
//    {
//        [globeViewC setFarClipPlane:10.0];
//        float minZoom,maxZoom;
//        [globeViewC getZoomLimitsMin:&minZoom max:&maxZoom];
//        [globeViewC setZoomLimitsMin:minZoom max:8.0];
//    }

    
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
    
    // For elevation mode, we need to do some other stuff
    if (startupMapType == MaplyGlobeWithElevation)
    {
        self.title = @"Cesium Terrain";
        
        // Tilt, so we can see it
        if (globeViewC)
            [globeViewC setTiltMinHeight:0.001 maxHeight:0.04 minTilt:1.40 maxTilt:0.0];
        globeViewC.frameInterval = 2;  // 30fps

        baseViewC.clearColor = [UIColor colorWithWhite:0.5 alpha:1.0];

        // Cesium as an elevation source
        MaplyRemoteTileElevationCesiumSource *cesiumElev = [[MaplyRemoteTileElevationCesiumSource alloc] initWithBaseURL:@"http://assets.agi.com/stk-terrain/tilesets/world/tiles/" ext:@"terrain" minZoom:0 maxZoom:16];
        elevSource = cesiumElev;
        cesiumElev.cacheDir = [NSString stringWithFormat:@"%@/cesiumElev/",cacheDir];
//        elevSource = [[MaplyElevationDatabase alloc] initWithName:@"world_web_mercator"];
        
        baseViewC.elevDelegate = elevSource;
        zoomLimit = 16;
        requireElev = true;
        baseViewC.elevDelegate = elevSource;
        
        // Don't forget to turn on the z buffer permanently
        [baseViewC setHints:@{kMaplyRenderHintZBuffer: @(YES)}];
        
        // Turn off most of the options for globe mode
        configViewC.configOptions = ConfigOptionsTerrain;
        
        // Set up their odd tiling system
        MaplyCesiumCoordSystem *cesiumCoordSys = [[MaplyCesiumCoordSystem alloc] init];
        MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:cesiumCoordSys minZoom:1 maxZoom:16 depth:1];
//        MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:[[MaplySphericalMercator alloc] initWebStandard] minZoom:0 maxZoom:16 depth:1];
        tileSource.useDelay = false;
        tileSource.transparentMode = false;
        tileSource.pixelsPerSide = 128;
        MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
        layer.requireElev = true;
        layer.maxTiles = 256;
        layer.handleEdges = true;
        layer.numSimultaneousFetches = 8;
        [baseViewC addLayer:layer];
        layer.drawPriority = BaseEarthPriority;
        baseLayer = layer;
        
//        // Start up over Everest
//        mapViewC.height = 1.0;
//        [mapViewC animateToPosition:MaplyCoordinateMakeWithDegrees(86.925278, 27.988056) time:1.0];
        
//        [self addSun];
//        [self addStars:@"starcatalog_short"];
    }
    
    // Force the view to load so we can get the default switch values
    [configViewC view];
    
    // Note: Testing
//    [self performSelector:@selector(findHeightTest) withObject:nil afterDelay:0.0];

    // Maximum number of objects for the layout engine to display
//    [baseViewC setMaxLayoutObjects:1000];
    
    // Bring up things based on what's turned on
    [self performSelector:@selector(changeMapContents) withObject:nil afterDelay:0.0];
    
    // Settings panel
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemEdit target:self action:@selector(showConfig)];
    
    // Test animation
//    if (globeViewC)
//        [self performSelector:@selector(viewAnimationTest) withObject:nil afterDelay:2.0];
    
//    [self performSelector:@selector(labelMarkerTest:) withObject:@(0.1) afterDelay:0.1];

//    [self wideLineTest];
  
    [baseViewC enable3dTouchSelection:self];

    if (startupMapType == MaplyGlobeScrollView || startupMapType == Maply2DScrollView) {
        for (NSNumber *dirNum in @[@(UISwipeGestureRecognizerDirectionLeft), @(UISwipeGestureRecognizerDirectionRight)]) {

            UISwipeGestureRecognizer *swipe = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(didSwipeScreen:)];
            swipe.delegate = self;
            swipe.direction = dirNum.intValue;
            swipe.delaysTouchesBegan = TRUE;
            [scrollView addGestureRecognizer:swipe];
            [baseViewC requirePanGestureRecognizerToFailForGesture:swipe];
        }
        bool panGesture = (startupMapType == MaplyGlobeScrollView && globeViewC.panGesture) || (startupMapType == Maply2DScrollView && mapViewC.panGesture);
        if (panGesture && scrollView.scrollEnabled)
            [baseViewC requirePanGestureRecognizerToFailForGesture:scrollView.panGestureRecognizer];
    }
}

- (void)billboardTest
{
    float size = 2.4;
    MaplyScreenObject *screenObj = [[MaplyScreenObject alloc] init];
    [screenObj addImage:[UIImage imageNamed:@"veins_diffuse.png"] color:[UIColor whiteColor] size:CGSizeMake(size, size)];
    [screenObj translateX:-size/2.0 y:-size/2.0];
    MaplyBillboard *bboard = [[MaplyBillboard alloc] init];
    bboard.screenObj = screenObj;
    bboard.center = MaplyCoordinate3dMake(0, 0, -EarthRadius);
    
    [baseViewC addBillboards:@[bboard] desc:@{kMaplyBillboardOrient:kMaplyBillboardOrientEye}  mode:MaplyThreadCurrent];
}

- (void)wideLineTest
{
    [self addGeoJson:@"sawtooth.geojson"];
    [self addGeoJson:@"moving-lawn.geojson"];
    [self addGeoJson:@"spiral.geojson"];
    [self addGeoJson:@"square.geojson"];
    [self addGeoJson:@"track.geojson"];
    [self addGeoJson:@"uturn2.geojson" dashPattern:@[@16, @16] width:40];
  
//    [self addGeoJson:@"straight.geojson"];
//    [self addGeoJson:@"uturn.geojson"];

    if (mapViewC)
        [mapViewC setPosition:MaplyCoordinateMakeWithDegrees(-100, 30) height:0.0046618999913334846];
}


/* Build two different versions of BNG.  One can go out larger than the other.
    If display is set, we'll allow a bigger bounding box.
 */
- (MaplyCoordinateSystem *)buildBritishNationalGrid:(bool)display
{
    // Set up the proj4 string including the local grid file
    NSString *proj4Str = [NSString stringWithFormat:@"+proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy +nadgrids=%@ +units=m +no_defs",[[NSBundle mainBundle] pathForResource:@"OSTN02_NTv2" ofType:@"gsb"]];
    MaplyProj4CoordSystem *coordSys = [[MaplyProj4CoordSystem alloc] initWithString:proj4Str];
    
    // Set the bounding box for validity.  It assumes it can go everywhere by default
    MaplyBoundingBox bbox;
    bbox.ll.x = 1393.0196;    bbox.ll.y = 13494.9764;
    bbox.ur.x = 671196.3657;    bbox.ur.y = 1230275.0454;
    
    // Now expand it out so we can see the whole of the UK
    if (display)
    {
        double spanX = bbox.ur.x - bbox.ll.x;
        double spanY = bbox.ur.y - bbox.ur.x;
        double extra = 1.0;
        bbox.ll.x -= extra*spanX;  bbox.ll.y -= extra*spanY;
        bbox.ur.x += extra*spanX;  bbox.ur.y += extra*spanY;
    }
    
    [coordSys setBounds:bbox];
    
    return coordSys;
}

- (void)markerOverlapTest
{
    int numTestMarkers = 3;
    
    // Make up a few markers
    NSMutableArray *markerImages = [NSMutableArray array];
    for (unsigned int ii=0;ii<numTestMarkers;ii++)
    {
        UIImage *image = [self randomImage];
        MaplyTexture *tex = [baseViewC addTextureToAtlas:image mode:MaplyThreadCurrent];
        [markerImages addObject:tex];
    }
    
    NSMutableArray *markers = [NSMutableArray array];
    for (unsigned int ii=0;ii<3;ii++)
    {
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.image = [markerImages objectAtIndex:random()%numTestMarkers];
        marker.size = CGSizeMake(16,16);
        marker.loc = MaplyCoordinateMakeWithDegrees(0.0,0.0);
        marker.layoutImportance = 1.0;
        [markers addObject:marker];
    }
    
    [baseViewC addScreenMarkers:markers desc:@{kMaplyClusterGroup: @(0)} mode:MaplyThreadCurrent];
}

- (void)markerTest2
{
    MaplyScreenMarker *marker1 = [[MaplyScreenMarker alloc] init];
    marker1.image = [UIImage imageNamed:@"map_pin"];
    marker1.loc = MaplyCoordinateMakeWithDegrees(12.454041, 55.643532);
    marker1.size = CGSizeMake(40, 40);
    [baseViewC addScreenMarkers:@[marker1] desc:@{kMaplyClusterGroup: @(0)} mode:MaplyThreadAny];

    MaplyScreenMarker *marker2 = [[MaplyScreenMarker alloc] init];
    marker2.image = [UIImage imageNamed:@"map_pin"];
    marker2.loc = MaplyCoordinateMakeWithDegrees(12.485252, 55.723499);
    marker2.size = CGSizeMake(40, 40);
    [baseViewC addScreenMarkers:@[marker2] desc:@{kMaplyClusterGroup: @(0)} mode:MaplyThreadAny];
}

- (void)labelMarkerTest:(NSNumber *)time
{
    if (labelTestObjs)
    {
        [baseViewC removeObjects:labelTestObjs mode:MaplyThreadCurrent];
        labelTestObjs = nil;
    }
    
    UIImage *redSquare = [UIImage imageNamed:@"redsquare"];
    UIImage *blueSquare = [UIImage imageNamed:@"bluesquare"];
    CGSize redSize = CGSizeMake(40, 40);
    CGSize blueSize = CGSizeMake(4, 4);
    
    MaplyCoordinate coord = MaplyCoordinateMakeWithDegrees(-94.58, 39.1);
    
    NSMutableArray *redMarkers = [NSMutableArray array];
    NSMutableArray *blueMarkers = [NSMutableArray array];
    NSMutableArray *labels = [NSMutableArray array];
    {
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.image = redSquare;
        marker.loc = coord;
        marker.size = redSize;
        marker.layoutImportance = 2.0;
        marker.offset = CGPointMake(0.0,-34.0);
        marker.selectable = true;
        marker.userObject = @"Red Screen Marker";
        [redMarkers addObject:marker];
    }
    
    {
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.image = blueSquare;
        marker.loc = coord;
        marker.size = blueSize;
        marker.layoutImportance = 3.0;
        marker.offset = CGPointMake(0.0,0.0);
        marker.selectable = true;
        marker.userObject = @"Blue Screen Marker";
        [blueMarkers addObject:marker];
    }
    
    {
        MaplyScreenLabel *label1 = [[MaplyScreenLabel alloc] init];
        label1.loc = coord;
        label1.text = @"Test Label 1";
        label1.layoutPlacement = kMaplyLayoutCenter;
        label1.layoutImportance = MAXFLOAT;
//        label1.offset = CGPointMake(0, 20.0);
        label1.selectable = true;
        label1.userObject = @"Test Label 1";
        label1.rotation = M_PI/2;
        [labels addObject:label1];

        MaplyScreenLabel *label2 = [[MaplyScreenLabel alloc] init];
        label2.loc = coord;
        label2.text = @"Test Label 2";
        label2.layoutPlacement = kMaplyLayoutRight;
        label2.layoutImportance = 2.0;
//        label2.offset = CGPointMake(0, 20.0);
        label2.selectable = true;
        label2.userObject = @"Test Label 2";
        //        label1.rotation = M_PI/2;
        [labels addObject:label2];
    }
    
    NSMutableArray *newObjs = [NSMutableArray array];
//    [newObjs addObject:[baseViewC addScreenMarkers:redMarkers desc:@{kMaplyDrawPriority: @(100)} mode:MaplyThreadCurrent]];
//    [newObjs addObject:[baseViewC addScreenMarkers:blueMarkers desc:@{kMaplyDrawPriority: @(101)} mode:MaplyThreadCurrent]];
    [newObjs addObject:[baseViewC addScreenLabels:labels desc:
                        @{kMaplyDrawPriority: @(102),
                          kMaplyFont: [UIFont systemFontOfSize:30.0],
                          kMaplyBackgroundColor: [UIColor blueColor]
                          } mode:MaplyThreadCurrent]];
    
//    [self performSelector:@selector(labelMarkerTest:) withObject:time afterDelay:[time floatValue]];
}

- (void)findHeightTest
{
    if (globeViewC)
    {
        MaplyBoundingBox bbox;
        bbox.ll = MaplyCoordinateMakeWithDegrees(7.05090689853, 47.7675500593);
        bbox.ur = MaplyCoordinateMakeWithDegrees(8.06813647023, 49.0562323851);
        MaplyCoordinate center = MaplyCoordinateMakeWithDegrees((7.05090689853+8.06813647023)/2, (47.7675500593+49.0562323851)/2);
        double height = [globeViewC findHeightToViewBounds:bbox pos:center];
        mapViewC.height = height;
        [globeViewC animateToPosition:center time:1.0];
        NSLog(@"height = %f",height);
    }    
}

// Test animation to a point with height and heading
- (void)viewAnimationTest
{
    [globeViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222) height:0.1 heading:45.0/180.0*M_PI time:10.0];
}

// Create a bunch of labels periodically
- (void)labelExercise
{
//     NSTimer *_labelAnimationTimer;
//     NSMutableDictionary *_trafficLabels;

    if (mapViewC) {
         _trafficLabels = [NSMutableDictionary dictionary];

         _labelAnimationTimer = [NSTimer scheduledTimerWithTimeInterval:1.25 target:self selector:@selector(labelAnimationCallback) userInfo:nil repeats:NO];
     }
}

- (void) labelAnimationCallback
{
    NSLog(@"anim callback");
    MaplyComponentObject *trafficLabelCompObj;

    NSArray *keys = [_trafficLabels allKeys];
    for (NSObject *key in keys) {
        trafficLabelCompObj = _trafficLabels[key];
        if (!trafficLabelCompObj)
            continue;
        [mapViewC removeObject:trafficLabelCompObj];
        [_trafficLabels removeObjectForKey:key];
    }

    NSDictionary *labelsDesc = @{kMaplyMinVis: @(0.0), kMaplyMaxVis: @(1.0), kMaplyFade: @(0.3), kMaplyJustify : @"left", kMaplyDrawPriority: @(50)};
    MaplyScreenLabel *label;
    for (int i=0; i<50; i++) {
         label = [[MaplyScreenLabel alloc] init];

         label.loc = MaplyCoordinateMakeWithDegrees(
             -100.0 + 0.25 * ((float)arc4random()/0x100000000),
             40.0 + 0.25 * ((float)arc4random()/0x100000000));
         label.rotation = 0.0;
         label.layoutImportance = 1.0;
         label.text = @"ABCDE";
         label.layoutPlacement = kMaplyLayoutRight;
         label.userObject = nil;
         label.color = [UIColor whiteColor];
 
        
         trafficLabelCompObj = [mapViewC addScreenLabels:[NSArray arrayWithObjects:label, nil] desc:labelsDesc];
         _trafficLabels[@(i)] = trafficLabelCompObj;
        
     }
    
    
     _labelAnimationTimer = [NSTimer scheduledTimerWithTimeInterval:1.25 target:self selector:@selector(labelAnimationCallback) userInfo:nil repeats:NO];
}

// Try to fetch the given WMS layer
- (void)fetchWMSLayer:(NSString *)baseURL layer:(NSString *)layerName style:(NSString *)styleName cacheDir:(NSString *)thisCacheDir ovlName:(NSString *)ovlName
{
    NSString *capabilitiesURL = [MaplyWMSCapabilities CapabilitiesURLFor:baseURL];

    NSURLSession *session = [NSURLSession sharedSession];
    NSURLSessionDataTask *task = [session dataTaskWithURL:[NSURL URLWithString:capabilitiesURL] completionHandler:
    ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        if (!error) {
            NSError *xmlError;
            DDXMLDocument *doc = [[DDXMLDocument alloc] initWithData:data options:0 error:&xmlError];
            [self startWMSLayerBaseURL:baseURL xml:doc layer:layerName style:styleName cacheDir:thisCacheDir ovlName:ovlName];
        } else {
            // Sometimes this works anyway
            //if (![self startWMSLayerBaseURL:baseURL xml:XMLDocument layer:layerName style:styleName cacheDir:thisCacheDir ovlName:ovlName])
            //    NSLog(@"Failed to get capabilities from WMS server: %@",capabilitiesURL);
        }
    }];
    [task resume];

}

// Try to start the layer, given the capabilities
- (bool)startWMSLayerBaseURL:(NSString *)baseURL xml:(DDXMLDocument *)XMLDocument layer:(NSString *)layerName style:(NSString *)styleName cacheDir:(NSString *)thisCacheDir ovlName:(NSString *)ovlName
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
        MaplyQuadImageTilesLayer *imageLayer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:coordSys tileSource:tileSource];
        imageLayer.coverPoles = false;
        imageLayer.handleEdges = false;
        imageLayer.requireElev = requireElev;
        imageLayer.waitLoad = imageWaitLoad;
        imageLayer.drawPriority = BaseEarthPriority + 1000;
        if (startupMapType == Maply2DMap)
        {
            imageLayer.singleLevelLoading = true;
            imageLayer.multiLevelLoads = @[@(-2)];
        }
        [baseViewC addLayer:imageLayer];
        
        if (ovlName)
            ovlLayers[ovlName] = imageLayer;
    }
    
    return true;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    // This should release the globe view
    if (baseViewC)
    {
        [baseViewC.view removeFromSuperview];
        [baseViewC removeFromParentViewController];
        baseViewC = nil;
        mapViewC = nil;
        globeViewC = nil;
    }

}

- (void)viewWillAppear:(BOOL)animated
{
    // This tests heading
//    [self performSelector:@selector(changeHeading:) withObject:@(0.0) afterDelay:1.0];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(changeHeading:) object:nil];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

#pragma mark - Data Display

// Change the heading every so often
- (void)changeHeading:(NSNumber *)heading
{
    if (globeViewC)
        [globeViewC setHeading:[heading floatValue]];
    else if (mapViewC)
        [mapViewC setHeading:[heading floatValue]];
    
    [self performSelector:@selector(changeHeading:) withObject:@([heading floatValue]+1.0/180.0*M_PI) afterDelay:1.0];
}

// Add screen (2D) markers at all our locations
- (void)addScreenMarkers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    UIImage *pinImage = [UIImage imageNamed:@"map_pin"];
    NSMutableArray *markers = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        
        {
            MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
            marker.image = pinImage;
            marker.loc = MaplyCoordinateMakeWithDegrees(location->lon,location->lat);
            marker.size = CGSizeMake(32,32);
            marker.userObject = [NSString stringWithFormat:@"%s",location->name];
            marker.layoutImportance = 2.0;
            [markers addObject:marker];
        }
    }
    
    screenMarkersObj = [baseViewC addScreenMarkers:markers desc:@{kMaplyDrawPriority: @(100)}];
}

// Add 3D markers
- (void)addMarkers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    CGSize size = CGSizeMake(0.05, 0.05);    
    UIImage *startImage = [UIImage imageNamed:@"Star"];
    
    NSMutableArray *markers = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyMarker *marker = [[MaplyMarker alloc] init];
        marker.image = startImage;
        marker.loc = MaplyCoordinateMakeWithDegrees(location->lon,location->lat);
        marker.size = size;
        marker.userObject = [NSString stringWithFormat:@"%s",location->name];
        [markers addObject:marker];
    }
    
    markersObj = [baseViewC addMarkers:markers desc:nil];
}

// Add screen (2D) labels
- (void)addScreenLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    NSMutableArray *labels = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
        label.loc = MaplyCoordinateMakeWithDegrees(location->lon,location->lat);
        label.text = [NSString stringWithFormat:@"%s",location->name];
        label.layoutImportance = 2.0;
        label.userObject = [NSString stringWithFormat:@"%s",location->name];
        [labels addObject:label];
    }
    
    screenLabelsObj = [baseViewC addScreenLabels:labels desc:screenLabelDesc];
}

// Add 3D labels
- (void)addLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    CGSize size = CGSizeMake(0, 0.05);
    
    NSMutableArray *labels = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyLabel *label = [[MaplyLabel alloc] init];
        label.loc = MaplyCoordinateMakeWithDegrees(location->lon,location->lat);
        label.size = size;
        label.text = [NSString stringWithFormat:@"%s",location->name];
        label.userObject = [NSString stringWithFormat:@"%s",location->name];
        [labels addObject:label];
    }
    
    labelsObj = [baseViewC addLabels:labels desc:labelDesc];
}

// Add cylinders
- (void)addShapeCylinders:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc
{
    NSMutableArray *cyls = [[NSMutableArray alloc] init];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyShapeCylinder *cyl = [[MaplyShapeCylinder alloc] init];
        cyl.baseCenter = MaplyCoordinateMakeWithDegrees(location->lon, location->lat);
        cyl.radius = 0.01;
        cyl.height = 0.06;
        cyl.selectable = true;
        [cyls addObject:cyl];
    }
    
    shapeCylObj = [baseViewC addShapes:cyls desc:desc];
}

// Add spheres
- (void)addShapeSpheres:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc
{
    NSMutableArray *spheres = [[NSMutableArray alloc] init];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
        sphere.center = MaplyCoordinateMakeWithDegrees(location->lon, location->lat);
        sphere.radius = 0.04;
        sphere.selectable = true;
        [spheres addObject:sphere];
    }

    shapeSphereObj = [baseViewC addShapes:spheres desc:desc];
}

// Add great circles
- (void)addGreatCircles:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc
{
    NSMutableArray *circles = [[NSMutableArray alloc] init];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *loc0 = &locations[ii];
        LocationInfo *loc1 = &locations[(ii+1)%len];
        MaplyShapeGreatCircle *greatCircle = [[MaplyShapeGreatCircle alloc] init];
        greatCircle.startPt = MaplyCoordinateMakeWithDegrees(loc0->lon, loc0->lat);
        greatCircle.endPt = MaplyCoordinateMakeWithDegrees(loc1->lon, loc1->lat);
        greatCircle.lineWidth = 6.0;
        greatCircle.selectable = true;
        // This limits the height based on the length of the great circle
        float angle = [greatCircle calcAngleBetween];
        greatCircle.height = 0.3 * angle / M_PI;
        [circles addObject:greatCircle];
    }
    
    greatCircleObj = [baseViewC addShapes:circles desc:desc ];
}

// Add arrows
- (void)addArrows:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc
{
    // Start out the arrow at 1m
    double size = 1;
    double arrowCoords[2*7] = {-0.25*size,-0.75*size, -0.25*size,0.25*size, -0.5*size,0.25*size, 0.0*size,1.0*size,  0.5*size,0.25*size, 0.25*size,0.25*size, 0.25*size,-0.75*size};
    
    MaplyShapeExtruded *exShape = [[MaplyShapeExtruded alloc] initWithOutline:arrowCoords numCoordPairs:7];
    exShape.thickness = size * 1.0;
    exShape.height = 0.0;
    exShape.color = [UIColor colorWithRed:0.8 green:0.25 blue:0.25 alpha:1.0];
    // Each shape is about 10km
//    exShape.transform = [[MaplyMatrix alloc] initWithScale:10000*1/EarthRadius];
    exShape.scale = 1.0;
    MaplyGeomModel *shapeModel = [[MaplyGeomModel alloc] initWithShape:exShape];

    NSMutableArray *arrows = [[NSMutableArray alloc] init];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *loc = &locations[ii];
        MaplyGeomModelInstance *geomInst = [[MaplyGeomModelInstance alloc] init];
        MaplyCoordinate coord = MaplyCoordinateMakeWithDegrees(loc->lon, loc->lat);
        geomInst.center = MaplyCoordinate3dMake(coord.x, coord.y, 10000);
        MaplyMatrix *orientMat = [[MaplyMatrix alloc] initWithYaw:0.0 pitch:0.0 roll:45.0/180.0*M_PI];
        geomInst.transform = [[[MaplyMatrix alloc] initWithScale:10000*1/EarthRadius] multiplyWith:orientMat];
        geomInst.selectable = true;
        geomInst.model = shapeModel;
        
        [arrows addObject:geomInst];
    }
    
    arrowsObj = [baseViewC addModelInstances:arrows desc:desc mode:MaplyThreadAny];
}

// Add models
- (void)addModels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc
{
    // Load the model
    NSString *fullPath = [[NSBundle mainBundle] pathForResource:@"cessna" ofType:@"obj"];
    if (!fullPath)
        return;
    MaplyGeomModel *model = [[MaplyGeomModel alloc] initWithObj:fullPath];
    if (!model)
        return;

    NSMutableArray *modelInstances = [NSMutableArray array];
    // We need to scale the models down to display space.  They start out in meters.
    // Note: Changes this to 1000.0/6371000.0 if you can't find the models
    MaplyMatrix *scaleMat = [[MaplyMatrix alloc] initWithScale:1000.0/6371000.0];
    // Then we need to rotate around the X axis to get the model pointed up
    MaplyMatrix *rotMat = [[MaplyMatrix alloc] initWithAngle:M_PI/2.0 axisX:1.0 axisY:0.0 axisZ:0.0];
    // Combine the scale and rotation
    MaplyMatrix *localMat = [rotMat multiplyWith:scaleMat];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *loc = &locations[ii];
        MaplyMovingGeomModelInstance *mInst = [[MaplyMovingGeomModelInstance alloc] init];
        mInst.model = model;
        mInst.transform = localMat;
        MaplyCoordinate loc2d = MaplyCoordinateMakeWithDegrees(loc->lon, loc->lat);
        // Put it 1km above the earth
        mInst.center = MaplyCoordinate3dMake(loc2d.x, loc2d.y, 10000);
        mInst.endCenter = MaplyCoordinate3dMake(loc2d.x+0.1, loc2d.y+0.1, 10000);
        mInst.duration = 100.0;
        mInst.selectable = true;
        [modelInstances addObject:mInst];
    }
    
    modelsObj = [baseViewC addModelInstances:modelInstances desc:desc mode:MaplyThreadCurrent];
}

- (void)addLinesLon:(float)lonDelta lat:(float)latDelta color:(UIColor *)color
{
    NSMutableArray *vectors = [[NSMutableArray alloc] init];
    NSDictionary *desc = @{kMaplyColor: color,
                           kMaplySelectable: @YES,
                           kMaplySubdivType: kMaplySubdivSimple, kMaplySubdivEpsilon: @(0.001), kMaplyVecWidth: @(4.0)};
    // Longitude lines
    for (float lon = -180;lon < 180;lon += lonDelta)
    {
        MaplyCoordinate coords[3];
        coords[0] = MaplyCoordinateMakeWithDegrees(lon, -90);
        coords[1] = MaplyCoordinateMakeWithDegrees(lon, 0);
        coords[2] = MaplyCoordinateMakeWithDegrees(lon, +90);
        MaplyVectorObject *vec = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:3 attributes:nil];
        [vectors addObject:vec];
    }
    // Latitude lines
    for (float lat = -90;lat < 90;lat += latDelta)
    {
        MaplyCoordinate coords[5];
        coords[0] = MaplyCoordinateMakeWithDegrees(-180, lat);
        coords[1] = MaplyCoordinateMakeWithDegrees(-90, lat);
        coords[2] = MaplyCoordinateMakeWithDegrees(0, lat);
        coords[3] = MaplyCoordinateMakeWithDegrees(90, lat);
        coords[4] = MaplyCoordinateMakeWithDegrees(+180, lat);
        MaplyVectorObject *vec = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:5 attributes:nil];
        [vectors addObject:vec];
    }
    
    latLonObj = [baseViewC addVectors:vectors desc:desc];
}

- (NSArray *)addWideVectors:(MaplyVectorObject *)vecObj
{
    NSMutableArray *compObjs = [NSMutableArray array];
    
    UIColor *color = [UIColor blueColor];
    float fade = 0.25;
//    MaplyComponentObject *lines = [baseViewC addVectors:@[vecObj] desc:@{kMaplyColor: color,
//                                                                         kMaplyVecWidth: @(1.0),
//                                                                         kMaplyFade: @(fade),
//                                                                         kMaplyVecCentered: @(true),
//                                                                         kMaplyMaxVis: @(10.0),
//                                                                         kMaplyMinVis: @(0.00032424763776361942)
//                                                                         }];
    
    
    MaplyComponentObject *screenLines = [baseViewC addWideVectors:@[vecObj] desc:@{kMaplyColor: [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:1.0],
                                                                                   kMaplyFade: @(0),
                                                                                   kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 1),
                                                                                   kMaplyVecWidth: @(8.0),
                                                                                   kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
//                                                                                   kMaplyVecTexture: filledLineTex,
                                                                                   kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
//                                                                                   kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
//                                                                                   kMaplyWideVecMiterLimit: @(1.01),
//                                                                                   kMaplyWideVecTexRepeatLen: @(8),
//                                                                                   kMaplyMaxVis: @(0.00032424763776361942),
//                                                                                   kMaplyMinVis: @(0.00011049506429117173)
                                                                                   }];
    [compObjs addObject:screenLines];
    
    MaplyComponentObject *screenLines2 = [baseViewC addWideVectors:@[vecObj] desc:@{kMaplyColor: [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0],
                                                                                   kMaplyFade: @(0),
                                                                                   kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 2),
                                                                                   kMaplyVecWidth: @(6.0),
                                                                                   kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
                                                                                   //                                                                                   kMaplyVecTexture: filledLineTex,
                                                                                   kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                                                                                   //                                                                                   kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
                                                                                   //                                                                                   kMaplyWideVecMiterLimit: @(1.01),
                                                                                   //                                                                                   kMaplyWideVecTexRepeatLen: @(8),
                                                                                   //                                                                                   kMaplyMaxVis: @(0.00032424763776361942),
                                                                                   //                                                                                   kMaplyMinVis: @(0.00011049506429117173)
                                                                                   }];
    [compObjs addObject:screenLines2];

    
    // Note: Real world width doesn't quite work
//    MaplyComponentObject *realLines = [baseViewC addWideVectors:@[vecObj] desc:@{kMaplyColor: color,
//                                                                                 kMaplyFade: @(fade),
//                                                                                 kMaplyVecTexture: dashedLineTex,
//                                                                                 // 8m in display coordinates
//                                                                                 kMaplyVecWidth: @(10.0/6371000),
//                                                                                 kMaplyWideVecCoordType: kMaplyWideVecCoordTypeReal,
//                                                                                 kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
//                                                                                 kMaplyWideVecMiterLimit: @(1.01),
//                                                                                 // Repeat every 10m
//                                                                                 kMaplyWideVecTexRepeatLen: @(10/6371000.f),
//                                                                                 kMaplyMaxVis: @(0.00011049506429117173),
//                                                                                 kMaplyMinVis: @(0.0)
//                                                                                 }];
    
    // Look for some labels
    MaplyComponentObject *labelObj = nil;
    NSMutableArray *labels = [NSMutableArray array];
    for (MaplyVectorObject *road in [vecObj splitVectors])
    {
        MaplyCoordinate middle;
        double rot;
        // Note: We should get this from the view controller
        MaplyCoordinateSystem *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
        [road linearMiddle:&middle rot:&rot displayCoordSys:coordSys];
        NSDictionary *attrs = road.attributes;
        
        NSString *name = attrs[@"FULLNAME"];
        
        if (name)
        {
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            label.loc = middle;
            label.text = name;
            label.layoutImportance = 1.0;
            label.rotation = rot + M_PI/2.0;
            label.keepUpright = true;
            label.layoutImportance = kMaplyLayoutBelow;
            [labels addObject:label];
        }
    }
    labelObj = [baseViewC addScreenLabels:labels desc:
                @{kMaplyTextOutlineSize: @(1.0),
                  kMaplyTextOutlineColor: [UIColor blackColor],
                  kMaplyFont: [UIFont systemFontOfSize:18.0],
                  kMaplyDrawPriority: @(200)
                  }];
    [compObjs addObject:labelObj];
    
    return compObjs;
}

- (void)addShapeFile:(NSString *)shapeFileName
{
    // Make the dashed line if it isn't already there
    if (!dashedLineTex)
    {
        MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
        [lineTexBuilder setPattern:@[@(4),@(4)]];
        UIImage *dashedLineImage = [lineTexBuilder makeImage];
        dashedLineTex = [baseViewC addTexture:dashedLineImage
                                         desc:@{kMaplyTexMinFilter: kMaplyMinFilterLinear,
                                                kMaplyTexMagFilter: kMaplyMinFilterLinear,
                                                kMaplyTexWrapX: @true,
                                                kMaplyTexWrapY: @true,
                                                kMaplyTexFormat: @(MaplyImageIntRGBA)}
                                         mode:MaplyThreadCurrent];
    }
    if (!filledLineTex)
    {
        MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
        [lineTexBuilder setPattern:@[@(32)]];
        UIImage *lineImage = [lineTexBuilder makeImage];
        filledLineTex = [baseViewC addTexture:lineImage
                                                   desc:@{kMaplyTexMinFilter: kMaplyMinFilterLinear,
                                                          kMaplyTexMagFilter: kMaplyMinFilterLinear,
                                                          kMaplyTexWrapX: @true,
                                                          kMaplyTexWrapY: @true,
                                                          kMaplyTexFormat: @(MaplyImageIntRGBA)}
                                                   mode:MaplyThreadCurrent];
    }

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
    ^{
        // Add the vectors at three different levels

        MaplyVectorDatabase *vecDb = [[MaplyVectorDatabase alloc] initWithShape:shapeFileName];
        if (vecDb)
        {
            MaplyVectorObject *vecObj = [vecDb fetchAllVectors];
            if (vecObj)
            {
                sfRoadsObjArray = [self addWideVectors:vecObj];
            }
        }
    });
}

- (void)addGeoJson:(NSString*)name {
    [self addGeoJson:name dashPattern:@[@8, @8] width:4];
}

- (void)addGeoJson:(NSString*)name dashPattern:(NSArray*)dashPattern width:(CGFloat)width {
    MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
    [lineTexBuilder setPattern:dashPattern];
    UIImage *lineImage = [lineTexBuilder makeImage];
    MaplyTexture *lineTexture = [baseViewC addTexture:lineImage
                                          imageFormat:MaplyImageIntRGBA
                                            wrapFlags:MaplyImageWrapY
                                                 mode:MaplyThreadCurrent];
    
    NSString *path = [[NSBundle mainBundle] pathForResource:name ofType:nil];
    if(path) {
        NSData *data = [NSData dataWithContentsOfFile:path];
        NSDictionary *jsonDictionary = [NSJSONSerialization JSONObjectWithData:data
                                                                       options:0 error:nil];
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithGeoJSONDictionary:jsonDictionary];
        if(vecObj) {
            [baseViewC addWideVectors:@[vecObj]
                                 desc: @{kMaplyColor: [UIColor colorWithRed:1 green:0 blue:0 alpha:1.0],
                                         kMaplyFilled: @NO,
                                         kMaplyEnable: @YES,
                                         kMaplyFade: @0,
                                         kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 1),
                                         kMaplyVecCentered: @YES,
                                         kMaplyVecTexture: lineTexture,
                                         kMaplyWideVecEdgeFalloff: @(1.0),
                                         kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
                                         kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                                         // More than 10 degrees need a bevel join
                                         kMaplyWideVecMiterLimit: @(10),
                                         kMaplyVecWidth: @(width)}
                                 mode:MaplyThreadCurrent];
            [baseViewC addVectors:@[vecObj]
                             desc: @{kMaplyColor: [UIColor blackColor],
                                     kMaplyFilled: @NO,
                                     kMaplyEnable: @YES,
                                     kMaplyFade: @0,
                                     kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault),
                                     kMaplyVecCentered: @YES,
                                     kMaplyVecWidth: @(1)}
                             mode:MaplyThreadCurrent];
            sfRoadsObjArray = @[vecObj];
        }
    }
}

- (void)addArcGISQuery:(NSString *)url
{
    
    NSURLSession *session = [NSURLSession sharedSession];
    NSURLSessionDataTask *task = [session dataTaskWithURL:[NSURL URLWithString:url] completionHandler:
    ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        if (!error) {
            MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithGeoJSON:data];
            if (vecObj)
                arcGisObj = [baseViewC addVectors:@[vecObj] desc:@{kMaplyColor: [UIColor redColor]}];

        } else
            NSLog(@"Unable to fetch ArcGIS layer:\n%@",error);
    }];
    [task resume];
}

- (void)addStickers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset desc:(NSDictionary *)desc
{
    UIImage *startImage = [UIImage imageNamed:@"Smiley_Face_Avatar_by_PixelTwist"];
    
    NSMutableArray *stickers = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplySticker *sticker = [[MaplySticker alloc] init];
        // Stickers are sized in geographic (because they're for KML ground overlays).  Bleah.
        sticker.ll = MaplyCoordinateMakeWithDegrees(location->lon, location->lat);
        sticker.ur = MaplyCoordinateMakeWithDegrees(location->lon+10.0, location->lat+10.0);
        sticker.image = startImage;
        // And a random rotation
//        sticker.rotation = 2*M_PI * drand48();
        [stickers addObject:sticker];
    }
    
    stickersObj = [baseViewC addStickers:stickers desc:desc];
}

static const bool CountryTextures = false;
static const bool SubdivisionTest = false;

// Add country outlines.  Pass in the names of the geoJSON files
- (void)addCountries:(NSArray *)names stride:(int)stride
{
    MaplyTexture *smileTex = nil;
    UIImage *smileImage = nil;
    if (CountryTextures)
    {
        smileImage = [UIImage imageNamed:@"Smiley_Face_Avatar_by_PixelTwist"];
        smileTex = [baseViewC addTexture:smileImage imageFormat:MaplyImageUShort5551 wrapFlags:MaplyImageWrapX|MaplyImageWrapY mode:MaplyThreadCurrent];
    }

    // Parsing the JSON can take a while, so let's hand that over to another queue
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), 
         ^{
             NSMutableArray *locVecObjects = [NSMutableArray array];
             NSMutableArray *locAutoLabels = [NSMutableArray array];
             
             int ii = 0;
             for (NSString *name in names)
             {
                 if (ii % stride == 0)
                 {
                     NSString *fileName = [[NSBundle mainBundle] pathForResource:name ofType:@"geojson"];
                     if (fileName)
                     {
                         
                         NSData *jsonData = [NSData dataWithContentsOfFile:fileName];
                         if (jsonData)
                         {
                             MaplyVectorObject *wgVecObj = [[MaplyVectorObject alloc] initWithGeoJSON:jsonData];
                             NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
                             wgVecObj.userObject = vecName;
                             NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:@{
                                                                                                         kMaplySelectable: @YES
                                                                                                         }];
                             if (CountryTextures)
                             {
                                 desc[kMaplyVecTexture] = smileTex;
                                 desc[kMaplyVecTextureProjection] = kMaplyProjectionScreen;
                                 desc[kMaplyVecTexScaleX] = @(1.0/smileImage.size.width);
                                 desc[kMaplyVecTexScaleY] = @(1.0/smileImage.size.height);
                                 desc[kMaplyFilled] = @(YES);
                             }
                             
                             // Note: Subdivision test
                             if (SubdivisionTest)
                             {
                                 desc[kMaplyFilled] = @(YES);
                                 desc[kMaplySubdivType] = kMaplySubdivGrid;
                                 desc[kMaplySubdivEpsilon] = @(0.05);
                                 desc[kMaplyColor] = [UIColor redColor];
                             }
                             
                             MaplyComponentObject *compObj = [baseViewC addVectors:[NSArray arrayWithObject:wgVecObj] desc:desc];
                             MaplyScreenLabel *screenLabel = [[MaplyScreenLabel alloc] init];
                             // Add a label right in the middle
                             MaplyCoordinate center;
                             if ([wgVecObj centroid:&center])
                             {
                                 screenLabel.loc = center;
                                 screenLabel.layoutImportance = 1.0;
                                 screenLabel.text = vecName;
                                 screenLabel.userObject = screenLabel.text;
                                 screenLabel.layoutPlacement = kMaplyLayoutRight | kMaplyLayoutAbove | kMaplyLayoutLeft | kMaplyLayoutBelow;
                                 screenLabel.selectable = true;
                                 if (screenLabel.text)
                                     [locAutoLabels addObject:screenLabel];
                             }
                             if (compObj)
                                 [locVecObjects addObject:compObj];
                         }
                     }
                 }
                 ii++;
             }
             
             // Keep track of the created objects
             // Note: You could lose track of the objects if you turn the countries on/off quickly
             dispatch_async(dispatch_get_main_queue(),
                            ^{
                                // Toss in all the labels at once, more efficient
                                MaplyComponentObject *autoLabelObj = [baseViewC addScreenLabels:locAutoLabels desc:
                                                                      @{kMaplyTextColor: [UIColor colorWithRed:0.85 green:0.85 blue:0.85 alpha:1.0],
                                                                            kMaplyFont: [UIFont systemFontOfSize:24.0],
                                                                         kMaplyTextOutlineColor: [UIColor blackColor],
                                                                          kMaplyTextOutlineSize: @(1.0),
//                                                                               kMaplyShadowSize: @(1.0)
                                                                      } mode:MaplyThreadAny];

                                vecObjects = locVecObjects;
                                autoLabels = autoLabelObj;
                            });

         }
    );
}

- (void)addStars:(NSString *)inFile
{
    if (!globeViewC)
        return;
    
    // Load the stars
    NSString *fileName = [[NSBundle mainBundle] pathForResource:inFile ofType:@"txt"];
    if (fileName)
    {
        stars = [[MaplyStarsModel alloc] initWithFileName:fileName];
        stars.image = [UIImage imageNamed:@"star_background"];
        [stars addToViewC:globeViewC date:[NSDate date] desc:nil mode:MaplyThreadCurrent];
    }
}

static const bool UseSunSphere = false;
static const bool UseMoonSphere = false;
static const float EarthRadius = 6371000;

- (void)addSun
{
    if (!globeViewC)
        return;
    
    [globeViewC setClearColor:[UIColor blackColor]];
    
    // Lighting for the sun
    MaplySun *sun = [[MaplySun alloc] initWithDate:[NSDate date]];
    MaplyLight *sunLight = [sun makeLight];
    [baseViewC clearLights];
    [baseViewC addLight:sunLight];
    
    // And a model, because why not
    if (UseSunSphere)
    {
        MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
        sphere.center = [sun asPosition];
        sphere.radius = 0.2;
        sphere.height = 4.0;
        sunObj = [globeViewC addShapes:@[sphere] desc:
                    @{kMaplyColor: [UIColor yellowColor],
                      kMaplyShader: kMaplyShaderDefaultTriNoLighting}];
    } else {
        MaplyBillboard *bill = [[MaplyBillboard alloc] init];
        MaplyCoordinate centerGeo = [sun asPosition];
        bill.center = MaplyCoordinate3dMake(centerGeo.x, centerGeo.y, 5.4*EarthRadius);
        bill.selectable = false;
        bill.screenObj = [[MaplyScreenObject alloc] init];
        UIImage *globeImage = [UIImage imageNamed:@"SunImage"];
        [bill.screenObj addImage:globeImage color:[UIColor whiteColor] size:CGSizeMake(0.9, 0.9)];
        sunObj = [globeViewC addBillboards:@[bill] desc:@{kMaplyBillboardOrient: kMaplyBillboardOrientEye,kMaplyDrawPriority: @(kMaplySunDrawPriorityDefault)} mode:MaplyThreadAny];
    }
    
    // Position for the moon
    MaplyMoon *moon = [[MaplyMoon alloc] initWithDate:[NSDate date]];
    if (UseMoonSphere)
    {
        MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
        sphere.center = [moon asCoordinate];
        sphere.radius = 0.2;
        sphere.height = 4.0;
        moonObj = [globeViewC addShapes:@[sphere] desc:
                   @{kMaplyColor: [UIColor grayColor],
                     kMaplyShader: kMaplyShaderDefaultTriNoLighting}];
    } else {
        MaplyBillboard *bill = [[MaplyBillboard alloc] init];
        MaplyCoordinate3d centerGeo = [moon asPosition];
        bill.center = MaplyCoordinate3dMake(centerGeo.x, centerGeo.y, 5.4*EarthRadius);
        bill.selectable = false;
        bill.screenObj = [[MaplyScreenObject alloc] init];
        UIImage *moonImage = [UIImage imageNamed:@"moon"];
        [bill.screenObj addImage:moonImage color:[UIColor colorWithWhite:moon.illuminatedFraction alpha:1.0] size:CGSizeMake(0.75, 0.75)];
        moonObj = [globeViewC addBillboards:@[bill] desc:@{kMaplyBillboardOrient: kMaplyBillboardOrientEye, kMaplyDrawPriority: @(kMaplyMoonDrawPriorityDefault)} mode:MaplyThreadAny];
    }
    
    // And some atmosphere, because the iDevice fill rate is just too fast
    atmosObj = [[MaplyAtmosphere alloc] initWithViewC:globeViewC];
    // Very red
//    float wavelength[3] = {0.350f,0.970f,0.975f};
    // Blueish atmosphere
//    float wavelength[3] = {0.650f,0.570f,0.475f};
    float wavelength[3] = {0.650f,0.570f,0.475f};
//    atmosObj.outerRadius = 1.1;
//    atmosObj.Kr = atmosObj.Kr * 2;
//    atmosObj.Km = atmosObj.Km * 2;
    [atmosObj setWavelength:wavelength];
    [atmosObj setSunPosition:[sun getDirection]];
}

// Number of unique images to use for the mega markers
static const int NumMegaMarkerImages = 1000;
// Number of markers to whip up for the large test case
static const int NumMegaMarkers = 15000;

// Generate a random image for testing
- (UIImage *)randomImage
{
    float scale = [UIScreen mainScreen].scale;
    
    CGSize size = CGSizeMake(16*scale, 16*scale);
    UIGraphicsBeginImageContext(size);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    CGRect rect = CGRectMake(1, 1, size.width-2, size.height-2);
    CGContextAddEllipseInRect(ctx, rect);
    [[UIColor whiteColor] setStroke];
    CGContextStrokePath(ctx);
    [[UIColor colorWithRed:drand48() green:drand48() blue:drand48() alpha:1.0] setFill];
    CGContextFillEllipseInRect(ctx, rect);
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

// Make up a large number of markers and add them
- (void)addMegaMarkers
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
       ^{
           // Make up a few markers
           NSMutableArray *markerImages = [NSMutableArray array];
           for (unsigned int ii=0;ii<NumMegaMarkerImages;ii++)
           {
               UIImage *image = [self randomImage];
               MaplyTexture *tex = [baseViewC addTextureToAtlas:image mode:MaplyThreadCurrent];
               [markerImages addObject:tex];
           }
           
           NSMutableArray *markers = [NSMutableArray array];
           for (unsigned int ii=0;ii<NumMegaMarkers;ii++)
           {
               MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
               marker.image = [markerImages objectAtIndex:random()%NumMegaMarkerImages];
               marker.size = CGSizeMake(16,16);
               marker.loc = MaplyCoordinateMakeWithDegrees(drand48()*360-180, drand48()*140-70);
               marker.layoutImportance = MAXFLOAT;
//               marker.layoutImportance = 1.0;
               [markers addObject:marker];
           }

           megaMarkersObj = [baseViewC addScreenMarkers:markers desc:@{kMaplyClusterGroup: @(0)} mode:MaplyThreadCurrent];
           megaMarkersImages = markerImages;
       }
    );
}

// Number of degrees around a given point to spread out random markers
static const float MarkerSpread = 2.0;

// Make up a large number of markers and add them
- (void)addMarkerCluster:(LocationInfo *)locations num:(int)howMany markersPer:(int)markersPer
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
    ^{
        NSMutableArray *markers = [NSMutableArray array];
        UIImage *pinImage = [UIImage imageNamed:@"map_pin"];

        // Work through the locations
        for (unsigned int ii=0;ii<howMany;ii++)
        {
            LocationInfo *location = &locations[ii];
            
            int howMany = markersPer/2.0 + markersPer * drand48() / 2.0;
            
            // Make up a few markers per location
            for (unsigned int jj=0;jj<howMany;jj++)
            {
                MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
                marker.image = pinImage;
                marker.size = CGSizeMake(32,32);
                marker.loc = MaplyCoordinateMakeWithDegrees(location->lon + drand48()*MarkerSpread, location->lat + drand48()*MarkerSpread);
                marker.layoutImportance = 1.0;
                marker.userObject = [NSString stringWithFormat:@"%s %d",location->name,jj];
                [markers addObject:marker];
            }
        }
       
       markerClusterObj = [baseViewC addScreenMarkers:markers desc:@{kMaplyClusterGroup: @(0)} mode:MaplyThreadCurrent];
    }
    );
}

- (void)addMarkerPagingTest
{
    markerDelegate = [[PagingTestDelegate alloc] init];
    markerLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:markerDelegate.coordSys delegate:markerDelegate];
    [baseViewC addLayer:markerLayer];
    
    [self markerSpamRefresh];
}

- (void)markerSpamRefresh
{
    [markerLayer reload];

    if (markerLayer)
        [self performSelector:@selector(markerSpamRefresh) withObject:nil afterDelay:4.0];
}

// Create an animated sphere
- (void)addAnimatedSphere
{
    animSphere = [[AnimatedSphere alloc] initWithPeriod:20.0 radius:0.01 color:[UIColor orangeColor] viewC:baseViewC];
    [baseViewC addActiveObject:animSphere];
}

// Test sequence for zoom
- (void)zoomTest
{
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)((2.0) * NSEC_PER_SEC)), dispatch_get_main_queue(),
                   ^{
                       [mapViewC setHeight:mapViewC.height/2.0];
                   });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)((2.0+1.0) * NSEC_PER_SEC)), dispatch_get_main_queue(),
                   ^{
                       [mapViewC setHeight:mapViewC.height/2.0];
                   });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)((2.0+2.0) * NSEC_PER_SEC)), dispatch_get_main_queue(),
                   ^{
                       [mapViewC setHeight:mapViewC.height/2.0];
                   });
    //    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)((2.0+3.0) * NSEC_PER_SEC)), dispatch_get_main_queue(),
    //                   ^{
    //                       [mapViewC setHeight:mapViewC.height*2.0];
    //                   });
}

// Set this to reload the base layer ever so often.  Purely for testing
//#define RELOADTEST 1

// Set up the base layer depending on what they've picked.
// Also tear down an old one
- (void)setupBaseLayer:(NSDictionary *)baseSettings
{
    // No fancy base layers for globe elevation
    if (startupMapType == MaplyGlobeWithElevation)
        return;
    
    // Figure out which one we're supposed to display
    NSString *newBaseLayerName = nil;
    for (NSString *key in [baseSettings allKeys])
    {
        if ([baseSettings[key] boolValue])
        {
            newBaseLayerName = key;
            break;
        }
    }
    
    // Didn't change
    if (![newBaseLayerName compare:baseLayerName])
        return;
    
    // Tear down the old layer
    if (baseLayer)
    {
        baseLayerName = nil;
        [baseViewC removeLayer:baseLayer];
        baseLayer = nil;
    }
    baseLayerName = newBaseLayerName;
    
    // For network paging layers, where we'll store temp files
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
    
    // We'll pick default colors for the labels
    UIColor *screenLabelColor = [UIColor whiteColor];
    UIColor *screenLabelBackColor = [UIColor clearColor];
    UIColor *labelColor = [UIColor whiteColor];
    UIColor *labelBackColor = [UIColor clearColor];
    // And for the vectors to stand out
    UIColor *vecColor = [UIColor whiteColor];
    float vecWidth = 4.0;
    
    NSString *jsonTileSpec = nil;
    NSString *thisCacheDir = nil;
    
#ifdef RELOADTEST
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(reloadLayer:) object:nil];
#endif
    
    if (![baseLayerName compare:kMaplyTestBlank])
    {
        // Nothing to see here
    }
    if (![baseLayerName compare:kMaplyTestGeographyClass])
    {
        self.title = @"Geography Class - MBTiles Local";
        // This is the Geography Class MBTiles data set from MapBox
        MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc] initWithMBTiles:@"geography-class_medres"];
        MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
        baseLayer = layer;
        layer.handleEdges = (globeViewC != nil);
        layer.coverPoles = (globeViewC != nil);
        layer.requireElev = requireElev;
        layer.waitLoad = imageWaitLoad;
        layer.drawPriority = BaseEarthPriority;
        layer.singleLevelLoading = (startupMapType == Maply2DMap);
//        layer.northPoleColor = [UIColor whiteColor];
//        layer.southPoleColor = [UIColor greenColor];
        [layer setTesselationValues:tessValues];
        [baseViewC addLayer:layer];
        
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor colorWithRed:0.4 green:0.4 blue:0.4 alpha:1.0];
        
#ifdef RELOADTEST
        [self performSelector:@selector(reloadLayer:) withObject:nil afterDelay:10.0];
#endif

    } else if (![baseLayerName compare:kMaplyTestBlueMarble])
    {
        self.title = @"Blue Marble Single Res";
        if (globeViewC)
        {
            // This is the static image set, included with the app, built with ImageChopper
            WGViewControllerLayer *layer = [globeViewC addSphericalEarthLayerWithImageSet:@"lowres_wtb_info"];
            baseLayer = (MaplyViewControllerLayer *)layer;
            baseLayer.drawPriority = BaseEarthPriority;
            screenLabelColor = [UIColor whiteColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
            vecColor = [UIColor whiteColor];
            vecWidth = 4.0;
        }        
    } else if (![baseLayerName compare:kMaplyTestStamenWatercolor])
    {
        self.title = @"Stamen Water Color - Remote";
        // These are the Stamen Watercolor tiles.
        thisCacheDir = [NSString stringWithFormat:@"%@/stamentiles/",cacheDir];
        int maxZoom = 16;
        if (zoomLimit != 0 && zoomLimit < maxZoom)
            maxZoom = zoomLimit;
        MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:@"http://tile.stamen.com/watercolor/" ext:@"png" minZoom:0 maxZoom:maxZoom];
        tileSource.cacheDir = thisCacheDir;
        MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
        layer.handleEdges = true;
        layer.requireElev = requireElev;
        [layer setTesselationValues:tessValues];
        [baseViewC addLayer:layer];
        layer.drawPriority = BaseEarthPriority;
        layer.waitLoad = imageWaitLoad;
        if (startupMapType == Maply2DMap)
        {
            layer.singleLevelLoading = true;
            layer.multiLevelLoads = @[@(-4), @(-2)];
        }
        baseLayer = layer;
        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor blackColor];
        vecColor = [UIColor grayColor];
        vecWidth = 4.0;
    } else if (![baseLayerName compare:kMaplyTestOSM])
    {
        self.title = @"OpenStreetMap - Remote";
        // This points to the OpenStreetMap tile set hosted by MapQuest (I think)
        thisCacheDir = [NSString stringWithFormat:@"%@/osmtiles/",cacheDir];
        int maxZoom = 18;
        if (zoomLimit != 0 && zoomLimit < maxZoom)
            maxZoom = zoomLimit;
        MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/" ext:@"png" minZoom:0 maxZoom:maxZoom];
        tileSource.cacheDir = thisCacheDir;
        MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
        layer.drawPriority = BaseEarthPriority;
        layer.handleEdges = true;
        layer.requireElev = requireElev;
        layer.waitLoad = imageWaitLoad;
        layer.maxTiles = maxLayerTiles;
        if (startupMapType == Maply2DMap)
        {
            layer.singleLevelLoading = true;
            layer.multiLevelLoads = @[@(-4), @(-2)];
        }
        [layer setTesselationValues:tessValues];
        [baseViewC addLayer:layer];
        layer.drawPriority = BaseEarthPriority;
        baseLayer = layer;
        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor blackColor];
        vecWidth = 4.0;
    } else if (![baseLayerName compare:kMaplyTestMapBoxSat])
    {
        self.title = @"MapBox Tiles Satellite - Remote";
        jsonTileSpec = @"http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2.json";
        thisCacheDir = [NSString stringWithFormat:@"%@/mbtilessat1/",cacheDir];
        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor whiteColor];
        vecWidth = 4.0;
    } else if (![baseLayerName compare:kMaplyTestMapBoxTerrain])
    {
        self.title = @"MapBox Tiles Terrain - Remote";
        jsonTileSpec = @"http://a.tiles.mapbox.com/v3/examples.map-zq0f1vuc.json";
        thisCacheDir = [NSString stringWithFormat:@"%@/mbtilesterrain1/",cacheDir];
        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor blackColor];
        vecWidth = 4.0;
    } else if (![baseLayerName compare:kMaplyTestMapBoxRegular])
    {
        self.title = @"MapBox Tiles Regular - Remote";
        jsonTileSpec = @"http://a.tiles.mapbox.com/v3/examples.map-zswgei2n.json";
        thisCacheDir = [NSString stringWithFormat:@"%@/mbtilesregular1/",cacheDir];
        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor blackColor];
        vecWidth = 4.0;
    } else if (![baseLayerName compare:kMaplyTestNightAndDay])
    {
        self.title = @"Day/Night Basemap";
        int minZoom = 1;
        int maxZoom = 8;
        MaplyRemoteTileInfo *tileSource1 = [[MaplyRemoteTileInfo alloc] initWithBaseURL:@"http://map1.vis.earthdata.nasa.gov/wmts-webmerc/MODIS_Terra_CorrectedReflectance_TrueColor/default/2015-05-07/GoogleMapsCompatible_Level9/{z}/{y}/{x}" ext:@"jpg" minZoom:minZoom maxZoom:maxZoom];
        tileSource1.cacheDir = [NSString stringWithFormat:@"%@/daytexture-2015-05-07/",cacheDir];
        MaplyRemoteTileInfo *tileSource2 = [[MaplyRemoteTileInfo alloc] initWithBaseURL:@"http://map1.vis.earthdata.nasa.gov/wmts-webmerc/VIIRS_CityLights_2012/default/2015-05-07/GoogleMapsCompatible_Level8/{z}/{y}/{x}" ext:@"jpg" minZoom:minZoom maxZoom:maxZoom];
        tileSource1.cacheDir = [NSString stringWithFormat:@"%@/nighttexture-2015-05-07/",cacheDir];
        
        MaplyMultiplexTileSource *tileSource = [[MaplyMultiplexTileSource alloc] initWithSources:@[tileSource1,tileSource2]];
        
        MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource1.coordSys tileSource:tileSource];
        layer.drawPriority = BaseEarthPriority;
        layer.handleEdges = true;
        layer.requireElev = requireElev;
        layer.waitLoad = imageWaitLoad;
        layer.maxTiles = maxLayerTiles;
        layer.imageDepth = 2;
        layer.allowFrameLoading = false;
        layer.currentImage = 0.5;
        layer.singleLevelLoading = (startupMapType == Maply2DMap);
        layer.shaderProgramName = kMaplyShaderDefaultTriNightDay;
        [layer setTesselationValues:tessValues];
        if (atmosObj)
            layer.shaderProgramName = atmosObj.groundShader.name;
        [baseViewC addLayer:layer];
        layer.drawPriority = BaseEarthPriority;
        baseLayer = layer;

        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor blackColor];
        vecWidth = 4.0;
    } else if (![baseLayerName compare:kMaplyTestQuadTest])
    {
        self.title = @"Quad Paging Test Layer";
        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor blackColor];
        vecWidth = 4.0;
        MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:[[MaplySphericalMercator alloc] initWebStandard] minZoom:0 maxZoom:22 depth:1];
//        MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:[[MaplySphericalMercator alloc] initWebStandard] minZoom:0 maxZoom:22 depth:1];
        tileSource.pixelsPerSide = 256;
        tileSource.transparentMode = true;
        MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
        layer.waitLoad = imageWaitLoad;
        layer.requireElev = requireElev;
        layer.maxTiles = 512;
        layer.handleEdges = true;
        layer.flipY = true;
        
//        layer.color = [UIColor colorWithWhite:0.5 alpha:0.5];
        if (startupMapType == Maply2DMap)
        {
            layer.useTargetZoomLevel = true;
            layer.singleLevelLoading = true;
            layer.multiLevelLoads = @[@(-2)];
        }
        [layer setTesselationValues:tessValues];
        [baseViewC addLayer:layer];
        layer.drawPriority = BaseEarthPriority;
        baseLayer = layer;
        
//        [self zoomTest];
    } else if (![baseLayerName compare:kMaplyTestQuadVectorTest])
    {
        self.title = @"Quad Paging Test Layer";
        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor blackColor];
        vecWidth = 4.0;
        MaplyPagingVectorTestTileSource *tileSource = [[MaplyPagingVectorTestTileSource alloc] initWithCoordSys:[[MaplySphericalMercator alloc] initWebStandard] minZoom:0 maxZoom:10];
        MaplyQuadPagingLayer *layer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:tileSource.coordSys delegate:tileSource];
        layer.importance = 128*128;
        layer.singleLevelLoading = (startupMapType == Maply2DMap);
        [baseViewC addLayer:layer];
        layer.drawPriority = BaseEarthPriority;
        baseLayer = layer;
    } else if (![baseLayerName compare:kMaplyTestElevation])
    {
        self.title = @"Cesium Elevation Test Layer";

        int maxZoom = 16;
        if (zoomLimit != 0 && zoomLimit < maxZoom)
            maxZoom = zoomLimit;

        elevSource = [[MaplyRemoteTileElevationCesiumSource alloc] initWithBaseURL:@"http://cesiumjs.org/stk-terrain/tilesets/world/tiles/" ext:@"terrain" minZoom:0 maxZoom:maxZoom];
        baseViewC.elevDelegate = elevSource;

        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor blackColor];
        vecWidth = 4.0;
        MaplyPagingElevationTestTileSource *tileSource = [[MaplyPagingElevationTestTileSource alloc] initWithCoordSys:[[MaplySphericalMercator alloc] initWebStandard] minZoom:0 maxZoom:10 elevSource:elevSource];
        MaplyQuadPagingLayer *layer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:tileSource.coordSys delegate:tileSource];
        layer.importance = 128*128;
        layer.singleLevelLoading = (startupMapType == Maply2DMap);
        [baseViewC addLayer:layer];
        layer.drawPriority = 0;
        baseLayer = layer;
    } else if (![baseLayerName compare:kMaplyTestQuadTestAnimate])
    {
        self.title = @"Quad Paging Test Layer (animated)";
        screenLabelColor = [UIColor whiteColor];
        screenLabelBackColor = [UIColor whiteColor];
        labelColor = [UIColor blackColor];
        labelBackColor = [UIColor whiteColor];
        vecColor = [UIColor blackColor];
        vecWidth = 4.0;
        MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:[[MaplySphericalMercator alloc] initWebStandard] minZoom:0 maxZoom:17 depth:4];
        tileSource.transparentMode = true;
        tileSource.pixelsPerSide = 128;
        MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
        layer.waitLoad = imageWaitLoad;
        layer.requireElev = requireElev;
        layer.imageDepth = 4;
        layer.handleEdges = (startupMapType != Maply2DMap);
        layer.maxTiles = 512;
        // We'll cycle through at 1/2s per layer
        layer.animationPeriod = 2.0;
        layer.allowFrameLoading = false;
        layer.useTargetZoomLevel = true;
        layer.singleLevelLoading = true;
        layer.multiLevelLoads = @[@(-3)];
        [layer setTesselationValues:tessValues];
        [baseViewC addLayer:layer];
        layer.drawPriority = BaseEarthPriority;
        baseLayer = layer;        
    }
    
    // If we're fetching one of the JSON tile specs, kick that off
    if (jsonTileSpec)
    {
        NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:jsonTileSpec]];

        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionDataTask *task = [session dataTaskWithRequest:request completionHandler:
        ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
            NSError *jsonError;
            NSDictionary *jsonDict;
            if (!error) {
                jsonDict = [NSJSONSerialization JSONObjectWithData:data options:kNilOptions error:&jsonError];
            }
            if (!error && !jsonError) {
                // Add a quad earth paging layer based on the tile spec we just fetched
                MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithTilespec:jsonDict];
                tileSource.cacheDir = thisCacheDir;
                if (zoomLimit != 0 && zoomLimit < tileSource.maxZoom)
                    tileSource.tileInfo.maxZoom = zoomLimit;

                MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
                layer.handleEdges = true;
                layer.waitLoad = imageWaitLoad;
                layer.requireElev = requireElev;
                layer.maxTiles = maxLayerTiles;
                if (startupMapType == Maply2DMap)
                {
                    layer.singleLevelLoading = true;
                    layer.multiLevelLoads = @[@(-4), @(-2)];
                }
                [layer setTesselationValues:tessValues];
                [baseViewC addLayer:layer];
                layer.drawPriority = BaseEarthPriority;
                baseLayer = layer;

#ifdef RELOADTEST
                [self performSelector:@selector(reloadLayer:) withObject:nil afterDelay:10.0];
#endif

            } else
                NSLog(@"Failed to reach JSON tile spec at: %@",jsonTileSpec);
        }];
        [task resume];

    }
    
    // Set up some defaults for display
    screenLabelDesc = @{kMaplyTextColor: screenLabelColor,
                        //                        kMaplyBackgroundColor: screenLabelBackColor,
                        kMaplyFade: @(1.0),
                        kMaplyTextOutlineSize: @(1.5),
                        kMaplyTextOutlineColor: [UIColor blackColor],
                        };
    labelDesc = @{kMaplyTextColor: labelColor,
                  kMaplyBackgroundColor: labelBackColor,
                  kMaplyFade: @(1.0)};
    vectorDesc = @{kMaplyColor: vecColor,
                   kMaplyVecWidth: @(vecWidth),
                   kMaplyFade: @(1.0),
                   kMaplySelectable: @(true)};
    
}

// Reload testing
- (void)reloadLayer:(MaplyQuadImageTilesLayer *)layer
{
    if (baseLayer && [baseLayer isKindOfClass:[MaplyQuadImageTilesLayer class]])
    {
        MaplyQuadImageTilesLayer *layer = (MaplyQuadImageTilesLayer *)baseLayer;
        NSLog(@"Reloading layer");
        [layer reload];

        [self performSelector:@selector(reloadLayer:) withObject:nil afterDelay:10.0];
    }
}

// Fade testing
- (void)quadImageFadeTest:(MaplyQuadImageTilesLayer *)layer
{
    if (layer)
    {
        layer.fade = drand48();
        
        [self performSelector:@selector(quadImageFadeTest:) withObject:layer afterDelay:2.0];
    }
}

// Run through the overlays the user wants turned on
- (void)setupOverlays:(NSDictionary *)baseSettings
{
    // For network paging layers, where we'll store temp files
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
    NSString *thisCacheDir = nil;

    for (NSString *layerName in [baseSettings allKeys])
    {
        bool isOn = [baseSettings[layerName] boolValue];
        MaplyViewControllerLayer *layer = ovlLayers[layerName];
        // Need to create the layer
        if (isOn && !layer)
        {
            if (![layerName compare:kMaplyTestUSGSOrtho])
            {
                thisCacheDir = [NSString stringWithFormat:@"%@/usgs_naip/",cacheDir];
                [self fetchWMSLayer:@"http://raster.nationalmap.gov/arcgis/services/Orthoimagery/USGS_EROS_Ortho_NAIP/ImageServer/WMSServer" layer:@"0" style:nil cacheDir:thisCacheDir ovlName:layerName];
            } else if (![layerName compare:kMaplyTestOWM])
            {
                NSString *weatherDataType = @"surface_pressure";

                // Spherical mercator tile sets
//                NSString *coordSysStr = @"mercator";
//                MaplyCoordinateSystem *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
//                NSString *baseURL = @"http://weather.openportguide.de/demo";

                // Plate Carree tile sets
                NSString *coordSysStr = @"geo";
                MaplyBoundingBox geobbox;
                geobbox.ll = MaplyCoordinateMakeWithDegrees(-180, -180);
                geobbox.ur = MaplyCoordinateMakeWithDegrees(180, 90);
                MaplyCoordinateSystem *coordSys = [[MaplyPlateCarree alloc] initWithBoundingBox:geobbox];
                NSString *baseURL = @"http://weather.openportguide.com/tiles/actual/";

                NSString *urlStr = [NSString stringWithFormat:@"%@/%@/5/{z}/{x}/{y}",baseURL,weatherDataType];
                MaplyRemoteTileSource *tileSource = [[MaplyRemoteTileSource alloc] initWithBaseURL:urlStr ext:@"png" minZoom:1 maxZoom:7];
                tileSource.coordSys = coordSys;
                tileSource.cacheDir = [NSString stringWithFormat:@"%@/%@_%@/",cacheDir,weatherDataType,coordSysStr];
                tileSource.tileInfo.cachedFileLifetime = 3 * 60 * 60; // invalidate OWM data after three hours
                MaplyQuadImageTilesLayer *weatherLayer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
                weatherLayer.coverPoles = false;
                weatherLayer.drawPriority = BaseEarthPriority+200;
                layer = weatherLayer;
                weatherLayer.handleEdges = false;
                weatherLayer.flipY = true;
                [baseViewC addLayer:weatherLayer];
                ovlLayers[layerName] = layer;
            } else if (![layerName compare:kMaplyTestForecastIO])
            {
                // Collect up the various precipitation sources
                NSMutableArray *tileSources = [NSMutableArray array];
                for (unsigned int ii=0;ii<5;ii++)
                {
                    MaplyRemoteTileInfo *precipTileSource =
                    [[MaplyRemoteTileInfo alloc]
                     initWithBaseURL:[NSString stringWithFormat:@"http://a.tiles.mapbox.com/v3/mousebird.precip-example-layer%d/",ii] ext:@"png" minZoom:0 maxZoom:6];
                    precipTileSource.cacheDir = [NSString stringWithFormat:@"%@/forecast_io_weather_layer%d/",cacheDir,ii];
                    [tileSources addObject:precipTileSource];
                }
                MaplyMultiplexTileSource *precipTileSource = [[MaplyMultiplexTileSource alloc] initWithSources:tileSources];
                // Create a precipitation layer that animates
                MaplyQuadImageTilesLayer *precipLayer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:precipTileSource.coordSys tileSource:precipTileSource];
                precipLayer.imageDepth = (int)[tileSources count];
                precipLayer.animationPeriod = 6.0;
                precipLayer.imageFormat = MaplyImageUByteRed;
                //                precipLayer.texturAtlasSize = 512;
                precipLayer.numSimultaneousFetches = 4;
                precipLayer.handleEdges = false;
                precipLayer.coverPoles = false;
                precipLayer.shaderProgramName = [WeatherShader setupWeatherShader:baseViewC];
                precipLayer.fade = 0.5;
                //                [self quadImageFadeTest:precipLayer];
                [baseViewC addLayer:precipLayer];
                layer = precipLayer;
                ovlLayers[layerName] = layer;
            } else if (![layerName compare:kMaplyTestMapboxStreets])
            {
#ifdef NOTPODSPECWG
                self.title = @"Mapbox Vector Streets";
                thisCacheDir = [NSString stringWithFormat:@"%@/mapbox-streets-vectiles",cacheDir];
                [MaplyMapnikVectorTiles StartRemoteVectorTilesWithTileSpec:@"https://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v6.json"
                    // Note: You need your own access token here
                    accessToken:@"pk.eyJ1IjoicGV0ZXJxbGl1IiwiYSI6ImpvZmV0UEEifQ._D4bRmVcGfJvo1wjuOpA1g"
                    style:@"https://raw.githubusercontent.com/mapbox/mapbox-gl-styles/master/styles/emerald-v8.json"
                    styleType:MapnikMapboxGLStyle
                    cacheDir:thisCacheDir
                    viewC:baseViewC
                 success:
                 ^(MaplyMapnikVectorTiles *vecTiles)
                 {
                     // Don't load the lowest levels for the globe
                     if (globeViewC)
                         vecTiles.minZoom = 5;
                     
                     // Note: These are set after the MapnikStyleSet has already been initialized
                     MaplyMapboxVectorStyleSet *styleSet = (MaplyMapboxVectorStyleSet *)vecTiles.tileParser.styleDelegate;
                     styleSet.tileStyleSettings.markerImportance = 10.0;
                     styleSet.tileStyleSettings.fontName = @"Gill Sans";
                     UIColor *backColor = [styleSet backgroundColor];
                     if (backColor)
                         [baseViewC setClearColor:backColor];
                     
                     // Now for the paging layer itself
                     MaplyQuadPagingLayer *pageLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:[[MaplySphericalMercator alloc] initWebStandard] delegate:vecTiles];
                     pageLayer.numSimultaneousFetches = 6;
                     pageLayer.flipY = false;
                     pageLayer.importance = 1024*1024*2;
                     pageLayer.useTargetZoomLevel = true;
                     pageLayer.singleLevelLoading = true;
                     [baseViewC addLayer:pageLayer];
                     ovlLayers[layerName] = pageLayer;
                 }
                                                              failure:
                 ^(NSError *error){
                     NSLog(@"Failed to load Mapnik vector tiles because: %@",error);
                 }
                 ];
#endif
                
//                [MaplyMapnikVectorTiles StartRemoteVectorTilesWithTileSpec:@"http://a.tiles.mapbox.com/v3/mapbox.mapbox-streets-v4.json"
//                  style:[[NSBundle mainBundle] pathForResource:@"osm-bright" ofType:@"xml"]
//                  cacheDir:thisCacheDir
//                     viewC:baseViewC
//                   success:
//                         ^(MaplyMapnikVectorTiles *vecTiles)
//                        {
//                            // Don't load the lowest levels for the globe
//                            if (globeViewC)
//                                vecTiles.minZoom = 5;
//                            
//                            // Note: These are set after the MapnikStyleSet has already been initialized
//                            MapnikStyleSet *styleSet = (MapnikStyleSet *)vecTiles.styleDelegate;
//                            styleSet.tileStyleSettings.markerImportance = 10.0;
//                            styleSet.tileStyleSettings.fontName = @"Gill Sans";
//                            
//                            // Now for the paging layer itself
//                            MaplyQuadPagingLayer *pageLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:[[MaplySphericalMercator alloc] initWebStandard] delegate:vecTiles];
//                            pageLayer.numSimultaneousFetches = 6;
//                            pageLayer.flipY = false;
//                            pageLayer.importance = 1024*1024*2;
//                            pageLayer.useTargetZoomLevel = true;
//                            pageLayer.singleLevelLoading = true;
//                            [baseViewC addLayer:pageLayer];
//                            ovlLayers[layerName] = pageLayer;
//                         }
//                   failure:
//                         ^(NSError *error){
//                             NSLog(@"Failed to load Mapnik vector tiles because: %@",error);
//                         }
//                 ];
            } else if (![layerName compare:kMaplyMapzenVectors])
            {
#ifdef NOTPODSPECWG
                // Get the style file
                NSData *styleData = [NSData dataWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"MapzenGLStyle" ofType:@"json"]];
                
                if (styleData)
                {
                    thisCacheDir = [NSString stringWithFormat:@"%@/mapzen-vectiles",cacheDir];
                    MapzenSource *mzSource = [[MapzenSource alloc]
                                              initWithBase:@"http://vector.mapzen.com/osm"
                                              layers:@[@"all"]
                                              // Note: Go get your own API key
                                              apiKey:@"vector-tiles-05s8a-0"
                                              sourceType:MapzenSourcePBF
                                              styleData:styleData
                                              styleType:MapnikMapboxGLStyle
                                              viewC:baseViewC];
                    mzSource.minZoom = 0;
                    mzSource.maxZoom = 24;
                   
                    // Now for the paging layer itself
                    MaplyQuadPagingLayer *pageLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:[[MaplySphericalMercator alloc] initWebStandard] delegate:mzSource];
                    pageLayer.numSimultaneousFetches = 8;
                    pageLayer.flipY = false;
                    pageLayer.importance = 512*512;
                    pageLayer.useTargetZoomLevel = true;
                    pageLayer.singleLevelLoading = true;
                    [baseViewC addLayer:pageLayer];
                    ovlLayers[layerName] = pageLayer;
                    
                    self.title = @"Mapzen Vector Tiles";
                } else
                    NSLog(@"Failed to load style sheet for Mapzen.");
#endif
            } else if (![layerName compare:kMaplyWindTest])
            {
                ParticleTileDelegate *partDelegate = [[ParticleTileDelegate alloc] initWithURL:@"http://tilesets.s3-website-us-east-1.amazonaws.com/wind_test/{dir}_tiles/{z}/{x}/{y}.png" minZoom:2 maxZoom:5 viewC:baseViewC];
                MaplyQuadPagingLayer *layer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:partDelegate.coordSys delegate:partDelegate];
                layer.flipY = false;

                [baseViewC addLayer:layer];
                ovlLayers[layerName] = layer;
            } else if (![layerName compare:kMaplyOrdnanceSurveyTest])
            {
                MaplyCoordinateSystem *bngCoordSys = [self buildBritishNationalGrid:false];
                MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:bngCoordSys minZoom:0 maxZoom:22 depth:1];
                //        MaplyAnimationTestTileSource *tileSource = [[MaplyAnimationTestTileSource alloc] initWithCoordSys:[[MaplySphericalMercator alloc] initWebStandard] minZoom:0 maxZoom:22 depth:1];
                tileSource.pixelsPerSide = 128;
                tileSource.transparentMode = true;
                MaplyQuadImageTilesLayer *layer = [[MaplyQuadImageTilesLayer alloc] initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
                layer.maxTiles = 256;
                layer.handleEdges = false;
                layer.flipY = true;
                layer.coverPoles = false;
                
                //        layer.color = [UIColor colorWithWhite:0.5 alpha:0.5];
                if (startupMapType == Maply2DMap)
                {
                    layer.useTargetZoomLevel = true;
                    layer.singleLevelLoading = true;
                    layer.multiLevelLoads = @[@(-2)];
                }
                [baseViewC addLayer:layer];
                layer.drawPriority = BaseEarthPriority+100;
                ovlLayers[layerName] = layer;
            }
        }
        else if (!isOn && layer)
        {
            // Get rid of the layer
            [baseViewC removeLayer:layer];
            [ovlLayers removeObjectForKey:layerName];
        }
    }

    // Fill out the cache dir if there is one
    if (thisCacheDir)
    {
        NSError *error = nil;
        [[NSFileManager defaultManager] createDirectoryAtPath:thisCacheDir withIntermediateDirectories:YES attributes:nil error:&error];
    }
}

// Look at the configuration controller and decide what to turn off or on
- (void)changeMapContents
{
    imageWaitLoad = [configViewC valueForSection:kMaplyTestCategoryInternal row:kMaplyTestWaitLoad];
    
    [self setupBaseLayer:((ConfigSection *)configViewC.values[0]).rows];
    if ([configViewC.values count] > 1)
        [self setupOverlays:((ConfigSection *)configViewC.values[1]).rows];
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestLabel2D])
    {
        if (!screenLabelsObj)
            [self addScreenLabels:locations len:NumLocations stride:4 offset:0];
    } else {
        if (screenLabelsObj)
        {
            [baseViewC removeObject:screenLabelsObj];
            screenLabelsObj = nil;
        }
    }    

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestLabel3D])
    {
        if (!labelsObj)
            [self addLabels:locations len:NumLocations stride:4 offset:1];
    } else {
        if (labelsObj)
        {
            [baseViewC removeObject:labelsObj];
            labelsObj = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestMarker2D])
    {
        if (!screenMarkersObj)
            [self addScreenMarkers:locations len:NumLocations stride:1 offset:0];
    } else {
        if (screenMarkersObj)
        {
            [baseViewC removeObject:screenMarkersObj];
            screenMarkersObj = nil;
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestMarker3D])
    {
        if (!markersObj)
            [self addMarkers:locations len:NumLocations stride:4 offset:3];
    } else {
        if (markersObj)
        {
            [baseViewC removeObject:markersObj];
            markersObj = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestSticker])
    {
        if (!stickersObj)
            [self addStickers:locations len:NumLocations stride:4 offset:2 desc:@{kMaplyFade: @(1.0)}];
    } else {
        if (stickersObj)
        {
            [baseViewC removeObject:stickersObj];
            stickersObj = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestShapeCylinder])
    {
        if (!shapeCylObj)
        {
            [self addShapeCylinders:locations len:NumLocations stride:4 offset:0 desc:@{kMaplyColor : [UIColor colorWithRed:0.0 green:0.0 blue:1.0 alpha:0.8], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)}];
        }
    } else {
        if (shapeCylObj)
        {
            [baseViewC removeObject:shapeCylObj];
            shapeCylObj = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestShapeSphere])
    {
        if (!shapeSphereObj)
        {
            [self addShapeSpheres:locations len:NumLocations stride:4 offset:1 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.8], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)}];
        }
    } else {
        if (shapeSphereObj)
        {
            [baseViewC removeObject:shapeSphereObj];
            shapeSphereObj = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestShapeGreatCircle])
    {
        if (!greatCircleObj)
        {
            [self addGreatCircles:locations len:NumLocations stride:4 offset:2 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.1 blue:0.0 alpha:1.0], kMaplyFade: @(1.0), kMaplyDrawPriority: @(1000)}];
        }
    } else {
        if (greatCircleObj)
        {
            [baseViewC removeObject:greatCircleObj];
            greatCircleObj = nil;
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestShapeArrows])
    {
        if (!arrowsObj)
        {
            [self addArrows:locations len:NumLocations stride:4 offset:2 desc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.1 blue:0.0 alpha:1.0], kMaplyFade: @(1.0)}];
        }
    } else {
        if (arrowsObj)
        {
            [baseViewC removeObject:arrowsObj];
            arrowsObj = nil;
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestModels])
    {
        if (!modelsObj)
        {
            [self addModels:locations len:NumLocations stride:4 offset:2 desc:@{}];
        }
    } else {
        if (modelsObj)
        {
            [baseViewC removeObject:modelsObj];
            modelsObj = nil;
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestLatLon])
    {
        if (!latLonObj)
        {
            [self addLinesLon:20 lat:10 color:[UIColor blueColor]];
        }
    } else {
        if (latLonObj)
        {
            [baseViewC removeObject:latLonObj];
            latLonObj = nil;
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestRoads])
    {
        if (!sfRoadsObjArray)
        {
            [self addShapeFile:@"tl_2013_06075_roads"];
        }
    } else {
        if (sfRoadsObjArray)
        {
            [baseViewC removeObjects:sfRoadsObjArray];
            sfRoadsObjArray = nil;
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestArcGIS])
    {
        if (!arcGisObj)
        {
            [self addArcGISQuery:@"http://services.arcgis.com/OfH668nDRN7tbJh0/arcgis/rest/services/SandyNYCEvacMap/FeatureServer/0/query?WHERE=Neighbrhd=%27Rockaways%27&f=pgeojson&outSR=4326"];
        }
    } else {
        if (arcGisObj)
        {
            [baseViewC removeObject:arcGisObj];
            arcGisObj = nil;
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestStarsAndSun])
    {
        if (!stars)
        {
            [self addStars:@"starcatalog_orig"];
            [self addSun];
        }
    } else {
        if (stars)
        {
            [stars removeFromViewC];
            [baseViewC removeObject:sunObj];
            [baseViewC removeObject:moonObj];
            [atmosObj removeFromViewC];
            sunObj = nil;
            moonObj = nil;
            stars = nil;
            atmosObj = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestCountry])
    {
        // Countries we have geoJSON for
        NSArray *countryArray = [NSArray arrayWithObjects:
         @"ABW", @"AFG", @"AGO", @"AIA", @"ALA", @"ALB", @"AND", @"ARE", @"ARG", @"ARM", @"ASM", @"ATA", @"ATF", @"ATG", @"AUS", @"AUT",
         @"AZE", @"BDI", @"BEL", @"BEN", @"BES", @"BFA", @"BGD", @"BGR", @"BHR", @"BHS", @"BIH", @"BLM", @"BLR", @"BLZ", @"BMU", @"BOL",
         @"BRA", @"BRB", @"BRN", @"BTN", @"BVT", @"BWA", @"CAF", @"CAN", @"CCK", @"CHE", @"CHL", @"CHN", @"CIV", @"CMR", @"COD", @"COG",
         @"COK", @"COL", @"COM", @"CPV", @"CRI", @"CUB", @"CUW", @"CXR", @"CYM", @"CYP", @"CZE", @"DEU", @"DJI", @"DMA", @"DNK", @"DOM",
         @"DZA", @"ECU", @"EGY", @"ERI", @"ESH", @"ESP", @"EST", @"ETH", @"FIN", @"FJI", @"FLK", @"FRA", @"FRO", @"FSM", @"GAB", @"GBR",
         @"GEO", @"GGY", @"GHA", @"GIB", @"GIN", @"GLP", @"GMB", @"GNB", @"GNQ", @"GRC", @"GRD", @"GRL", @"GTM", @"GUF", @"GUM", @"GUY",
         @"HKG", @"HMD", @"HND", @"HRV", @"HTI", @"HUN", @"IDN", @"IMN", @"IND", @"IOT", @"IRL", @"IRN", @"IRQ", @"ISL", @"ISR", @"ITA",
         @"JAM", @"JEY", @"JOR", @"JPN", @"KAZ", @"KEN", @"KGZ", @"KHM", @"KIR", @"KNA", @"KOR", @"KWT", @"LAO", @"LBN", @"LBR", @"LBY",
         @"LCA", @"LIE", @"LKA", @"LSO", @"LTU", @"LUX", @"LVA", @"MAC", @"MAF", @"MAR", @"MCO", @"MDA", @"MDG", @"MDV", @"MEX", @"MHL",
         @"MKD", @"MLI", @"MLT", @"MMR", @"MNE", @"MNG", @"MNP", @"MOZ", @"MRT", @"MSR", @"MTQ", @"MUS", @"MWI", @"MYS", @"MYT", @"NAM",
         @"NCL", @"NER", @"NFK", @"NGA", @"NIC", @"NIU", @"NLD", @"NOR", @"NPL", @"NRU", @"NZL", @"OMN", @"PAK", @"PAN", @"PCN", @"PER",
         @"PHL", @"PLW", @"PNG", @"POL", @"PRI", @"PRK", @"PRT", @"PRY", @"PSE", @"PYF", @"QAT", @"REU", @"ROU", @"RUS", @"RWA", @"SAU",
         @"SDN", @"SEN", @"SGP", @"SGS", @"SHN", @"SJM", @"SLB", @"SLE", @"SLV", @"SMR", @"SOM", @"SPM", @"SRB", @"SSD", @"STP", @"SUR",
         @"SVK", @"SVN", @"SWE", @"SWZ", @"SXM", @"SYC", @"SYR", @"TCA", @"TCD", @"TGO", @"THA", @"TJK", @"TKL", @"TKM", @"TLS", @"TON",
         @"TTO", @"TUN", @"TUR", @"TUV", @"TWN", @"TZA", @"UGA", @"UKR", @"UMI", @"URY", @"USA", @"UZB", @"VAT", @"VCT", @"VEN", @"VGB",
         @"VIR", @"VNM", @"VUT", @"WLF", @"WSM", @"YEM", @"ZAF", @"ZMB", @"ZWE", nil];
        
        if (!vecObjects)
            [self addCountries:countryArray stride:1];
    } else {
        if (vecObjects)
        {
            [baseViewC removeObjects:vecObjects];
            vecObjects = nil;
        }
        if (autoLabels)
        {
            [baseViewC removeObject:autoLabels];
            autoLabels = nil;
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestLoftedPoly])
    {
    } else {
        if ([loftPolyDict count] > 0)
        {
            [baseViewC removeObjects:loftPolyDict.allValues];
            loftPolyDict = [NSMutableDictionary dictionary];
        }
    }
    
    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestMegaMarkers])
    {
        if (!megaMarkersObj)
            [self addMegaMarkers];
    } else {
        if (megaMarkersObj)
        {
            [baseViewC removeObject:megaMarkersObj];
            [baseViewC removeTextures:megaMarkersImages mode:MaplyThreadAny];
            megaMarkersObj = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestMarkerCluster])
    {
        if (!markerClusterObj)
            [self addMarkerCluster:locations num:NumLocations markersPer:120];
    } else {
        if (markerClusterObj)
        {
            [baseViewC removeObject:markerClusterObj];
            markerClusterObj = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestQuadMarkers])
    {
        if (!markerLayer)
            [self addMarkerPagingTest];
    } else {
        if (markerLayer)
        {
            [baseViewC removeLayer:markerLayer];
            markerLayer = nil;
            markerDelegate = nil;
        }
    }

    if ([configViewC valueForSection:kMaplyTestCategoryAnimation row:kMaplyTestAnimateSphere])
    {
        if (!animSphere)
            [self addAnimatedSphere];
    } else {
        if (animSphere)
        {
            [baseViewC removeActiveObject:animSphere];
            animSphere = nil;
        }
    }
    
    baseViewC.performanceOutput = [configViewC valueForSection:kMaplyTestCategoryInternal row:kMaplyTestPerf];
    
    if (globeViewC)
    {
        globeViewC.keepNorthUp = [configViewC valueForSection:kMaplyTestCategoryGestures row:kMaplyTestNorthUp];
        globeViewC.panGesture = [configViewC valueForSection:kMaplyTestCategoryGestures row:kMaplyTestPan];
        globeViewC.pinchGesture = [configViewC valueForSection:kMaplyTestCategoryGestures row:kMaplyTestPinch];
        globeViewC.rotateGesture = [configViewC valueForSection:kMaplyTestCategoryGestures row:kMaplyTestRotate];
    } else {
        if([configViewC valueForSection:kMaplyTestCategoryGestures row:kMaplyTestNorthUp]) {
            mapViewC.heading = 0;
        }
        mapViewC.panGesture = [configViewC valueForSection:kMaplyTestCategoryGestures row:kMaplyTestPan];
        mapViewC.pinchGesture = [configViewC valueForSection:kMaplyTestCategoryGestures row:kMaplyTestPinch];
        mapViewC.rotateGesture = [configViewC valueForSection:kMaplyTestCategoryGestures row:kMaplyTestRotate];
    }
    
    // Update rendering hints
    NSMutableDictionary *hintDict = [NSMutableDictionary dictionary];
    [hintDict setObject:[NSNumber numberWithBool:[configViewC valueForSection:kMaplyTestCategoryInternal row:kMaplyTestCulling]] forKey:kMaplyRenderHintCulling];
    [baseViewC setHints:hintDict];
}

- (void)showConfig
{
    if (UI_USER_INTERFACE_IDIOM() ==  UIUserInterfaceIdiomPad)
    {
        popControl = [[UIPopoverController alloc] initWithContentViewController:configViewC];
        popControl.delegate = self;
        [popControl setPopoverContentSize:CGSizeMake(400.0,4.0/5.0*self.view.bounds.size.height)];
        [popControl presentPopoverFromRect:CGRectMake(0, 0, 10, 10) inView:self.view permittedArrowDirections:UIPopoverArrowDirectionUp animated:YES];
    } else {
        configViewC.navigationItem.hidesBackButton = YES;
        configViewC.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(editDone)];
        [self.navigationController pushViewController:configViewC animated:YES];
    }
}

- (void)editDone
{
    [self.navigationController popToViewController:self animated:YES];
    [self changeMapContents];
}

#pragma mark - Whirly Globe Delegate

// Build a simple selection view to draw over top of the globe
- (UIView *)makeSelectionView:(NSString *)msg
{
    float fontSize = 32.0;
    float marginX = 32.0;
    
    // Make a label and stick it in as a view to track
    // We put it in a top level view so we can center it
    UIView *topView = [[UIView alloc] initWithFrame:CGRectZero];
    // Start out hidden before the first placement.  The tracker will turn it on.
    topView.hidden = YES;
    topView.alpha = 0.8;
    UIView *backView = [[UIView alloc] initWithFrame:CGRectZero];
    [topView addSubview:backView];
    topView.clipsToBounds = NO;
    UILabel *testLabel = [[UILabel alloc] initWithFrame:CGRectZero];
    [backView addSubview:testLabel];
    testLabel.font = [UIFont systemFontOfSize:fontSize];
    testLabel.textColor = [UIColor whiteColor];
    testLabel.backgroundColor = [UIColor clearColor];
    testLabel.text = msg;
    CGSize textSize = [testLabel.text sizeWithAttributes:@{NSFontAttributeName: testLabel.font}];
    testLabel.frame = CGRectMake(marginX/2.0,0,textSize.width,textSize.height);
    testLabel.opaque = NO;
    backView.layer.cornerRadius = 5.0;
    backView.backgroundColor = [UIColor colorWithRed:0.0 green:102/255.0 blue:204/255.0 alpha:1.0];
    backView.frame = CGRectMake(-(textSize.width)/2.0,-(textSize.height)/2.0,textSize.width+marginX,textSize.height);
    
    return topView;
}

- (void)handleSelection:(id)selectedObjs
{
    // If we've currently got a selected view, get rid of it
//    if (selectedViewTrack)
//    {
//        [baseViewC removeViewTrackForView:selectedViewTrack.view];
//        selectedViewTrack = nil;
//    }
    [baseViewC clearAnnotations];
    
    bool isMultiple = [selectedObjs isKindOfClass:[NSArray class]] && [(NSArray *)selectedObjs count] > 1;
    
    NSString *title = nil,*subTitle = nil;
    CGPoint offset = CGPointZero;
    MaplyCoordinate loc;
    if (isMultiple)
    {
        NSArray *selArr = selectedObjs;
        if ([selArr count] == 0)
            return;
        
        MaplySelectedObject *firstObj = [selArr objectAtIndex:0];
        // Only screen objects will be clustered
        if ([firstObj.selectedObj isKindOfClass:[MaplyScreenMarker class]])
        {
            MaplyScreenMarker *marker = firstObj.selectedObj;
            loc = marker.loc;
        } else if ([firstObj.selectedObj isKindOfClass:[MaplyScreenLabel class]])
        {
            MaplyScreenLabel *label = firstObj.selectedObj;
            loc = label.loc;
        } else
            return;
        
        title = @"Cluster";
        subTitle = [NSString stringWithFormat:@"%d objects",[selArr count]];
    } else {
        id selectedObj = nil;
        if ([selectedObjs isKindOfClass:[NSArray class]])
        {
            NSArray *selArr = selectedObjs;
            if ([selArr count] == 0)
                return;
            selectedObj = [(MaplySelectedObject *)[selArr objectAtIndex:0] selectedObj];
        } else
            selectedObj = selectedObjs;
        
        if ([selectedObj isKindOfClass:[MaplyMarker class]])
        {
            MaplyMarker *marker = (MaplyMarker *)selectedObj;
            loc = marker.loc;
            title = (NSString *)marker.userObject;
            subTitle = @"Marker";
        } else if ([selectedObj isKindOfClass:[MaplyScreenMarker class]])
        {
            MaplyScreenMarker *screenMarker = (MaplyScreenMarker *)selectedObj;
            loc = screenMarker.loc;
            title = (NSString *)screenMarker.userObject;
            subTitle = @"Screen Marker";
            offset = CGPointMake(0.0, -8.0);
        } else if ([selectedObj isKindOfClass:[MaplyLabel class]])
        {
            MaplyLabel *label = (MaplyLabel *)selectedObj;
            loc = label.loc;
            title = (NSString *)label.userObject;
            subTitle = @"Label";
        } else if ([selectedObj isKindOfClass:[MaplyScreenLabel class]])
        {
            MaplyScreenLabel *screenLabel = (MaplyScreenLabel *)selectedObj;
            loc = screenLabel.loc;
            title = (NSString *)screenLabel.userObject;
            subTitle = @"Screen Label";
            offset = CGPointMake(0.0, -6.0);
        } else if ([selectedObj isKindOfClass:[MaplyVectorObject class]])
        {
            MaplyVectorObject *vecObj = (MaplyVectorObject *)selectedObj;
            if ([vecObj centroid:&loc])
            {
                NSString *name = (NSString *)vecObj.userObject;
                title = (NSString *)vecObj.userObject;
                subTitle = @"Vector";
                if ([configViewC valueForSection:kMaplyTestCategoryObjects row:kMaplyTestLoftedPoly])
                {
                    // See if there already is one
                    if (!loftPolyDict[name])
                    {
                        MaplyComponentObject *compObj = [baseViewC addLoftedPolys:@[vecObj] key:nil cache:nil desc:
                                                            @{kMaplyColor: [UIColor colorWithRed:0.25 green:0.0 blue:0.0 alpha:0.25], kMaplyLoftedPolyHeight: @(0.05),
                                                              kMaplyFade: @(0.5),
                                                              kMaplyDrawPriority: @(kMaplyLoftedPolysDrawPriorityDefault),
    //                                                          kMaplyLoftedPolyOutline: @(YES),
    //                                                          kMaplyLoftedPolyOutlineBottom: @(YES),
    //                                                          kMaplyLoftedPolyOutlineColor: [UIColor whiteColor],
    //                                                          kMaplyLoftedPolyOutlineWidth: @(4),
    //                                                          kMaplyLoftedPolyOutlineDrawPriority: @(kMaplyLoftedPolysDrawPriorityDefault+1),
    //                                                          kMaplyLoftedPolyOutlineSide: @(YES)
                                                              }
                                                                             mode:MaplyThreadAny];
                        if (compObj)
                        {
                            loftPolyDict[name] = compObj;
                        }
                    }
                }
            }
        } else if ([selectedObj isKindOfClass:[MaplyShapeSphere class]])
        {
            MaplyShapeSphere *sphere = (MaplyShapeSphere *)selectedObj;
            loc = sphere.center;
            title = @"Shape";
            subTitle = @"Sphere";
        } else if ([selectedObj isKindOfClass:[MaplyShapeCylinder class]])
        {
            MaplyShapeCylinder *cyl = (MaplyShapeCylinder *)selectedObj;
            loc = cyl.baseCenter;
            title = @"Shape";
            subTitle = @"Cylinder";
        } else if ([selectedObj isKindOfClass:[MaplyShapeGreatCircle class]])
        {
            MaplyShapeGreatCircle *gc = (MaplyShapeGreatCircle *)selectedObj;
            loc = gc.startPt;
            title = @"Shape";
            subTitle = @"Great Circle";
        } else if ([selectedObj isKindOfClass:[MaplyShapeExtruded class]])
        {
            MaplyShapeExtruded *ex = (MaplyShapeExtruded *)selectedObj;
            loc = ex.center;
            title = @"Shape";
            subTitle = @"Extruded";
        } else if ([selectedObj isKindOfClass:[MaplyGeomModelInstance class]]) {
            MaplyGeomModelInstance *modelInst = (MaplyGeomModelInstance *)selectedObj;
            loc = MaplyCoordinateMake(modelInst.center.x,modelInst.center.y);
            title = @"Model";
            subTitle = @"Instance";
        } else
        {
            // Don't know what it is
            return;
        }
    }
    
    // Build the selection view and hand it over to the globe to track
//    selectedViewTrack = [[MaplyViewTracker alloc] init];
//    selectedViewTrack.loc = loc;
//    selectedViewTrack.view = [self makeSelectionView:[NSString stringWithFormat:@"%@: %@",title,subTitle]];
//    [baseViewC addViewTracker:selectedViewTrack];
//    
//    [self performSelector:@selector(moveViewTracker:) withObject:selectedViewTrack afterDelay:1.0];
    if (title)
    {
        MaplyAnnotation *annotate = [[MaplyAnnotation alloc] init];
        annotate.title = title;
        annotate.subTitle = subTitle;
        [baseViewC clearAnnotations];
        [baseViewC addAnnotation:annotate forPoint:loc offset:offset];
    }
}

// Test moving a view tracker
- (void)moveViewTracker:(MaplyViewTracker *)viewTrack
{
    [baseViewC moveViewTracker:viewTrack moveTo:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793)];
}

// User selected something
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj
{
    [self handleSelection:selectedObj];
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC allSelect:(NSArray *)selectedObjs atLoc:(MaplyCoordinate)coord onScreen:(CGPoint)screenPt
{
    [self handleSelection:selectedObjs];
}

// User didn't select anything, but did tap
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
    // Just clear the selection
    [baseViewC clearAnnotations];
    
    if (globeViewC)
    {
//        MaplyCoordinate geoCoord;
//        if ([globeViewC geoPointFromScreen:CGPointMake(0, 0) geoCoord:&geoCoord])
//            NSLog(@"GeoCoord (upper left): %f, %f",geoCoord.x,geoCoord.y);
//        else
//            NSLog(@"GeoCoord not on globe");
//        MaplyCoordinate geoCoord = MaplyCoordinateMakeWithDegrees(0, 0);
//        CGPoint screenPt;
//        if ([globeViewC screenPointFromGeo:geoCoord screenPt:&screenPt])
//            NSLog(@"Origin at: %f,%f",screenPt.x,screenPt.y);
//        else
//            NSLog(@"Origin not on screen");
    }
    
    // Screen shot
//    UIImage *image = [baseViewC snapshot];
    
//    if (selectedViewTrack)
//    {
//        [baseViewC removeViewTrackForView:selectedViewTrack.view];
//        selectedViewTrack = nil;
//    }
}

// Bring up the config view when the user taps outside
- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController *)viewC
{
//    [self showPopControl];
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC layerDidLoad:(MaplyViewControllerLayer *)layer
{
    NSLog(@"Spherical Earth Layer loaded.");
}

- (void)globeViewControllerDidStartMoving:(WhirlyGlobeViewController *)viewC userMotion:(bool)userMotion
{
//    NSLog(@"Started moving");
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC didStopMoving:(MaplyCoordinate *)corners userMotion:(bool)userMotion
{
//    NSLog(@"Globe Stopped moving");
}

#pragma mark - Maply delegate

- (void)maplyViewController:(MaplyViewController *)viewC didSelect:(NSObject *)selectedObj
{
    [self handleSelection:selectedObj];
}

- (void)maplyViewController:(MaplyViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
    // Poke the map
//    if (mapViewC)
//    {
//        MaplyCoordinate coord;
//        float height;
//        [mapViewC getPosition:&coord height:&height];
//        [mapViewC setPosition:coord height:height];
//    }

    // Just clear the selection
    [baseViewC clearAnnotations];
//    if (selectedViewTrack)
//    {
//        [baseViewC removeViewTrackForView:selectedViewTrack.view];
//        selectedViewTrack = nil;
//    }    
}

- (void)maplyViewControllerDidStartMoving:(MaplyViewController *)viewC userMotion:(bool)userMotion
{
//    NSLog(@"Started moving");
}

- (void)maplyViewController:(MaplyViewController *)viewC didStopMoving:(MaplyCoordinate *)corners userMotion:(bool)userMotion
{
//    NSLog(@"Maply Stopped moving");
}

#pragma mark - Popover Delegate

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController
{
    [self changeMapContents];
}


#pragma mark -
- (UIViewController * _Nullable)maplyViewController:(MaplyBaseViewController *)viewC
                  previewViewControllerForSelection:(NSObject *)selectedObj
{
  return [[UIViewController alloc] init];
}


- (void)maplyViewController:(MaplyBaseViewController *)viewC
  showPreviewViewController:(UIViewController *)previewViewC
{
  [self showViewController:previewViewC sender:self];
}

- (void)didSwipeScreen:(UISwipeGestureRecognizer *)gesture
{
    if (gesture.direction == UISwipeGestureRecognizerDirectionUp || gesture.direction == UISwipeGestureRecognizerDirectionDown)
        return;

    CGRect frame = scrollView.frame;
    if (gesture.direction == UISwipeGestureRecognizerDirectionLeft)
        frame.origin.x = frame.size.width;
    else
        frame.origin.x = 0.0;
    frame.origin.y = 0;
    [scrollView scrollRectToVisible:frame animated:YES];
}

#pragma mark - Gesture Recognizer Delegate

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {

    if (gestureRecognizer == scrollView.panGestureRecognizer) {
        if ([otherGestureRecognizer isKindOfClass:[UISwipeGestureRecognizer class]])
            return YES;
    } else if (otherGestureRecognizer == scrollView.panGestureRecognizer) {
        if ([gestureRecognizer isKindOfClass:[UISwipeGestureRecognizer class]])
            return YES;
    }
    return NO;
}

@end
