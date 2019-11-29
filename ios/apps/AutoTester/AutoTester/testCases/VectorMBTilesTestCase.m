//
//  VectorMBTilesTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 3/9/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import <WhirlyGlobeComponent.h>
#import "VectorMBTilesTestCase.h"
#import "GeographyClassTestCase.h"

@implementation VectorMBTilesTestCase
{
    MapboxVectorTilesPagingDelegate *vecTiles;
    MaplyVectorStyleSimpleGenerator *simpleStyle;
    MaplyQuadPagingLayer *layer;
    GeographyClassTestCase *baseCase;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Vector MBTiles";
        self.captureDelay = 20;
        self.implementations = MaplyTestCaseOptionMap;
    }
    
    return self;
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    baseCase = [[GeographyClassTestCase alloc]init];
    [baseCase setUpWithMap:mapVC];
    
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(3.773839, 43.754354) height:0.035 time:0.2];

    // Simple style with random colors
    simpleStyle = [[MaplyVectorStyleSimpleGenerator alloc] initWithViewC:mapVC];

    // Set up a loader for Mapbox Vector tiles in an MBTiles
    MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc] initWithMBTiles:@"France"];
    vecTiles = [[MapboxVectorTilesPagingDelegate alloc] initWithMBTiles:tileSource style:simpleStyle viewC:mapVC];
    layer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:tileSource.coordSys delegate:vecTiles];
    layer.flipY = false;
    [mapVC addLayer:layer];
}

- (void)tearDownWithMap:(MaplyViewController *)mapVC
{
    [baseCase tearDownWithMap:mapVC];
    
    [mapVC removeLayer:layer];
    layer = nil;
}

@end
