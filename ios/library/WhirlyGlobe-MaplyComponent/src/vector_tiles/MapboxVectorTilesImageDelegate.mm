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

static bool debugMode = false;

// Objects sorted by Tile ID
class ObjectsByTile
{
public:
    ObjectsByTile() : compObjs(NULL) { }
    ObjectsByTile(MaplyTileID tileID) : tileID(tileID), compObjs(NULL), enabled(true) { }
    
    bool operator < (const ObjectsByTile &that) const
    {
        if (tileID.level == that.tileID.level) {
            if (tileID.x == that.tileID.x) {
                return tileID.y < that.tileID.y;
            }
            return tileID.x < that.tileID.x;
        }
        return tileID.level < that.tileID.level;
    }
    
    void enable(NSObject<MaplyRenderControllerProtocol> *viewC)
    {
        enabled = true;
        if (compObjs) {
            [viewC enableObjects:compObjs mode:MaplyThreadCurrent];
        }
    }
    
    void disable(NSObject<MaplyRenderControllerProtocol> *viewC)
    {
        enabled = false;
        if (compObjs) {
            [viewC disableObjects:compObjs mode:MaplyThreadCurrent];
        }
    }
    
    MaplyTileID tileID;
    bool enabled;
    NSArray *compObjs;
};

typedef std::shared_ptr<ObjectsByTile> ObjectsByTileRef;

typedef struct
{
    bool operator () (const ObjectsByTileRef a,const ObjectsByTileRef b) const { return *a < *b; }
} ObjectsByTileRefSorter;


@implementation MapboxVectorTileImageSource
{
    MaplyRemoteTileInfo *tileInfo;
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    MapboxVectorStyleSet *imageStyle,*vecStyle;
    MaplySphericalMercator *coordSys;
    MaplyRenderController *offlineRender;
    UIColor *backColor;
    
    MapboxVectorTileParser *imageTileParser,*vecTileParser;
    
    std::set<ObjectsByTileRef,ObjectsByTileRefSorter> tiles;
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

    offlineRender = [[MaplyRenderController alloc] initWithSize:CGSizeMake(512.0,512.0)];
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
    return 512;
}

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame
{
    return false;
}

- (nonnull MaplyCoordinateSystem *)coordSys
{
    return coordSys;
}

- (void)clear
{
    NSMutableArray *compObjs = [NSMutableArray array];
    @synchronized(self)
    {
        for (auto tile : tiles) {
            if (tile->compObjs != nil)
                [compObjs addObjectsFromArray:tile->compObjs];
        }
    }
    [viewC removeObjects:compObjs mode:MaplyThreadAny];
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

- (void)startFetchLayer:(MaplyQuadImageTilesLayer *)layer tile:(MaplyTileID)tileID
{
    NSURLRequest *urlReq = [tileInfo requestForTile:tileID];
    if (!urlReq)
        return;
    
    // Add an entry for this tile
    @synchronized(self)
    {
        if (debugMode)
            NSLog(@"Started adding tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
        ObjectsByTileRef newTile(new ObjectsByTile(tileID));
        auto it = tiles.find(newTile);
        if (it == tiles.end()) {
            tiles.insert(newTile);
        } else {
            if (debugMode)
                NSLog(@"Tried to add a tile that's already there");
        }
    }

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
        if (![self processData:tileData tile:tileID layer:layer])
            tileData = nil;
    }
    
    if (!tileData) {
        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionDataTask *task = [session dataTaskWithRequest:urlReq completionHandler:
                                      ^(NSData * _Nullable netData, NSURLResponse * _Nullable response, NSError * _Nullable error)
                                      {
                                          NSData *thisTileData = nil;
                                          if(netData.length) {
                                              if([netData isCompressed]) {
                                                  thisTileData = [netData uncompressGZip];
                                                  if(!thisTileData.length) {
                                                      NSLog(@"Error: tile data was nil after decompression");
                                                      return;
                                                  }
                                              } else {
                                                  thisTileData = netData;
                                              }
                                          }
                                          
                                          if ([self processData:thisTileData tile:tileID layer:layer] && cacheFileName)
                                              [netData writeToFile:cacheFileName atomically:NO];
                                      }];
        [task resume];
    }
}

- (bool)processData:(NSData *)tileData tile:(MaplyTileID)tileID layer:(MaplyQuadImageTilesLayer *)layer
{
    MaplyBoundingBox imageBBox;
    imageBBox.ll = MaplyCoordinateMake(0,0);  imageBBox.ur = MaplyCoordinateMake(512, 512);
    MaplyBoundingBox localBBox,geoBBox;
    localBBox = [layer boundsForTile:tileID];
    geoBBox = [layer geoBoundsForTile:tileID];
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
    
    // Parse everything else and turn into vectors
    MaplyVectorTileData *retData = nil;
    if ((retData = [vecTileParser buildObjects:tileData tile:tileID bounds:spherMercBBox geoBounds:geoBBox]))
    {
        // Success
    } else {
        NSLog(@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
        return false;
    }
    
    @synchronized(self)
    {
        // Make sure we still want the tile
        ObjectsByTileRef testTile(new ObjectsByTile(tileID));
        auto it = tiles.find(testTile);
        if (it == tiles.end()) {
            // Uh oh.  Got deleted while we were loading.  Nuke everything.
            [viewC removeObjects:retData.compObjs mode:MaplyThreadCurrent];
            if (debugMode)
                NSLog(@"In-transit delete for tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
        } else {
            (*it)->compObjs = retData.compObjs;
            if ((*it)->enabled)
                (*it)->enable(viewC);
            else
                (*it)->disable(viewC);
            if (debugMode)
                NSLog(@"Finished adding tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
        }
    }
    
    [layer loadedImages:image forTile:tileID frame:-1];
    return true;
}

- (void)tileWasDisabled:(MaplyTileID)tileID
{
    @synchronized(self)
    {
        ObjectsByTileRef dummyTile(new ObjectsByTile(tileID));
        auto it = tiles.find(dummyTile);
        if (it != tiles.end()) {
            if ((*it)->enabled)
                (*it)->disable(viewC);
        } else {
            if (debugMode)
                NSLog(@"Tried to disable tile that isn't there");
        }
    }
    
    if (debugMode)
        NSLog(@"Disabling tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
}

- (void)tileWasEnabled:(MaplyTileID)tileID
{
    @synchronized(self)
    {
        ObjectsByTileRef dummyTile(new ObjectsByTile(tileID));
        auto it = tiles.find(dummyTile);
        if (it != tiles.end()) {
            if (!(*it)->enabled)
                (*it)->enable(viewC);
        } else {
            if (debugMode)
                NSLog(@"Tried to enable tile that isn't there");
        }
    }

    if (debugMode)
        NSLog(@"Enabling tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
}

- (void)tileUnloaded:(MaplyTileID)tileID
{
    @synchronized(self)
    {
        ObjectsByTileRef dummyTile(new ObjectsByTile(tileID));
        auto it = tiles.find(dummyTile);
        if (it != tiles.end()) {
            if ((*it)->compObjs)
                [viewC removeObjects:(*it)->compObjs mode:MaplyThreadCurrent];
            tiles.erase(it);
        } else {
            if (debugMode)
                NSLog(@"Tried to unload tile that isn't there");
        }
    }
    
    if (debugMode)
        NSLog(@"Unloading tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
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
