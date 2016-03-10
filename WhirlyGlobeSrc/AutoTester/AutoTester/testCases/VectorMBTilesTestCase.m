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
    MaplyVectorTiles *vecTiles;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Vector MBTiles";
        self.captureDelay = 20;
    }
    
    return self;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{
    GeographyClassTestCase *baseView = [[GeographyClassTestCase alloc]init];
    [baseView setUpWithMap:mapVC];
    
    // For network paging layers, where we'll store temp files
    vecTiles = [[MaplyVectorTiles alloc] initWithDatabase:@"France" viewC:mapVC];
    MaplyCoordinateSystem *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    MaplyQuadPagingLayer *layer = [[MaplyQuadPagingLayer alloc] initWithCoordSystem:coordSys delegate:vecTiles];
    [mapVC addLayer:layer];
        
    return true;
}

@end
