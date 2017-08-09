//
//  ScreenMarkersTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 6/16/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "ScreenMarkersTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyMarker.h"
#import "VectorsTestCase.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyViewController.h"

@implementation ScreenMarkersTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Screen Markers";
        self.captureDelay = 4;
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
    }
    return self;
}


- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    VectorsTestCase * baseView = [[VectorsTestCase alloc]init];
    [baseView setUpWithGlobe:globeVC];
    [self insertMarker:baseView.compList theView:(MaplyBaseViewController*)globeVC];
    [globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    VectorsTestCase * baseView = [[VectorsTestCase alloc]init];
    [baseView setUpWithMap:mapVC];
    [self insertMarker:baseView.compList theView:(MaplyBaseViewController*)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
}

- (void) insertMarker:(NSMutableArray*) arrayComp theView: (MaplyBaseViewController*) theView
{
    UIImage *alcohol = [UIImage imageNamed:@"alcohol-shop-24@2x"];
    NSMutableArray *markers = [NSMutableArray array];
    for (MaplyVectorObject* object in arrayComp) {
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc]init];
        marker.image = alcohol;
        marker.loc = object.center;
        marker.userObject = object.userObject;
        marker.selectable = true;
        [markers addObject:marker];
    }
    self.markersObj = [theView addScreenMarkers:markers desc:nil];
    
    // Disable them slightly later
//    [self performSelector:@selector(disableObjects) withObject:nil afterDelay:10.0];
}

// Test disable and selection
- (void)disableObjects
{
    [self.baseViewController disableObjects:@[self.markersObj] mode:MaplyThreadCurrent];
}

@end
