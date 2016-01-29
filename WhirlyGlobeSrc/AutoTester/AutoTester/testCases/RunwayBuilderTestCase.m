//
//  RunwayBuilderTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 1/28/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "RunwayBuilderTestCase.h"
#import "AutoTester-swift.h"

@implementation RunwayBuilderTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.captureDelay = 20;
        self.name = @"Runway Builder";
    }
    return self;
}

// This is a simple fake runway, intended as a test.
// This is not a real runway
- (MaplyGeomModel *)buildRunwayModel:(MaplyBaseViewController *)viewC
{
    // Color states
    MaplyGeomState *tarmacState = [[MaplyGeomState alloc] init];
    tarmacState.color = [UIColor blackColor];
    MaplyGeomState *stripeState = [[MaplyGeomState alloc] init];
    stripeState.color = [UIColor whiteColor];
    MaplyGeomState *yellowStripeState = [[MaplyGeomState alloc] init];
    yellowStripeState.color = [UIColor yellowColor];
    
    MaplyGeomBuilder *wholeBuilder = [[MaplyGeomBuilder alloc] init];
    
    // Various size constants
    double width = 80.0;
    double overrun = 200.0;
    double displaced = 200.0;
    double runway = 2000.0;
    double total = overrun+displaced+runway;

    // Overrun on the end
    {
        MaplyGeomBuilder *geomBuilder = [[MaplyGeomBuilder alloc] init];
        [geomBuilder addRectangleAroundX:0.0 y:overrun/2.0 width:width height:overrun state:tarmacState];
        
        [wholeBuilder addGeomFromBuilder:geomBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:0.0 y:0.0 z:0.0]];
    }

    // Next up, the displaced
    {
        MaplyGeomBuilder *geomBuilder = [[MaplyGeomBuilder alloc] init];
        [geomBuilder addRectangleAroundX:0.0 y:displaced/2.0 width:width height:displaced state:tarmacState];
        
        [wholeBuilder addGeomFromBuilder:geomBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:0.0 y:overrun z:0.0]];
    }
    
    // Main part of the runway
    {
        MaplyGeomBuilder *geomBuilder = [[MaplyGeomBuilder alloc] init];
        [geomBuilder addRectangleAroundX:0.0 y:runway/2.0 width:width height:runway state:tarmacState];
        
        [wholeBuilder addGeomFromBuilder:geomBuilder transform:[[MaplyMatrix alloc] initWithTranslateX:(overrun+displaced) y:0.0 z:0.0]];
    }
    
    // Balance it in the middle and then tilt it
    double runwayMinZ = 100.0;
    double runwayMaxZ = 110.0;
    double rot = asin((runwayMaxZ-runwayMinZ)/total);
    [wholeBuilder rotate:rot aroundX:1.0 y:0.0 z:0.0];
    [wholeBuilder translateX:0.0 y:0.0 z:(runwayMinZ+runwayMaxZ)/2.0];
    
    return [wholeBuilder makeGeomModel:MaplyThreadCurrent];
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC {
    MapBoxSatelliteTestCase *baseView = [[MapBoxSatelliteTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    
    // Build the model
    MaplyGeomModel *geomModel = [self buildRunwayModel:globeVC];
    
    // Use the model
    MaplyGeomModelInstance *modelInst = [[MaplyGeomModelInstance alloc] init];
    modelInst.model = geomModel;
    MaplyCoordinate coord = MaplyCoordinateMakeWithDegrees(-122.270833, 37.804444);
    modelInst.center = MaplyCoordinate3dMake(coord.x,coord.y,200.0);
    
    [globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.270833, 37.804444) time:1.0];
    return true;
}


@end
