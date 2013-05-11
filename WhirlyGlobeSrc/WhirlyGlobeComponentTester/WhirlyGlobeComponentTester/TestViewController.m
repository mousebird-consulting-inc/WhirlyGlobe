/*
 *  TestViewController.m
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 7/23/12.
 *  Copyright 2011-2012 mousebird consulting
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
#import "AFJSONRequestOperation.h"

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

// Local interface for TestViewController
// We'll hide a few things here
@interface TestViewController ()
{
    // The configuration view comes up when the user taps outside the globe
    ConfigViewController *configViewC;
        
    // These represent a group of objects we've added to the globe.
    // This is how we track them for removal
    MaplyComponentObject *screenMarkersObj;
    MaplyComponentObject *markersObj;
    MaplyComponentObject *shapeCylObj;
    MaplyComponentObject *shapeSphereObj;
    MaplyComponentObject *greatCircleObj;
    MaplyComponentObject *screenLabelsObj;
    MaplyComponentObject *labelsObj;
    MaplyComponentObject *stickersObj;
    NSArray *vecObjects;
    MaplyComponentObject *megaMarkersObj;
    MaplyComponentObject *autoLabels;
    NSMutableDictionary *loftPolyDict;
    
    // The view we're using to track a selected object
    MaplyViewTracker *selectedViewTrack;
}

// These routine add objects to the globe based on locations and/or labels in the
//  locations passed in.  There's on locations array (for simplicity), so we just
//  pass in a stride and an offset so we're not putting two different objects in
//  the same place.
- (void)addMarkers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;
- (void)addScreenMarkers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;
- (void)addLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;
- (void)addScreenLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;
- (void)addStickers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;

// Change what we're showing based on the Configuration
- (void)changeMapContents;
@end

@implementation TestViewController
{
    MapType startupMapType;
    BaseLayer startupLayer;
}

- (id)initWithMapType:(MapType)mapType baseLayer:(BaseLayer)baseLayer
{
    self = [super init];
    if (self) {
        startupMapType = mapType;
        startupLayer = baseLayer;
    }
    return self;
}

- (void)dealloc
{
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
    
    loftPolyDict = [NSMutableDictionary dictionary];
    
    // Configuration controller for turning features on and off
    configViewC = [[ConfigViewController alloc] initWithNibName:@"ConfigViewController" bundle:nil];
    // Force the view to load so we can get the default switch values
    [configViewC view];

    // Create an empty globe or map controller
    if (startupMapType == MapGlobe)
    {
        globeViewC = [[WhirlyGlobeViewController alloc] init];
        globeViewC.delegate = self;
        baseViewC = globeViewC;
    } else {
        mapViewC = [[MaplyViewController alloc] init];
        mapViewC.delegate = self;
        baseViewC = mapViewC;
    }
    [self.view addSubview:baseViewC.view];
    baseViewC.view.frame = self.view.bounds;
    [self addChildViewController:baseViewC];
    
    // Set the background color for the globe
    baseViewC.clearColor = [UIColor blackColor];
    
    // This will get us taps and such
    if (globeViewC)
    {
        // Start up over San Francisco
        globeViewC.height = 0.8;
        [globeViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) time:1.0];
    } else {
        mapViewC.height = 1.0;
        [mapViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) time:1.0];
    }

    // For network paging layers, where we'll store temp files
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];

    // We'll pick default colors for the labels
    UIColor *screenLabelColor = [UIColor whiteColor];
    UIColor *screenLabelBackColor = [UIColor clearColor];
    UIColor *labelColor = [UIColor whiteColor];
    UIColor *labelBackColor = [UIColor clearColor];
    // And for the vectors to stand out
    UIColor *vecColor = [UIColor whiteColor];
    float vecWidth = 1.0;

    NSString *jsonTileSpec = nil;
    NSString *thisCacheDir = nil;
    
    // Set up the base layer
    switch (startupLayer)
    {
        case BlueMarbleSingleResLocal:
            self.title = @"Blue Marble Single Res";
            if (globeViewC)
            {
                // This is the static image set, included with the app, built with ImageChopper
                [globeViewC addSphericalEarthLayerWithImageSet:@"lowres_wtb_info"];
                screenLabelColor = [UIColor blackColor];
                screenLabelBackColor = [UIColor whiteColor];
                labelColor = [UIColor blackColor];
                labelBackColor = [UIColor whiteColor];
                vecColor = [UIColor whiteColor];
                vecWidth = 4.0;
            }
            break;
        case GeographyClassMBTilesLocal:
            self.title = @"Geography Class - MBTiles Local";
            // This is the Geography Class MBTiles data set from MapBox
            [baseViewC addQuadEarthLayerWithMBTiles:@"geography-class"];
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
            vecColor = [UIColor colorWithRed:0.4 green:0.4 blue:0.4 alpha:1.0];
            vecWidth = 4.0;
            break;
        case StamenWatercolorRemote:
        {
            self.title = @"Stamen Water Color - Remote";
            // These are the Stamen Watercolor tiles.
            // They're beautiful, but the server isn't so great.
            thisCacheDir = [NSString stringWithFormat:@"%@/stamentiles/",cacheDir];
            [baseViewC addQuadEarthLayerWithRemoteSource:@"http://tile.stamen.com/watercolor/" imageExt:@"png" cache:thisCacheDir minZoom:0 maxZoom:10];
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor blackColor];
            vecColor = [UIColor grayColor];
            vecWidth = 4.0;
        }
            break;
        case OpenStreetmapRemote:
        {
            self.title = @"OpenStreetMap - Remote";
            // This points to the OpenStreetMap tile set hosted by MapQuest (I think)
            thisCacheDir = [NSString stringWithFormat:@"%@/osmtiles/",cacheDir];
            [baseViewC addQuadEarthLayerWithRemoteSource:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/" imageExt:@"png" cache:thisCacheDir minZoom:0 maxZoom:17];
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
            vecColor = [UIColor blackColor];
            vecWidth = 4.0;
        }
            break;
        case MapBoxTilesSat1:
        {
            self.title = @"MapBox Tiles Satellite - Remote";
            jsonTileSpec = @"http://a.tiles.mapbox.com/v3/examples.map-zyt2v9k2.json";
            thisCacheDir = [NSString stringWithFormat:@"%@/mbtilessat1/",cacheDir];
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
            vecColor = [UIColor whiteColor];
            vecWidth = 4.0;
        }
            break;
        case MapBoxTilesTerrain1:
        {
            self.title = @"MapBox Tiles Terrain - Remote";
            jsonTileSpec = @"http://a.tiles.mapbox.com/v3/examples.map-zq0f1vuc.json";
            thisCacheDir = [NSString stringWithFormat:@"%@/mbtilesterrain1/",cacheDir];
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
            vecColor = [UIColor blackColor];
            vecWidth = 4.0;
        }
            break;
        case MapBoxTilesRegular1:
        {
            self.title = @"MapBox Tiles Regular - Remote";
            jsonTileSpec = @"http://a.tiles.mapbox.com/v3/examples.map-zswgei2n.json";
            thisCacheDir = [NSString stringWithFormat:@"%@/mbtilesregular1/",cacheDir];
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
            vecColor = [UIColor blackColor];
            vecWidth = 4.0;
        }
            break;
        default:
            break;
    }
    
    // Fill out the cache dir if there is one
    if (thisCacheDir)
    {
        NSError *error = nil;
        [[NSFileManager defaultManager] createDirectoryAtPath:thisCacheDir withIntermediateDirectories:YES attributes:nil error:&error];        
    }
    
    // If we're fetching one of the JSON tile specs, kick that off
    if (jsonTileSpec)
    {
        NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:jsonTileSpec]];
        
        AFJSONRequestOperation *operation =
        [AFJSONRequestOperation JSONRequestOperationWithRequest:request
                                        success:^(NSURLRequest *request, NSHTTPURLResponse *response, id JSON)
        {
            // Add a quad earth paging layer based on the tile spec we just fetched
            [baseViewC addQuadEarthLayerWithRemoteSource:JSON cache:thisCacheDir];
        }
                                                        failure:^(NSURLRequest *request, NSHTTPURLResponse *response, NSError *error, id JSON)
        {
            NSLog(@"Failed to reach JSON tile spec at: %@",jsonTileSpec);
        }
         ];
        
        [operation start];
    }
    
    // Set up some defaults for display
    NSDictionary *screenLabelDesc = [NSDictionary dictionaryWithObjectsAndKeys: 
                       screenLabelColor,kMaplyTextColor,
                       screenLabelBackColor,kMaplyBackgroundColor,
                       nil];
    [baseViewC setScreenLabelDesc:screenLabelDesc];
    NSDictionary *labelDesc = [NSDictionary dictionaryWithObjectsAndKeys: 
                 labelColor,kMaplyTextColor,
                 labelBackColor,kMaplyBackgroundColor,
                 nil];
    [baseViewC setLabelDesc:labelDesc];
    NSDictionary *vectorDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                vecColor,kMaplyColor,
                                [NSNumber numberWithFloat:vecWidth],kMaplyVecWidth,
                                nil];
    [baseViewC setVectorDesc:vectorDesc];
    
    // Maximum number of objects for the layout engine to display
    [baseViewC setMaxLayoutObjects:1000];
    
    // Bring up things based on what's turned on
    [self performSelector:@selector(changeMapContents) withObject:nil afterDelay:0.0];
    
    // Settings panel
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemEdit target:self action:@selector(showPopControl)];
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
    }
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

#pragma mark - Data Display

// Add screen (2D) markers at all our locations
- (void)addScreenMarkers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    CGSize size = CGSizeMake(40, 40);
    UIImage *pinImage = [UIImage imageNamed:@"map_pin"];
    
    NSMutableArray *markers = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.image = pinImage;
        marker.loc = MaplyCoordinateMakeWithDegrees(location->lon,location->lat);
        marker.size = size;
        marker.userObject = [NSString stringWithFormat:@"%s",location->name];
        [markers addObject:marker];
    }
    
    screenMarkersObj = [baseViewC addScreenMarkers:markers];
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
    
    markersObj = [baseViewC addMarkers:markers];
}

// Add screen (2D) labels
- (void)addScreenLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    CGSize size = CGSizeMake(0, 20);
    
    NSMutableArray *labels = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
        label.loc = MaplyCoordinateMakeWithDegrees(location->lon,location->lat);
        label.size = size;
        label.text = [NSString stringWithFormat:@"%s",location->name];
        label.userObject = [NSString stringWithFormat:@"%s",location->name];
        [labels addObject:label];
    }
    
    screenLabelsObj = [baseViewC addScreenLabels:labels];    
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
    
    labelsObj = [baseViewC addLabels:labels];        
}

// Add cylinders
- (void)addShapeCylinders:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    NSMutableArray *cyls = [[NSMutableArray alloc] init];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyShapeCylinder *cyl = [[MaplyShapeCylinder alloc] init];
        cyl.baseCenter = MaplyCoordinateMakeWithDegrees(location->lon, location->lat);
        cyl.radius = 0.01;
        cyl.height = 0.06;
        [cyls addObject:cyl];
    }
    
    shapeCylObj = [baseViewC addShapes:cyls];
}

// Add spheres
- (void)addShapeSpheres:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    NSMutableArray *spheres = [[NSMutableArray alloc] init];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
        sphere.center = MaplyCoordinateMakeWithDegrees(location->lon, location->lat);
        sphere.radius = 0.04;
        [spheres addObject:sphere];
    }

    shapeSphereObj = [baseViewC addShapes:spheres];
}

// Add spheres
- (void)addGreatCircles:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
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
        // This limits the height based on the length of the great circle
        float angle = [greatCircle calcAngleBetween];
        greatCircle.height = 0.3 * angle / M_PI;
        [circles addObject:greatCircle];
    }
    
    greatCircleObj = [baseViewC addShapes:circles];
}

- (void)addStickers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
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
        sticker.rotation = 2*M_PI * drand48();
        [stickers addObject:sticker];
    }
    
    stickersObj = [baseViewC addStickers:stickers];
}

// Add country outlines.  Pass in the names of the geoJSON files
- (void)addCountries:(NSArray *)names stride:(int)stride
{
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
                             MaplyVectorObject *wgVecObj = [MaplyVectorObject VectorObjectFromGeoJSON:jsonData];
                             NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
                             wgVecObj.userObject = vecName;
                             MaplyComponentObject *compObj = [baseViewC addVectors:[NSArray arrayWithObject:wgVecObj]];
                             MaplyScreenLabel *screenLabel = [[MaplyScreenLabel alloc] init];
                             // Add a label right in the middle
                             MaplyCoordinate center;
                             [wgVecObj largestLoopCenter:&center mbrLL:nil mbrUR:nil];
                             screenLabel.loc = center;
                             screenLabel.size = CGSizeMake(0, 20);
                             screenLabel.layoutImportance = 1.0;
                             screenLabel.text = vecName;
                             screenLabel.userObject = screenLabel.text;
                             if (screenLabel.text)
                                 [locAutoLabels addObject:screenLabel];
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
                                [baseViewC setScreenLabelDesc:@{kMaplyTextColor: [UIColor whiteColor], kMaplyBackgroundColor: [UIColor clearColor], kMaplyShadowSize: @(4.0)}];
                                MaplyComponentObject *autoLabelObj = [baseViewC addScreenLabels:locAutoLabels];
                                [baseViewC setScreenLabelDesc:@{kMaplyTextColor: [NSNull null], kMaplyBackgroundColor: [NSNull null], kMaplyShadowSize: [NSNull null]}];

                                vecObjects = locVecObjects;
                                autoLabels = autoLabelObj;
                            });

         }
    );
}

// Number of markers to whip up for the large test case
static const int NumMegaMarkers = 40000;

// Make up a large number of markers and add them
- (void)addMegaMarkers
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
       ^{
           UIImage *image = [UIImage imageNamed:@"map_pin.png"];
           NSMutableArray *markers = [NSMutableArray array];
           for (unsigned int ii=0;ii<NumMegaMarkers;ii++)
           {
               MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
               marker.image = image;
               marker.size = CGSizeMake(40,40);
               marker.loc = MaplyCoordinateMakeWithDegrees(drand48()*360-180, drand48()*140-70);
               marker.layoutImportance = drand48();
               [markers addObject:marker];
           }
           dispatch_async(dispatch_get_main_queue(),
                          ^{
                              megaMarkersObj = [baseViewC addScreenMarkers:markers];
                          }
                          );
       }
    );
}

// Look at the configuration controller and decide what to turn off or on
- (void)changeMapContents
{
    if (configViewC.label2DSwitch.on)
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

    if (configViewC.label3DSwitch.on)
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

    if (configViewC.marker2DSwitch.on)
    {
        if (!screenMarkersObj)
            [self addScreenMarkers:locations len:NumLocations stride:4 offset:2];
    } else {
        if (screenMarkersObj)
        {
            [baseViewC removeObject:screenMarkersObj];
            screenMarkersObj = nil;
        }
    }
    
    if (configViewC.marker3DSwitch.on)
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

    if (configViewC.stickerSwitch.on)
    {
        if (!stickersObj)
            [self addStickers:locations len:NumLocations stride:4 offset:2];
    } else {
        if (stickersObj)
        {
            [baseViewC removeObject:stickersObj];
            stickersObj = nil;
        }
    }

    if (configViewC.shapeCylSwitch.on)
    {
        if (!shapeCylObj)
        {
            [baseViewC setShapeDesc:@{kMaplyColor : [UIColor colorWithRed:0.0 green:0.0 blue:1.0 alpha:0.8]}];
            [self addShapeCylinders:locations len:NumLocations stride:4 offset:0];
            [baseViewC setShapeDesc:@{kMaplyColor : [NSNull null]}];
        }
    } else {
        if (shapeCylObj)
        {
            [baseViewC removeObject:shapeCylObj];
            shapeCylObj = nil;
        }
    }

    if (configViewC.shapeSphereSwitch.on)
    {
        if (!shapeSphereObj)
        {
            [baseViewC setShapeDesc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.8]}];
            [self addShapeSpheres:locations len:NumLocations stride:4 offset:1];
            [baseViewC setShapeDesc:@{kMaplyColor : [NSNull null]}];
        }
    } else {
        if (shapeSphereObj)
        {
            [baseViewC removeObject:shapeSphereObj];
            shapeSphereObj = nil;
        }
    }

    if (configViewC.shapeGreatCircleSwitch.on)
    {
        if (!greatCircleObj)
        {
            [baseViewC setShapeDesc:@{kMaplyColor : [UIColor colorWithRed:1.0 green:0.1 blue:0.0 alpha:1.0]}];
            [self addGreatCircles:locations len:NumLocations stride:4 offset:2];
            [baseViewC setShapeDesc:@{kMaplyColor : [NSNull null]}];
        }
    } else {
        if (greatCircleObj)
        {
            [baseViewC removeObject:greatCircleObj];
            greatCircleObj = nil;
        }
    }

    if (configViewC.countrySwitch.on)
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
    
    if (configViewC.loftPolySwitch.on)
    {
    } else {
        if ([loftPolyDict count] > 0)
        {
            [baseViewC removeObjects:loftPolyDict.allValues];
            loftPolyDict = [NSMutableDictionary dictionary];
        }
    }
    
    if (configViewC.megaMarkersSwitch.on)
    {
        if (!megaMarkersObj)
            [self addMegaMarkers];
    } else {
        if (megaMarkersObj)
        {
            [baseViewC removeObject:megaMarkersObj];
            megaMarkersObj = nil;
        }
    }
    
    baseViewC.performanceOutput = configViewC.perfSwitch.on;
    
    if (globeViewC)
    {
        globeViewC.keepNorthUp = configViewC.northUpSwitch.on;
        globeViewC.pinchGesture = configViewC.pinchSwitch.on;
        globeViewC.rotateGesture = configViewC.rotateSwitch.on;
    }
    
    // Update rendering hints
    NSMutableDictionary *hintDict = [NSMutableDictionary dictionary];
    [hintDict setObject:[NSNumber numberWithBool:configViewC.cullingSwitch.on] forKey:kMaplyRenderHintCulling];
    [hintDict setObject:[NSNumber numberWithBool:configViewC.zBufferSwitch.on] forKey:kMaplyRenderHintZBuffer];
    [baseViewC setHints:hintDict];
}

// Show the popup control panel
- (void)showPopControl
{
    popControl = [[UIPopoverController alloc] initWithContentViewController:configViewC];
    popControl.delegate = self;
    [popControl presentPopoverFromRect:CGRectMake(0, 0, 10, 10) inView:self.view permittedArrowDirections:UIPopoverArrowDirectionUp animated:YES];
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
    CGSize textSize = [testLabel.text sizeWithFont:testLabel.font];
    testLabel.frame = CGRectMake(marginX/2.0,0,textSize.width,textSize.height);
    testLabel.opaque = NO;
    backView.layer.cornerRadius = 5.0;
    backView.backgroundColor = [UIColor colorWithRed:0.0 green:102/255.0 blue:204/255.0 alpha:1.0];
    backView.frame = CGRectMake(-(textSize.width)/2.0,-(textSize.height)/2.0,textSize.width+marginX,textSize.height);
    
    return topView;
}

- (void)handleSelection:(NSObject *)selectedObj
{
    // If we've currently got a selected view, get rid of it
    if (selectedViewTrack)
    {
        [baseViewC removeViewTrackForView:selectedViewTrack.view];
        selectedViewTrack = nil;
    }
    
    MaplyCoordinate loc;
    NSString *msg = nil;
    
    if ([selectedObj isKindOfClass:[MaplyMarker class]])
    {
        MaplyMarker *marker = (MaplyMarker *)selectedObj;
        loc = marker.loc;
        msg = [NSString stringWithFormat:@"Marker: %@",marker.userObject];
    } else if ([selectedObj isKindOfClass:[MaplyScreenMarker class]])
    {
        MaplyScreenMarker *screenMarker = (MaplyScreenMarker *)selectedObj;
        loc = screenMarker.loc;
        msg = [NSString stringWithFormat:@"Screen Marker: %@",screenMarker.userObject];
    } else if ([selectedObj isKindOfClass:[MaplyLabel class]])
    {
        MaplyLabel *label = (MaplyLabel *)selectedObj;
        loc = label.loc;
        msg = [NSString stringWithFormat:@"Label: %@",label.userObject];
    } else if ([selectedObj isKindOfClass:[MaplyScreenLabel class]])
    {
        MaplyScreenLabel *screenLabel = (MaplyScreenLabel *)selectedObj;
        loc = screenLabel.loc;
        msg = [NSString stringWithFormat:@"Screen Label: %@",screenLabel.userObject];
    } else if ([selectedObj isKindOfClass:[MaplyVectorObject class]])
    {
        MaplyVectorObject *vecObj = (MaplyVectorObject *)selectedObj;
        [vecObj largestLoopCenter:&loc mbrLL:nil mbrUR:nil];
        NSString *name = (NSString *)vecObj.userObject;
        msg = [NSString stringWithFormat:@"Vector: %@",vecObj.userObject];
        if (configViewC.loftPolySwitch.on)
        {
            // See if there already is one
            if (!loftPolyDict[name])
            {
                [baseViewC setLoftedPolyDesc:@{kMaplyColor: [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.25], kMaplyLoftedPolyHeight: @(0.05)}];
                MaplyComponentObject *compObj = [baseViewC addLoftedPolys:@[vecObj] key:nil cache:nil];
                if (compObj)
                {
                    loftPolyDict[name] = compObj;
                }
                [baseViewC setLoftedPolyDesc:@{kMaplyColor: [NSNull null], kMaplyLoftedPolyHeight: [NSNull null]}];
            }
        }
    } else
        // Don't know what it is
        return;
    
    // Build the selection view and hand it over to the globe to track
    selectedViewTrack = [[MaplyViewTracker alloc] init];
    selectedViewTrack.loc = loc;
    selectedViewTrack.view = [self makeSelectionView:msg];
    [baseViewC addViewTracker:selectedViewTrack];    
}

// User selected something
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj
{
    [self handleSelection:selectedObj];
}

// User didn't select anything, but did tap
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
    // Just clear the selection
    if (selectedViewTrack)
    {
        [baseViewC removeViewTrackForView:selectedViewTrack.view];
        selectedViewTrack = nil;        
    }
}

// Bring up the config view when the user taps outside
- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController *)viewC
{
    [self showPopControl];
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC layerDidLoad:(MaplyViewControllerLayer *)layer
{
    NSLog(@"Spherical Earth Layer loaded.");
}

#pragma mark - Maply delegate

- (void)maplyViewController:(MaplyViewController *)viewC didSelect:(NSObject *)selectedObj
{
    [self handleSelection:selectedObj];
}

- (void)maplyViewController:(MaplyViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
    // Just clear the selection
    if (selectedViewTrack)
    {
        [baseViewC removeViewTrackForView:selectedViewTrack.view];
        selectedViewTrack = nil;
    }    
}

#pragma mark - Popover Delegate

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController
{
    [self changeMapContents];
}

@end
