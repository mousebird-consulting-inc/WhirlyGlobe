/*
 *  TestViewController.m
 *  MaplyComponentTester
 *
 *  Created by Steve Gifford on 9/7/12.
 *  Copyright 2012 mousebird consulting
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

#import "TestViewController.h"
#import "ConfigViewController.h"

@interface TestViewController()<UIPopoverControllerDelegate>
@end

@implementation TestViewController
{
    // The configuration view comes up when the user taps outside the globe
    ConfigViewController *configViewC;
    UIPopoverController *popControl;

    // The image layer we'll start with
    MaplyTestBaseLayer startupLayer;
}

- (id)initWithBaseLayer:(MaplyTestBaseLayer)baseLayer
{
    self = [super init];
    if (self)
    {
        startupLayer = baseLayer;
    }
    
    return self;
}

- (void)dealloc
{
    if (mapViewC)
    {
        [mapViewC.view removeFromSuperview];
        [mapViewC removeFromParentViewController];
        mapViewC = nil;
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Configuration controller for turning features on and off
    configViewC = [[ConfigViewController alloc] initWithNibName:@"ConfigViewController" bundle:nil];
    // Force the view to load so we can get the default switch values
    [configViewC view];

    // Create an empty Maply View Controller and hook it in
    mapViewC = [[MaplyViewController alloc] init];
    [self.view addSubview:mapViewC.view];
    mapViewC.view.frame = self.view.bounds;
    [self addChildViewController:mapViewC];
    
    // For network paging layers, where we'll store temp files
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];

    switch (startupLayer)
    {
        case GeographyClassMBTilesLocal:
            // This is the Geography Class MBTiles data set from MapBox
            [mapViewC addQuadEarthLayerWithMBTiles:@"geography-class"];
            break;
        case StamenWatercolorRemote:
        {
            // These are the Stamen Watercolor tiles.
            // They're beautiful, but the server isn't so great.
            NSString *thisCacheDir = [NSString stringWithFormat:@"%@/stamentiles/",cacheDir];
            NSError *error = nil;
            [[NSFileManager defaultManager] createDirectoryAtPath:thisCacheDir withIntermediateDirectories:YES attributes:nil error:&error];
            [mapViewC addQuadEarthLayerWithRemoteSource:@"http://tile.stamen.com/watercolor/" imageExt:@"png" cache:thisCacheDir minZoom:2 maxZoom:10];
        }
            break;
        case OpenStreetmapRemote:
        {
            // This points to the OpenStreetMap tile set hosted by MapQuest (I think)
            NSString *thisCacheDir = [NSString stringWithFormat:@"%@/osmtiles/",cacheDir];
            NSError *error = nil;
            [[NSFileManager defaultManager] createDirectoryAtPath:thisCacheDir withIntermediateDirectories:YES attributes:nil error:&error];
            [mapViewC addQuadEarthLayerWithRemoteSource:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/" imageExt:@"png" cache:thisCacheDir minZoom:0 maxZoom:17];
        }
            break;            
        default:
            break;
    }
    
    // Toss a button into the corner to bring up configuration
    UIBarButtonItem *barButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemEdit target:self action:@selector(configAction:)];
    self.navigationItem.rightBarButtonItem = barButton;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    // Release the map
    if (mapViewC)
    {
        [mapViewC.view removeFromSuperview];
        [mapViewC removeFromParentViewController];
        mapViewC = nil;
    }
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

#pragma mark - Button actions

- (void)configAction:(id)sender
{
    popControl = [[UIPopoverController alloc] initWithContentViewController:configViewC];
    popControl.delegate = self;
    [popControl presentPopoverFromRect:CGRectMake(0, 0, 10, 10) inView:self.view permittedArrowDirections:UIPopoverArrowDirectionUp animated:YES];    
}

@end
