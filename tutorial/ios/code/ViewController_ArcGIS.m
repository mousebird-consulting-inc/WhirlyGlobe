//
//  ViewController.m
//  HelloEarth
//
//  Created by Steve Gifford on 11/11/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import "ViewController.h"
#import "WhirlyGlobeComponent.h"
#import "CartoDBLayer.h"
#import "ArcGISLayer.h"

@interface ViewController ()

- (void) addCountries;
- (void) addAnnotation:(NSString *)title withSubtitle:(NSString *)subtitle at: (MaplyCoordinate)coord;

@end

@implementation ViewController
{
    MaplyBaseViewController *theViewC;
    WhirlyGlobeViewController *globeViewC;
    MaplyViewController *mapViewC;
    NSDictionary *vectorDict;
}


// Set this to false for a map
const bool DoGlobe = true;

- (void)viewDidLoad {
    [super viewDidLoad];
    
    if (DoGlobe)
    {
        globeViewC = [[WhirlyGlobeViewController alloc] init];
        theViewC = globeViewC;
    } else {
        mapViewC = [[MaplyViewController alloc] init];
        theViewC = mapViewC;
    }
    // If you're doing a globe
    if (globeViewC != nil)
        globeViewC.delegate = self;
    
    // If you're doing a map
    if (mapViewC != nil)
        mapViewC.delegate = self;
    
    // Create an empty globe or map and add it to the view
    [self.view addSubview:theViewC.view];
    theViewC.view.frame = self.view.bounds;
    [self addChildViewController:theViewC];
    
    // we want a black background for a globe, a white background for a map.
    theViewC.clearColor = (globeViewC != nil) ? [UIColor blackColor] : [UIColor whiteColor];
    
    // and thirty fps if we can get it ­ change this to 3 if you find your app is struggling
    theViewC.frameInterval = 2;
    
    // add the capability to use the local tiles or remote tiles
    bool useLocalTiles = false;
    
    // we'll need this layer in a second
    MaplyQuadImageTilesLayer *layer;
    
    if (useLocalTiles)
    {
        MaplyMBTileSource *tileSource =
        [[MaplyMBTileSource alloc] initWithMBTiles:@"geography­-class_medres"];
        layer = [[MaplyQuadImageTilesLayer alloc]
                 initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
    } else {
        // Because this is a remote tile set, we'll want a cache directory
        NSString *baseCacheDir =
        [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)
         objectAtIndex:0];
        NSString *tilesCacheDir = [NSString stringWithFormat:@"%@/tiles/",
                                         baseCacheDir];
        int maxZoom = 17;
        
    // Example ArcGIS base map layers
        // http://services.arcgisonline.com/arcgis/rest/services/NatGeo_World_Map/MapServer
        // http://services.arcgisonline.com/arcgis/rest/services/World_Physical_Map/MapServer
        // http://services.arcgisonline.com/arcgis/rest/services/Ocean_Basemap/MapServer
        
    // Stamen Terrain Tiles, courtesy of Stamen Design under the Creative Commons Attribution License.
        // http://tile.stamen.com/terrain/
        
        MaplyRemoteTileSource *tileSource =
        [[MaplyRemoteTileSource alloc]
         initWithBaseURL:@"http://services.arcgisonline.com/arcgis/rest/services/NatGeo_World_Map/MapServer/tile/{z}/{y}/{x}"
         ext:@"png" minZoom:0 maxZoom:maxZoom];
        tileSource.cacheDir = tilesCacheDir;
        layer = [[MaplyQuadImageTilesLayer alloc]
                 initWithCoordSystem:tileSource.coordSys tileSource:tileSource];
    }
    layer.handleEdges = (globeViewC != nil);
    layer.coverPoles = (globeViewC != nil);
    layer.requireElev = false;
    layer.waitLoad = false;
    layer.drawPriority = 0;
    layer.singleLevelLoading = false;
    [theViewC addLayer:layer];
    
    // start up over New York
    if (globeViewC != nil)
    {
        globeViewC.height = 0.006;
        globeViewC.heading = 0.15;
        globeViewC.tilt = 0.25;         // PI/2 radians = horizon??
        [globeViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-73.90,40.75)
                                 time:1.0];
    } else {
        globeViewC.height = 0.0002;
        [mapViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-73.99,40.75)
                               time:1.0];
    }
    
// Adding a little sauce - AutoTilt
    [globeViewC setTiltMinHeight:0.01 maxHeight:0.1 minTilt:0.4 maxTilt:0.0];
    
    // set the vector characteristics to be pretty and selectable
    vectorDict = @{
                   kMaplyColor: [UIColor whiteColor],
                   kMaplySelectable: @(true),
                   kMaplyVecWidth: @(4.0)};
    
    // add the countries
   // [self addCountries];
    
    // add the bar icons
    //    [self addBars];
    
    // add some spheres
    //    [self addSpheres];
    
    // add the CartoDB layer
//    [self addBuildings];
    
// add the ArcGIS layer
    [self addVectorLayer];
}

- (void)addVectorLayer
{
    NSString *search = @"WHERE=Zone>=1&f=pgeojson&outSR=4326";
    //     NSString *search = @"SELECT the_geom,address,ownername,numfloors FROM mn_mappluto_13v1 WHERE the_geom && ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326) LIMIT 2000;";
    
    ArcGISLayer *vectorLayer = [[ArcGISLayer alloc] initWithSearch:search];
    vectorLayer.minZoom = 10;   //15
    vectorLayer.maxZoom = 10;
    MaplySphericalMercator *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    MaplyQuadPagingLayer *quadLayer =
    [[MaplyQuadPagingLayer alloc] initWithCoordSystem:coordSys delegate:vectorLayer];
    [theViewC addLayer:quadLayer];
}


- (void)addSpheres
{
    // set up some locations
    MaplyCoordinate capitals[10];
    capitals[0] = MaplyCoordinateMakeWithDegrees(-77.036667, 38.895111);
    capitals[1] = MaplyCoordinateMakeWithDegrees(120.966667, 14.583333);
    capitals[2] = MaplyCoordinateMakeWithDegrees(55.75, 37.616667);
    capitals[3] = MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222);
    capitals[4] = MaplyCoordinateMakeWithDegrees(-66.916667, 10.5);
    capitals[5] = MaplyCoordinateMakeWithDegrees(139.6917, 35.689506);
    capitals[6] = MaplyCoordinateMakeWithDegrees(166.666667, -77.85);
    capitals[7] = MaplyCoordinateMakeWithDegrees(-58.383333, -34.6);
    capitals[8] = MaplyCoordinateMakeWithDegrees(-74.075833, 4.598056);
    capitals[9] = MaplyCoordinateMakeWithDegrees(-79.516667, 8.983333);
    
    // work through the spheres
    NSMutableArray *spheres = [NSMutableArray array];
    for (unsigned int ii=0;ii<10;ii++)
    {
        MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
        sphere.center = capitals[ii];
        sphere.radius = 0.01;
        [spheres addObject:sphere];
    }
    [theViewC addShapes:spheres desc:
     @{
       kMaplyColor: [UIColor colorWithRed:0.75 green:0.0 blue:0.0 alpha:0.75]
       }];
}

- (void)addBars
{
    // set up some locations
    MaplyCoordinate capitals[10];
    capitals[0] = MaplyCoordinateMakeWithDegrees(-77.036667, 38.895111);
    capitals[1] = MaplyCoordinateMakeWithDegrees(120.966667, 14.583333);
    capitals[2] = MaplyCoordinateMakeWithDegrees(55.75, 37.616667);
    capitals[3] = MaplyCoordinateMakeWithDegrees(-0.1275, 51.507222);
    capitals[4] = MaplyCoordinateMakeWithDegrees(-66.916667, 10.5);
    capitals[5] = MaplyCoordinateMakeWithDegrees(139.6917, 35.689506);
    capitals[6] = MaplyCoordinateMakeWithDegrees(166.666667, -77.85);
    capitals[7] = MaplyCoordinateMakeWithDegrees(-58.383333, -34.6);
    capitals[8] = MaplyCoordinateMakeWithDegrees(-74.075833, 4.598056);
    capitals[9] = MaplyCoordinateMakeWithDegrees(-79.516667, 8.983333);
    
    // get the image and create the markers
    UIImage *icon = [UIImage imageNamed:@"alcohol-shop-24@2x.png"];
    NSMutableArray *markers = [NSMutableArray array];
    for (unsigned int ii=0;ii<10;ii++)
    {
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.image = icon;
        marker.loc = capitals[ii];
        marker.size = CGSizeMake(40,40);
        [markers addObject:marker];
    }
    // add them all at once (for efficency)
    [theViewC addScreenMarkers:markers desc:nil];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)addCountries
{
    // handle this in another thread
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0),
                   ^{
                       NSArray *allOutlines = [[NSBundle mainBundle] pathsForResourcesOfType:@"geojson" inDirectory:nil];
                       
                       for (NSString *outlineFile in allOutlines)
                       {
                           NSData *jsonData = [NSData dataWithContentsOfFile:outlineFile];
                           if (jsonData)
                           {
                               MaplyVectorObject *wgVecObj = [MaplyVectorObject VectorObjectFromGeoJSON:jsonData];
                               
                               // the admin tag from the country outline geojson has the country name ­ save
                               NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
                               wgVecObj.userObject = vecName;
                               
                               // add the outline to our view
                               MaplyComponentObject *compObj = [theViewC addVectors:[NSArray arrayWithObject:wgVecObj] desc:vectorDict];
                               // If you ever intend to remove these, keep track of the MaplyComponentObjects above.
                               
                               // Add a screen label per country
                               if ([vecName length] > 0)
                               {
                                   MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
                                   label.text = vecName;
                                   label.loc = [wgVecObj center];
                                   label.selectable = true;
                                   [theViewC addScreenLabels:@[label] desc:
                                    @{
                                      kMaplyFont: [UIFont boldSystemFontOfSize:24.0],
                                      kMaplyTextOutlineColor: [UIColor blackColor],
                                      kMaplyTextOutlineSize: @(2.0),
                                      kMaplyColor: [UIColor whiteColor]
                                      }];
                               }
                           }
                       }
                   });
}

- (void)addBuildings
{
    NSString *search = @"WHERE=Zone>=1&f=pgeojson&outSR=4326";
    // NSString *search = @"SELECT the_geom,address,ownername,numfloors FROM mn_mappluto_13v1 WHERE the_geom && ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326) LIMIT 2000;";
    
    CartoDBLayer *cartoLayer = [[CartoDBLayer alloc] initWithSearch:search];
    cartoLayer.minZoom = 9;
    cartoLayer.maxZoom = 13;
    MaplySphericalMercator *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    MaplyQuadPagingLayer *quadLayer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:coordSys delegate:cartoLayer];
    [theViewC addLayer:quadLayer];
}

- (void)addAnnotation:(NSString *)title withSubtitle:(NSString *)subtitle at:(MaplyCoordinate)coord
{
    [theViewC clearAnnotations];
    
    MaplyAnnotation *annotation = [[MaplyAnnotation alloc] init];
    annotation.title = title;
    annotation.subTitle = subtitle;
    [theViewC addAnnotation:annotation forPoint:coord offset:CGPointZero];
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC
                   didTapAt:(MaplyCoordinate)coord
{
    NSString *title = @"Tap Location:";
    NSString *subtitle = [NSString stringWithFormat:@"(%.2fN, %.2fE)",
                          coord.y*57.296,coord.x*57.296];
    [self addAnnotation:title withSubtitle:subtitle at:coord];
}

- (void)maplyViewController:(MaplyViewController *)viewC
                   didTapAt:(MaplyCoordinate)coord
{
    NSString *title = @"Tap Location:";
    NSString *subtitle = [NSString stringWithFormat:@"(%.2fN, %.2fE)",
                          coord.y*57.296,coord.x*57.296];
    [self addAnnotation:title withSubtitle:subtitle at:coord];
}

// Unified method to handle the selection
- (void) handleSelection:(MaplyBaseViewController *)viewC
                selected:(NSObject *)selectedObj
{
    // ensure it's a MaplyVectorObject. It should be one of our outlines.
    if ([selectedObj isKindOfClass:[MaplyVectorObject class]])
    {
        MaplyVectorObject *theVector = (MaplyVectorObject *)selectedObj;
        MaplyCoordinate location;
        
        if ([theVector centroid:&location])
        {
            NSString *title = @"Selected:";
            NSString *subtitle = (NSString *)theVector.userObject;
            [self addAnnotation:title withSubtitle:subtitle at:location];
        }
    } else if ([selectedObj isKindOfClass:[MaplyScreenMarker class]])
    {
        // or it might be a screen marker
        MaplyScreenMarker *theMarker = (MaplyScreenMarker *)selectedObj;
        
        NSString *title = @"Selected:";
        NSString *subtitle = @"Screen Marker";
        [self addAnnotation:title withSubtitle:subtitle at:theMarker.loc];
    }
}

// This is the version for a globe
- (void) globeViewController:(WhirlyGlobeViewController *)viewC
                   didSelect:(NSObject *)selectedObj
{
    [self handleSelection:viewC selected:selectedObj];
}


// This is the version for a map
- (void) maplyViewController:(MaplyViewController *)viewC
                   didSelect:(NSObject *)selectedObj
{
    [self handleSelection:viewC selected:selectedObj];
}

@end
