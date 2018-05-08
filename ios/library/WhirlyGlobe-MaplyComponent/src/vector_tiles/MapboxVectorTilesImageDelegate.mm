/*
 *  MapboxVectorTilesImageDelegate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on January 24 2018
 *  Copyright 2011-2018 Saildrone
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

#import "MapboxVectorTilesImageDelegate.h"
#import "MapboxVectorTiles.h"
#import "MaplyTileSource.h"
#import "MapboxVectorStyleSet.h"
#import "MapboxVectorStyleBackground.h"
#import "MaplyQuadImageLoader.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <set>

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

@implementation MapboxVectorTileImageSource
{
    MaplyRemoteTileInfo *tileInfo;
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    MapboxVectorStyleSet *imageStyle,*vecStyle;
    MaplySphericalMercator *coordSys;
    MaplyRenderController *offlineRender;
    UIColor *backColor;
    
    MapboxVectorTileParser *imageTileParser,*vecTileParser;
}

- (instancetype _Nullable ) initWithTileInfo:(MaplyRemoteTileInfo *_Nonnull)inTileInfo
                                  imageStyle:(MapboxVectorStyleSet *__nonnull)inImageStyle
                               offlineRender:(MaplyRenderController *__nonnull)inOfflineRender
                                 vectorStyle:(MapboxVectorStyleSet *__nonnull)inVectorStyle
                                       viewC:(MaplyBaseViewController *__nonnull)inViewC
{
    self = [super init];
    tileInfo = inTileInfo;
    imageStyle = inImageStyle;
    offlineRender = inOfflineRender;
    vecStyle = inVectorStyle;
    viewC = inViewC;
    coordSys = [[MaplySphericalMercator alloc] initWebStandard];

    offlineRender.clearColor = [UIColor blueColor];
    imageTileParser = [[MapboxVectorTileParser alloc] initWithStyle:imageStyle viewC:offlineRender];
    imageTileParser.localCoords = true;
    vecTileParser = [[MapboxVectorTileParser alloc] initWithStyle:vecStyle viewC:viewC];

    MapboxVectorLayerBackground *backLayer = imageStyle.layersByName[@"background"];
    backColor = backLayer.paint.color;
    
    return self;
}

- (int)minZoom
{
    return tileInfo.minZoom;
}

- (int)maxZoom
{
    return tileInfo.maxZoom;
}

- (int)tileSize
{
    return offlineRender.getFramebufferSize.width;
}

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame
{
    return false;
}

- (nonnull MaplyCoordinateSystem *)coordSys
{
    return coordSys;
}

- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox)bbox
{
    return true;
}

- (int)tileSizeForTile:(MaplyTileID)tileID
{
    return [self tileSize];
}

- (void)setCacheDir:(NSString *)cacheDir
{
    _cacheDir = cacheDir;
    
    if (_cacheDir) {
        NSError *error = nil;
        [[NSFileManager defaultManager] createDirectoryAtPath:_cacheDir withIntermediateDirectories:YES attributes:nil error:&error];
    }
}

- (void)startFetchLayer:(MaplyQuadImageLoader *)loader tile:(MaplyTileID)tileID frame:(int)frame
{
    NSURLRequest *urlReq = [tileInfo requestForTile:tileID];
    if (!urlReq)
        return;
    
    NSData *tileData = nil;
    NSString *cacheFileName;
    if (_cacheDir) {
        cacheFileName = [NSString stringWithFormat:@"%@/%d_%d_%d",_cacheDir,tileID.level,tileID.x,tileID.y];
        tileData = [[NSData alloc] initWithContentsOfFile:cacheFileName];
        if([tileData isCompressed]) {
            tileData = [tileData uncompressGZip];
            if(!tileData.length) {
                tileData = nil;
            }
        }
    }

    // Process from the cached data, but fall back on fetching it
    if (tileData) {
        if (![self processData:tileData tile:tileID loader:loader])
            tileData = nil;
    }
    
    if (!tileData) {
        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionDataTask *task = [session dataTaskWithRequest:urlReq completionHandler:
                                      ^(NSData * _Nullable netData, NSURLResponse * _Nullable response, NSError * _Nullable error)
                                      {
                                          dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                                              NSData *thisTileData = nil;
                                              if(netData.length) {
                                                  if([netData isCompressed]) {
                                                      thisTileData = [netData uncompressGZip];
                                                      if(!thisTileData.length) {
                                                          NSLog(@"Error: tile data was nil after decompression");
                                                          return;
                                                      }
                                                  }
                                              }
                                              
                                              if ([self processData:thisTileData tile:tileID loader:loader] && cacheFileName)
                                                  [netData writeToFile:cacheFileName atomically:NO];
                                          });
                                      }];
        [task resume];
        [loader registerTile:tileID frame:frame data:task];
    }
}

- (bool)processData:(NSData *)tileData tile:(MaplyTileID)tileID loader:(MaplyQuadImageLoader *)loader
{
    MaplyBoundingBox imageBBox;
    imageBBox.ll = MaplyCoordinateMake(0,0);  imageBBox.ur = MaplyCoordinateMake(offlineRender.getFramebufferSize.width,offlineRender.getFramebufferSize.height);
    MaplyBoundingBox localBBox,geoBBox;
    localBBox = [loader boundsForTile:tileID];
    geoBBox = [loader geoBoundsForTile:tileID];
    MaplyBoundingBox spherMercBBox;
    spherMercBBox.ll = [self toMerc:geoBBox.ll];
    spherMercBBox.ur = [self toMerc:geoBBox.ur];

    UIImage *image = nil;

    // Parse the polygons and draw into an image
    @synchronized(offlineRender)
    {
        // Build the vector objects for use in the image tile
        MaplyVectorTileData *retData = nil;
        offlineRender.clearColor = backColor;
        if ((retData = [imageTileParser buildObjects:tileData tile:tileID bounds:imageBBox geoBounds:imageBBox]))
        {
            // Turn all those objects on
            [offlineRender enableObjects:retData.compObjs mode:MaplyThreadCurrent];
            
            image = [offlineRender renderToImage];
            
            // And then remove them all
            [offlineRender removeObjects:retData.compObjs mode:MaplyThreadCurrent];
        } else {
            NSLog(@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
            return false;
        }
    }
    
    MaplyQuadImageLoaderReturn *loadReturn = [[MaplyQuadImageLoaderReturn alloc] init];

    // Parse everything else and turn into vectors
    MaplyVectorTileData *retData = nil;
    if ((retData = [vecTileParser buildObjects:tileData tile:tileID bounds:spherMercBBox geoBounds:geoBBox]))
    {
        // Success
    } else {
        NSLog(@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
        loadReturn.error = [[NSError alloc] initWithDomain:@"MapboxVectorTilesImageDelegate" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to parse tile"}];
        [loader loadedReturn:loadReturn];
        return false;
    }
    
    // Successful load
    loadReturn.tileID = tileID;
    loadReturn.frame = -1;
    loadReturn.image = image;
    loadReturn.compObjs = retData.compObjs;
    [loader loadedReturn:loadReturn];
    
    return true;
}

- (void)cancelTile:(MaplyTileID)tileID frame:(int)frame tileData:(id)tileData
{
    NSURLSessionDataTask *task = tileData;
    if ([task isKindOfClass:[NSURLSessionDataTask class]]) {
        [task cancel];
    }
}

- (void)clear:(MaplyQuadImageLoader *)loader tileData:(NSArray *)tileDatas
{
    for (NSURLSessionDataTask *task in tileDatas) {
        if ([task isKindOfClass:[NSURLSessionDataTask class]])
            [task cancel];
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
