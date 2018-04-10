/*
 *  MapboxVectorTilesPagingDelegate.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Copyright 2011-2017 mousebird consulting
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

#import "MapboxVectorTilesPagingDelegate.h"
#import "MaplyTileSource.h"
#import "MapboxVectorStyleSet.h"
#import "MapboxVectorStyleBackground.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>

#import "CoordSystem.h"
#import "MaplyRemoteTileSource.h"
#import "MaplyVectorStyle.h"
#import "MaplyVectorObject_private.h"
#import "MaplyScreenLabel.h"
#import "NSData+Zlib.h"
#import "vector_tile.pb.h"
#import "VectorData.h"
#import "MaplyMBTileSource.h"
#import "MapnikStyleSet.h"
#import "MaplyRenderController_private.h"

using namespace WhirlyKit;

static double MAX_EXTENT = 20037508.342789244;

@interface MapboxVectorTilesPagingDelegate ()
@property (nonatomic, strong, readwrite) NSArray *tileSources;

@end

@implementation MapboxVectorTilesPagingDelegate

- (instancetype) initWithTileSources:(NSArray*)tileSources style:(NSObject<MaplyVectorStyleDelegate> *)style viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if(self) {
        self.tileSources = tileSources;
        _tileParser = [[MapboxVectorTileParser alloc] initWithStyle:style viewC:viewC];
    }
    return self;
}

- (instancetype) initWithTileSource:(MaplyRemoteTileInfo*)tileSource style:(NSObject<MaplyVectorStyleDelegate> *)style viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [self initWithTileSources:@[tileSource] style:style viewC:viewC];
    return self;
}

- (instancetype) initWithMBTiles:(MaplyMBTileSource *)tileSource style:(NSObject<MaplyVectorStyleDelegate> *)style viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [self initWithTileSources:@[tileSource] style:style viewC:viewC];
    return self;
}

- (void)dealloc
{
    _tileSources = nil;
}

- (void)setAccessToken:(NSString *)accessToken
{
    _accessToken = accessToken;
    
    for (MaplyRemoteTileSource *tileSource in _tileSources)
    {
        if ([tileSource.tileInfo isKindOfClass:[MaplyRemoteTileInfo class]])
        {
            ((MaplyRemoteTileInfo *)tileSource.tileInfo).queryStr = [NSString stringWithFormat:@"access_token=%@",_accessToken];
        }
    }
}

#pragma mark - MaplyPagingDelegate

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    //    NSLog(@"%@ startFetchForTile: %d/%d/%d", NSStringFromClass([self class]), tileID.level,tileID.x,tileID.y);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       MaplyBoundingBox bbox;
                       if (!layer.valid)
                           return;
                       [layer geoBoundsforTile:tileID ll:&bbox.ll ur:&bbox.ur];
                       MaplyBoundingBox geoBbox = bbox;
                       bbox.ll = [self toMerc:bbox.ll];
                       bbox.ur = [self toMerc:bbox.ur];
                       
                       NSMutableArray *compObjs = [NSMutableArray array];
                       
                       for(NSObject<MaplyTileSource> *tileSource in self.tileSources) {
                           if(tileID.level > tileSource.maxZoom || tileID.level < tileSource.minZoom) {
                               //this should probably check validTile, but that could be slower
                               continue;
                           }
                           if (!layer.valid)
                               return;
                           
                           MaplyTileID flippedYTile;
                           if(layer.flipY) {
                               flippedYTile.level = tileID.level;
                               flippedYTile.x = tileID.x;
                               flippedYTile.y = ((int)(1<<tileID.level)-tileID.y)-1;
                           } else {
                               flippedYTile = tileID;
                           }
                           NSData *tileData = [tileSource imageForTile:flippedYTile];
                           
                           if(tileData.length) {
                               if([tileData isCompressed]) {
                                   tileData = [tileData uncompressGZip];
                                   if(!tileData.length) {
                                       NSLog(@"Error: tile data was nil after decompression");
                                       continue;
                                   }
                               }
                               
                               // If the app shuts down the layer while we're working
                               if (!layer.valid)
                                   break;
                               
                               MaplyVectorTileData *retData = nil;
                               MaplyRenderController *renderControl = [layer.viewC getRenderControl];
                               if ([renderControl startOfWork])
                               {
                                   retData = [self->_tileParser buildObjects:tileData tile:tileID bounds:bbox geoBounds:geoBbox];
                                   if (!retData)
                                       NSLog(@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,flippedYTile.y);
                                   
                                   [renderControl endOfWork];
                               }
                               
                               if (retData)
                               {
                                   // Keep track of the component objects created
                                   if ([retData.compObjs count] > 0)
                                       [compObjs addObjectsFromArray:retData.compObjs];
                                   // Note: Ignoring rasters
                               }
                           } else {
                               // Empty tile
                           }
                       }
                       
                       //        if (!layer.valid)
                       //            return;
                       
                       // Hand the objects over to the layer
                       [layer addData:compObjs forTile:tileID style:MaplyDataStyleReplace];
                       [layer tileDidLoad:tileID];
                       
                       //        NSLog(@"%@ finished load: %d/%d/%d for %d objects", NSStringFromClass([self class]), tileID.level,tileID.x,tileID.y,[compObjs count]);
                       
                       // Note: Turn this back on for debugging
                       //    CFTimeInterval duration = CFAbsoluteTimeGetCurrent() - start;
                       //    NSLog(@"Added %lu components for %d features for tile %d/%d/%d in %f seconds",
                       //          (unsigned long)components.count, featureCount,
                       //          tileID.level, tileID.x, tileID.y,
                       //          duration);
                   });
}


- (int)minZoom
{
    if (_minZoom != 0)
        return _minZoom;
    
    if(self.tileSources.count) {
        id tileSource = self.tileSources[0];
        if ([tileSource isKindOfClass:[MaplyMBTileSource class]])
            return [(MaplyMBTileSource *)tileSource minZoom];
        return [(MaplyRemoteTileInfo*)self.tileSources[0] minZoom];
    } else {
        return 3;
    }
}


- (int)maxZoom
{
    if(self.tileSources.count) {
        id tileSource = self.tileSources[0];
        if ([tileSource isKindOfClass:[MaplyMBTileSource class]])
            return [(MaplyMBTileSource *)tileSource maxZoom];
        return [(NSObject <MaplyTileSource>*)self.tileSources[0] maxZoom];
    } else {
        return 14;
    }
}


/**
 Convert a coordinate from lat/lon radians to epsg:3785
 Verified output with "cs2cs +init=epsg:4326 +to +init=epsg:3785", correct within .5 meters,
 but frequently off by .4
 */
- (MaplyCoordinate)toMerc:(MaplyCoordinate)coord {
    //  MaplyCoordinate orig = coord;
    coord.x = RadToDeg(coord.x) * MAX_EXTENT / 180;
    coord.y = 3189068.5 * log((1.0 + sin(coord.y)) / (1.0 - sin(coord.y)));
    //  NSLog(@"%f %f -> %.2f %.2f", RadToDeg(orig.x), RadToDeg(orig.y), coord.x, coord.y);
    return coord;
}


@end
