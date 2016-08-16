//
//  VectorHoleTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 8/11/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "VectorHoleTestCase.h"

@implementation VectorHoleTestCase



- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Vectors with Holes";
        self.captureDelay = 5;
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        
    }
    
    return self;
}


- (void) overlayVectors: (MaplyBaseViewController*) baseVC
{
    NSDictionary *vectorDict = @{
                                 kMaplyColor: [UIColor redColor],
                                 kMaplySelectable: @(true),
//                                 kMaplyFilled: @(YES),
                                 kMaplyVecWidth: @(4.0)};
    
    //Opp A (Anti-clockwise)
       MaplyCoordinate coord[5] = {MaplyCoordinateMakeWithDegrees(-2, 27),MaplyCoordinateMakeWithDegrees(-35, 27),MaplyCoordinateMakeWithDegrees(-35, -12),MaplyCoordinateMakeWithDegrees(-2, -12),MaplyCoordinateMakeWithDegrees(-2, 27)}; // = (MaplyCoordinate *)[pol.polyline bytes];
    
    //Opp B (clockwise)
//    MaplyCoordinate coord[5] = {MaplyCoordinateMakeWithDegrees(-35,27 ),MaplyCoordinateMakeWithDegrees(-2, 27),MaplyCoordinateMakeWithDegrees(-2, -12),MaplyCoordinateMakeWithDegrees(-35, -12),MaplyCoordinateMakeWithDegrees(-35,27 )};
    
    
    MaplyVectorObject *mainVector =  [[MaplyVectorObject alloc] initWithAreal:coord numCoords:5 attributes:nil];
    
    // Op A (Anti-clockwise) HOLE
//    MaplyCoordinate coordHo[5] = {MaplyCoordinateMakeWithDegrees(-20, 5),MaplyCoordinateMakeWithDegrees(-20, -5),MaplyCoordinateMakeWithDegrees(-10, -5),MaplyCoordinateMakeWithDegrees(-10, 5),MaplyCoordinateMakeWithDegrees(-20, 5)};
    
    // Op B (clockwise) HOLE
  MaplyCoordinate coordHo[5] = {MaplyCoordinateMakeWithDegrees(-20, 5),MaplyCoordinateMakeWithDegrees(-10, 5),MaplyCoordinateMakeWithDegrees(-10, -5),MaplyCoordinateMakeWithDegrees(-20, -5),MaplyCoordinateMakeWithDegrees(-20, 5)};
    
    [mainVector addHole:coordHo numCoords:5];

//    [mainVector subdivideToGlobe:0.001];
    mainVector = [mainVector clipToGrid:CGSizeMake(10.0/180.0*M_PI,10.0/180.0*M_PI)];
    mainVector = [mainVector tesselate];
    
    [baseVC addVectors:@[mainVector] desc:vectorDict];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    self.baseView = [[GeographyClassTestCase alloc]init];
    [self.baseView setUpWithGlobe:globeVC];
    //Overlay Countries
    [self overlayVectors:(MaplyBaseViewController*)globeVC];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    self.baseView = [[GeographyClassTestCase alloc]init];
    [self.baseView setUpWithMap:mapVC];
    [self overlayVectors:(MaplyBaseViewController*)mapVC];
}

- (void) handleSelection:(MaplyBaseViewController *)viewC
                selected:(NSObject *)selectedObj
{
    // ensure it's a MaplyVectorObject. It should be one of our outlines.
    if ([selectedObj isKindOfClass:[MaplyVectorObject class]]) {
        MaplyVectorObject *theVector = (MaplyVectorObject *)selectedObj;
        MaplyCoordinate location;
        
        if ([theVector centroid:&location]) {
            MaplyAnnotation *annotate = [[MaplyAnnotation alloc]init];
            annotate.title = @"Selected";
            annotate.subTitle = (NSString *)theVector.userObject;
            [viewC addAnnotation:annotate forPoint:location offset:CGPointZero];
        }
    }
}

@end
