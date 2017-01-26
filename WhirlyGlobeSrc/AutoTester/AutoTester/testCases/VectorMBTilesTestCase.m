//
//  VectorMBTilesTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 3/9/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import <WhirlyGlobeComponent.h>
#import "VectorMBTilesTestCase.h"
#import "GeographyClassTestCase.h"

@implementation VectorMBTilesTestCase
{
    MapboxVectorTiles *vecTiles;
    MaplyVectorStyleSimpleGenerator *simpleStyle;
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
    GeographyClassTestCase *baseView = [[GeographyClassTestCase alloc]init];
    [baseView setUpWithMap:mapVC];

    // Simple style with random colors
    simpleStyle = [[MaplyVectorStyleSimpleGenerator alloc] initWithViewC:mapVC];

    // Set up a loader for Mapbox Vector tiles in an MBTiles
    MaplyMBTileSource *tileSource = [[MaplyMBTileSource alloc] initWithMBTiles:@"France"];
    vecTiles = [[MapboxVectorTiles alloc] initWithMBTiles:tileSource style:simpleStyle viewC:mapVC];
    MaplyQuadPagingLayer *layer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:tileSource.coordSys delegate:vecTiles];
    layer.flipY = false;
    [mapVC addLayer:layer];
}

@end
