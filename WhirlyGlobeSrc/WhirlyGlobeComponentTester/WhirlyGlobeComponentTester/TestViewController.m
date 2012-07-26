//
//  TestViewController.m
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 7/23/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import "TestViewController.h"

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
    {"San Francisco",37.7793, -122.4192},
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
    {"Kansas City",39.1, -94.58},
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
    
    // These are default visual descriptions for the various data types
    NSDictionary *screenMarkerDesc;
    NSDictionary *markerDesc;
    NSDictionary *screenLabelDesc;
    NSDictionary *labelDesc;
    
    // These represent a group of objects we've added to the globe.
    // This is how we track them for removal
    WGComponentObject *screenMarkersObj;
    WGComponentObject *markersObj;
    WGComponentObject *screenLabelsObj;
    WGComponentObject *labelsObj;
    
    // The view we're using to track a selected object
    WGViewTracker *selectedViewTrack;
}

// These routine add objects to the globe based on locations and/or labels in the
//  locations passed in.  There's on locations array (for simplicity), so we just
//  pass in a stride and an offset so we're not putting two different objects in
//  the same place.
- (void)addMarkers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;
- (void)addScreenMarkers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;
- (void)addLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;
- (void)addScreenLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset;

// Change what we're showing based on the Configuration
- (void)changeGlobeContents;
@end

@implementation TestViewController
{
    BaseLayer startupLayer;
}

- (id)initWithBaseLayer:(BaseLayer)baseLayer;
{
    self = [super init];
    if (self) {
        startupLayer = baseLayer;
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // Configuration controller for turning features on and off
    configViewC = [[ConfigViewController alloc] initWithNibName:@"ConfigViewController" bundle:nil];
    // Force the view to load so we can get the default switch values
    [configViewC view];
    
    // Create an empty globe view controller and hook it in to our view hiearchy
    globeViewC = [[WhirlyGlobeViewController alloc] init];
    [self.view addSubview:globeViewC.view];
    globeViewC.view.frame = self.view.bounds;
    [self addChildViewController:globeViewC];
    
    // This will get us taps and such
    globeViewC.delegate = self;
    
    // Start up over San Francisco
    [globeViewC animateToPosition:WGCoordinateMakeWithDegrees(-122.4192, 37.7793) time:1.0];

    // For network paging layers, where 
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];

    // We'll pick default colors for the labels
    UIColor *screenLabelColor = [UIColor whiteColor];
    UIColor *screenLabelBackColor = [UIColor clearColor];
    UIColor *labelColor = [UIColor whiteColor];
    UIColor *labelBackColor = [UIColor clearColor];

    // Set up the base layer
    switch (startupLayer)
    {
        case BlueMarbleSingleResLocal:
            // This is the static image set, included with the app, built with ImageChopper
            [globeViewC addSphericalEarthLayerWithImageSet:@"lowres_wtb_info"];
            break;
        case GeographyClassMBTilesLocal:
            // This is the Geography Class MBTiles data set from MapBox
            [globeViewC addQuadEarthLayerWithMBTiles:@"geography-class"];
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
            break;
        case StamenWatercolorRemote:
        {
            // These are the Stamen Watercolor tiles.
            // They're beautiful, but the server isn't so great.
            NSString *thisCacheDir = [NSString stringWithFormat:@"%@/stamentiles/",cacheDir];
            NSError *error = nil;
            [[NSFileManager defaultManager] createDirectoryAtPath:thisCacheDir withIntermediateDirectories:YES attributes:nil error:&error];
            [globeViewC addQuadEarthLayerWithRemoteSource:@"http://tile.stamen.com/watercolor/" imageExt:@"png" cache:thisCacheDir minZoom:2 maxZoom:10];
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
        }
            break;
        case OpenStreetmapRemote:
        {
            // This points to the OpenStreetMap tile set hosted by MapQuest (I think)
            NSString *thisCacheDir = [NSString stringWithFormat:@"%@/osmtiles/",cacheDir];
            NSError *error = nil;
            [[NSFileManager defaultManager] createDirectoryAtPath:thisCacheDir withIntermediateDirectories:YES attributes:nil error:&error];
            [globeViewC addQuadEarthLayerWithRemoteSource:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/" imageExt:@"png" cache:thisCacheDir minZoom:0 maxZoom:12];            
            screenLabelColor = [UIColor blackColor];
            screenLabelBackColor = [UIColor whiteColor];
            labelColor = [UIColor blackColor];
            labelBackColor = [UIColor whiteColor];
        }
            break;
        default:
            break;
    }
    
    // Set up some defaults for display
    screenLabelDesc = [NSDictionary dictionaryWithObjectsAndKeys: 
                       screenLabelColor,kWGTextColor,
                       screenLabelBackColor,kWGBackgroundColor,
                       nil];
    [globeViewC setScreenLabelDesc:screenLabelDesc];
    labelDesc = [NSDictionary dictionaryWithObjectsAndKeys: 
                 labelColor,kWGTextColor,
                 labelBackColor,kWGBackgroundColor,
                 nil];
    [globeViewC setLabelDesc:labelDesc];
    
    // Bring up things based on what's turned on
    [self performSelector:@selector(changeGlobeContents) withObject:nil afterDelay:0.0];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    // This should release the globe view
    [globeViewC removeFromParentViewController];
    globeViewC = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

#pragma mark - Data Display

// Add screen (2D) markers at all our locations
- (void)addScreenMarkers:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    CGSize size = CGSizeMake(20, 20);
    UIImage *pinImage = [UIImage imageNamed:@"map_pin"];
    
    NSMutableArray *markers = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        WGScreenMarker *marker = [[WGScreenMarker alloc] init];
        marker.image = pinImage;
        marker.loc = WGCoordinateMakeWithDegrees(location->lon,location->lat);
        marker.size = size;
        [markers addObject:marker];
    }
    
    screenMarkersObj = [globeViewC addScreenMarkers:markers];
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
        WGMarker *marker = [[WGMarker alloc] init];
        marker.image = startImage;
        marker.loc = WGCoordinateMakeWithDegrees(location->lon,location->lat);
        marker.size = size;
        [markers addObject:marker];
    }
    
    markersObj = [globeViewC addMarkers:markers];
}

// Add screen (2D) labels
- (void)addScreenLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    CGSize size = CGSizeMake(0, 20);
    
    NSMutableArray *labels = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        WGScreenLabel *label = [[WGScreenLabel alloc] init];
        label.loc = WGCoordinateMakeWithDegrees(location->lon,location->lat);
        label.size = size;
        label.text = [NSString stringWithFormat:@"%s",location->name];
        [labels addObject:label];
    }
    
    screenLabelsObj = [globeViewC addScreenLabels:labels];    
}

// Add 3D labels
- (void)addLabels:(LocationInfo *)locations len:(int)len stride:(int)stride offset:(int)offset
{
    CGSize size = CGSizeMake(0, 0.05);
    
    NSMutableArray *labels = [NSMutableArray array];
    for (unsigned int ii=offset;ii<len;ii+=stride)
    {
        LocationInfo *location = &locations[ii];
        WGLabel *label = [[WGLabel alloc] init];
        label.loc = WGCoordinateMakeWithDegrees(location->lon,location->lat);
        label.size = size;
        label.text = [NSString stringWithFormat:@"%s",location->name];
        [labels addObject:label];
    }
    
    labelsObj = [globeViewC addLabels:labels];        
}

// Look at the configuration controller and decide what to turn off or on
- (void)changeGlobeContents
{
    if (configViewC.label2DSwitch.on)
    {
        if (!screenLabelsObj)
            [self addScreenLabels:locations len:NumLocations stride:4 offset:0];
    } else {
        if (screenLabelsObj)
        {
            [globeViewC removeObject:screenLabelsObj];
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
            [globeViewC removeObject:labelsObj];
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
            [globeViewC removeObject:screenMarkersObj];
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
            [globeViewC removeObject:markersObj];
            markersObj = nil;
        }
    }    
    
    globeViewC.keepNorthUp = configViewC.northUpSwitch.on;
    
    // Update rendering hints
    NSMutableDictionary *hintDict = [NSMutableDictionary dictionary];
    [hintDict setObject:[NSNumber numberWithBool:configViewC.cullingSwitch.on] forKey:kWGRenderHintCulling];
    [hintDict setObject:[NSNumber numberWithBool:configViewC.zBufferSwitch.on] forKey:kWGRenderHintZBuffer];
    [globeViewC setHints:hintDict];
}

#pragma mark - Whirly Globe Delegate

// User selected something
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj
{
    // If we've currently got a selected view, get rid of it
    if (selectedViewTrack)
    {
        [globeViewC removeViewTrackForView:selectedViewTrack.view];
        selectedViewTrack = nil;
    }
    
    WGCoordinate loc;
    NSString *msg = nil;
    
    if ([selectedObj isKindOfClass:[WGMarker class]])
    {
        WGMarker *marker = (WGMarker *)selectedObj;    
        loc = marker.loc;
        msg = [NSString stringWithFormat:@"Marker: %d",marker.image];
    } else if ([selectedObj isKindOfClass:[WGScreenMarker class]])
    {
        WGScreenMarker *screenMarker = (WGScreenMarker *)selectedObj;        
        loc = screenMarker.loc;
        msg = [NSString stringWithFormat:@"Screen Marker: %d",screenMarker.image];
    } else if ([selectedObj isKindOfClass:[WGLabel class]])
    {
        WGLabel *label = (WGLabel *)selectedObj;        
        loc = label.loc;
        msg = [NSString stringWithFormat:@"Label: %@",label.text];
    } else if ([selectedObj isKindOfClass:[WGScreenLabel class]])
    {
        WGScreenLabel *screenLabel = (WGScreenLabel *)selectedObj;        
        loc = screenLabel.loc;
        msg = [NSString stringWithFormat:@"Screen Label: %@",screenLabel.text];
    } else
        // Don't know what it is
        return;
    
    // Make a label and stick it in as a view to track
    UILabel *testLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 200, 40)];
    testLabel.backgroundColor = [UIColor blueColor];
    testLabel.text = msg;
    selectedViewTrack = [[WGViewTracker alloc] init];
    selectedViewTrack.loc = loc;
    selectedViewTrack.view = testLabel;
    [globeViewC addViewTracker:selectedViewTrack];
    
//    NSLog(@"User selected: %@",[selectedObj description]);
}

// Bring up the config view when the user taps outside
- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController *)viewC
{
    popControl = [[UIPopoverController alloc] initWithContentViewController:configViewC];
    popControl.delegate = self;
    [popControl presentPopoverFromRect:CGRectMake(0, 0, 10, 10) inView:self.view permittedArrowDirections:UIPopoverArrowDirectionUp animated:YES];
}

#pragma mark - Popover Delegate

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController
{
    [self changeGlobeContents];
}

@end
