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

@implementation TestViewController
{
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
    
    // Create an empty Maply View Controller and hook it in
    mapViewC = [[MaplyViewController alloc] init];
    [self.view addSubview:mapViewC.view];
    mapViewC.view.frame = self.view.bounds;
    [self addChildViewController:mapViewC];
    
    switch (startupLayer)
    {
        case GeographyClassMBTilesLocal:
            // This is the Geography Class MBTiles data set from MapBox
            [mapViewC addQuadEarthLayerWithMBTiles:@"geography-class"];
            break;
        default:
            break;
    }
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

@end
