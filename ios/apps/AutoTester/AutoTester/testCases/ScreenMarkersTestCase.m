//
//  ScreenMarkersTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 6/16/16.
//  Copyright © 2016-2017 mousebird consulting.
//

#import "ScreenMarkersTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyMarker.h"
#import "VectorsTestCase.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyViewController.h"

@implementation ScreenMarkersTestCase
{
    VectorsTestCase *baseCase;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Screen Markers";
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
    }
    return self;
}


- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    baseCase = [[VectorsTestCase alloc]init];
    [baseCase setUpWithGlobe:globeVC];
    [self insertMarker:baseCase.vecList theView:(MaplyBaseViewController*)globeVC];
    [globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
    globeVC.autoMoveToTap = false;
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    baseCase = [[VectorsTestCase alloc]init];
    [baseCase setUpWithMap:mapVC];
    [self insertMarker:baseCase.vecList theView:(MaplyBaseViewController*)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
    mapVC.autoMoveToTap = false;
}

- (void)maplyViewController:(MaplyViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
    viewC.showDebugLayoutBoundaries = !viewC.showDebugLayoutBoundaries;
}
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
    viewC.showDebugLayoutBoundaries = !viewC.showDebugLayoutBoundaries;
}

- (void)stop
{
    if (self.markersObj)
    {
        [self.baseViewController removeObject:self.markersObj];
        self.markersObj = nil;
    }
    [baseCase stop];
    [super stop];
}

- (void) insertMarker:(NSMutableArray*) arrayComp theView: (MaplyBaseViewController*) theView
{
    UIImage *alcohol = [UIImage imageNamed:@"alcohol-shop-24@2x"];
    UIImage *airport = [UIImage imageNamed:@"airport-24@2x"];
    NSMutableArray *markers = [NSMutableArray array];
    for (MaplyVectorObject* object in arrayComp) {
//        MaplyMovingScreenMarker *marker = [[MaplyMovingScreenMarker alloc] init];
//        marker.image = alcohol;
//        marker.loc = object.center;
//        marker.endLoc = MaplyCoordinateMake(marker.loc.x+0.01, marker.loc.y+0.01);
//        marker.duration = 20.0;
//        marker.userObject = object.attributes[@"title"];
//        marker.selectable = true;
//        marker.layoutImportance = MAXFLOAT;

        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc]init];
        marker.image = (markers.count % 2) ? alcohol : airport;
        marker.loc = object.center;
        marker.size = CGSizeMake(32.0f, 32.0f);
        marker.userObject = object.attributes[@"title"];
        marker.selectable = true;

        // Use layout for some and not others
        marker.layoutImportance = (object.center.x < 0) ? 1.0 : MAXFLOAT;

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
