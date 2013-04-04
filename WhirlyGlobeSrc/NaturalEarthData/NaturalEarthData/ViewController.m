/*
 *  ViewController.m
 *  NaturalEarthData
 *
 *  Created by Steve Gifford on 12/23/12.
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

#import "ViewController.h"
#import "WhirlyGlobeComponent.h"

@interface ViewController ()

@end

@implementation ViewController
{
    WhirlyGlobeViewController *globeViewC;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Create an empty globe view controller and hook it in to our view hiearchy
    globeViewC = [[WhirlyGlobeViewController alloc] init];
    [self.view addSubview:globeViewC.view];
    globeViewC.view.frame = self.view.bounds;
    [self addChildViewController:globeViewC];
    
    // Toss up the background image
    [globeViewC addSphericalEarthLayerWithImageSet:@"NE1_HR_LC_SR_W_DR"];
    
    // Set the background color for the globe
    globeViewC.clearColor = [UIColor blackColor];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc
{
    // This should release the globe view
    if (globeViewC)
    {
        [globeViewC.view removeFromSuperview];
        [globeViewC removeFromParentViewController];
        globeViewC = nil;
    }
}

@end
