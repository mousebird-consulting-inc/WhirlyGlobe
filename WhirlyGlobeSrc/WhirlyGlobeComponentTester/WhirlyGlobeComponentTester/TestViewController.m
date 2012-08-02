//
//  TestViewController.m
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 7/23/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
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
    WGComponentObject *screenMarkersObj;
    WGComponentObject *markersObj;
    WGComponentObject *screenLabelsObj;
    WGComponentObject *labelsObj;
    NSArray *vecObjects;
    
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
    
    // Set the background color for the globe
    globeViewC.clearColor = [UIColor blackColor];
    
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
    // And for the vectors to stand out
    UIColor *vecColor = [UIColor whiteColor];
    float vecWidth = 1.0;

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
            vecColor = [UIColor brownColor];
            vecWidth = 2.0;
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
            vecColor = [UIColor brownColor];
            vecWidth = 2.0;
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
            vecColor = [UIColor brownColor];
            vecWidth = 2.0;
        }
            break;
        default:
            break;
    }
    
    // Set up some defaults for display
    NSDictionary *screenLabelDesc = [NSDictionary dictionaryWithObjectsAndKeys: 
                       screenLabelColor,kWGTextColor,
                       screenLabelBackColor,kWGBackgroundColor,
                       nil];
    [globeViewC setScreenLabelDesc:screenLabelDesc];
    NSDictionary *labelDesc = [NSDictionary dictionaryWithObjectsAndKeys: 
                 labelColor,kWGTextColor,
                 labelBackColor,kWGBackgroundColor,
                 nil];
    [globeViewC setLabelDesc:labelDesc];
    NSDictionary *vectorDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                vecColor,kWGColor,
                                [NSNumber numberWithFloat:vecWidth],kWGVecWidth,
                                nil];
    [globeViewC setVectorDesc:vectorDesc];
    
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

// Add country outlines.  Pass in the names of the geoJSON files
- (void)addCountries:(NSArray *)names stride:(int)stride
{
    // Parsing the JSON can take a while, so let's hand that over to another queue
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), 
         ^{
             NSMutableArray *locVecObjects = [NSMutableArray array];
             
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
                             WGVectorObject *wgVecObj = [WGVectorObject VectorObjectFromGeoJSON:jsonData];
                             WGComponentObject *compObj = [globeViewC addVectors:[NSArray arrayWithObject:wgVecObj]];
                             if (compObj)
                                 [locVecObjects addObject:compObj];
                         }
                     }
                 }
                 ii++;

                 // Keep track of the created objects
                 // Note: You could lose track of the objects if you turn the countries on/off quickly
                 dispatch_async(dispatch_get_main_queue(),
                                ^{
                                    vecObjects = locVecObjects;
                                });
             }
         }
    );
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
            [globeViewC removeObjects:vecObjects];
            vecObjects = nil;
        }
    }
    
    globeViewC.keepNorthUp = configViewC.northUpSwitch.on;
    globeViewC.pinchGesture = configViewC.pinchSwitch.on;
    globeViewC.rotateGesture = configViewC.rotateSwitch.on;
    
    // Update rendering hints
    NSMutableDictionary *hintDict = [NSMutableDictionary dictionary];
    [hintDict setObject:[NSNumber numberWithBool:configViewC.cullingSwitch.on] forKey:kWGRenderHintCulling];
    [hintDict setObject:[NSNumber numberWithBool:configViewC.zBufferSwitch.on] forKey:kWGRenderHintZBuffer];
    [globeViewC setHints:hintDict];
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
        msg = [NSString stringWithFormat:@"Marker: Unknown"];
    } else if ([selectedObj isKindOfClass:[WGScreenMarker class]])
    {
        WGScreenMarker *screenMarker = (WGScreenMarker *)selectedObj;        
        loc = screenMarker.loc;
        msg = [NSString stringWithFormat:@"Screen Marker: Unknown"];
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

    // Build the selection view and hand it over to the globe to track
    selectedViewTrack = [[WGViewTracker alloc] init];
    selectedViewTrack.loc = loc;
    selectedViewTrack.view = [self makeSelectionView:msg];
    [globeViewC addViewTracker:selectedViewTrack];
}

// User didn't select anything, but did tap
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didTapAt:(WGCoordinate)coord
{
    // Just clear the selection
    if (selectedViewTrack)
    {
        [globeViewC removeViewTrackForView:selectedViewTrack.view];
        selectedViewTrack = nil;        
    }
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
