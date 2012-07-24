//
//  TestViewController.m
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 7/23/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import "TestViewController.h"

@interface TestViewController ()

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
    
    // Create an empty globe view controller and hook it in
    globeViewC = [[WhirlyGlobeViewController alloc] init];
    globeViewC.delegate = self;
    [self.view addSubview:globeViewC.view];
    globeViewC.view.frame = self.view.bounds;
    [self addChildViewController:globeViewC];
    
    // Start up over San Francisco
    // Woo!  California represent!
    [globeViewC animateToPosition:WGCoordinateMakeWithDegrees(-122.4192, 37.7793) time:1.0];
//    globeViewC.selection = false;
//    [globeViewC animateToPosition:WGCoordinateMakeWithDegrees(-77.036667, 38.895111) time:1.0];
    
    // Note: temporary
    [self addMarkers];
    
    // Set up the base layer
    [globeViewC addSphericalEarthLayerWithImageSet:@"lowres_wtb_info"];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    [globeViewC removeFromParentViewController];
    globeViewC = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

#pragma mark - Data Display

// Some random locations
const char *locationNames[] = {"San Francisco","Washington, DC","Manila","Moscow","London","Caracas","Lagos","Sydney","Seattle","Tokyo","McMurdo Station","Tehran","Santiago","Pretoria","Perth","Beijing","New Delhi",NULL};
// Coordinates in lon, lat
const float locations[] = {37.7793, -122.4192, 38.895111,-77.036667,14.583333,120.966667,55.75, 37.616667, 51.507222, -0.1275, 10.5, -66.916667, 6.453056, 3.395833, -33.859972, 151.211111, 47.609722, -122.333056, 35.689506, 139.6917, -77.85, 166.666667, 35.696111, 51.423056, -33.45, -70.666667, -25.746111, 28.188056, -31.952222, 115.858889, 39.913889, 116.391667, 28.613889, 77.208889};	

// Add markers at all our locations
- (void)addMarkers
{
    CGSize size = CGSizeMake(20, 20);
    UIImage *pinImage = [UIImage imageNamed:@"map_pin"];
    
    NSMutableArray *markers = [NSMutableArray array];
    for (unsigned int ii=0;locationNames[ii];ii++)
    {
        WGScreenMarker *marker = [[WGScreenMarker alloc] init];
        marker.image = pinImage;
        marker.loc = WGCoordinateMakeWithDegrees(locations[2*ii+1], locations[2*ii]);
        marker.size = size;
        [markers addObject:marker];
    }
    
    [globeViewC addScreenMarkers:markers];
}

#pragma mark - Whirly Globe Delegate

// User selected something
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj
{
    NSLog(@"User selected: %@",[selectedObj description]);
}

// Bring up the config view when the user taps outside
- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController *)viewC
{
    ConfigViewController *configViewC = [[ConfigViewController alloc] initWithNibName:@"ConfigViewController" bundle:nil];
    popControl = [[UIPopoverController alloc] initWithContentViewController:configViewC];
    [popControl presentPopoverFromRect:CGRectMake(0, 0, 10, 10) inView:self.view permittedArrowDirections:UIPopoverArrowDirectionUp animated:YES];
}

@end
