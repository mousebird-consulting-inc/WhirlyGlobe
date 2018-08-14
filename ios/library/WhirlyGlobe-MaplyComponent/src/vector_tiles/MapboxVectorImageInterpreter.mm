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

#import "MapboxVectorImageInterpreter.h"
#import "MapboxVectorTiles.h"
#import "MaplyTileSource.h"
#import "MapboxVectorStyleSet.h"
#import "MapboxVectorStyleBackground.h"
#import "MaplyQuadImageLoader.h"
#import "MaplyImageTile_private.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <set>

#import "CoordSystem.h"
#import "MaplyVectorStyle.h"
#import "MaplyVectorObject_private.h"
#import "MaplyScreenLabel.h"
#import "NSData+Zlib.h"
#import "vector_tile.pb.h"
#import "VectorData.h"
#import "MapnikStyleSet.h"
#import "MaplyRenderController_private.h"

using namespace WhirlyKit;

static double MAX_EXTENT = 20037508.342789244;

@implementation MapboxVectorImageInterpreter
{
    MaplyQuadImageLoader * __weak loader;
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    MapboxVectorStyleSet *imageStyle,*vecStyle;
    MaplySphericalMercator *coordSys;
    MaplyRenderController *offlineRender;
    UIColor *backColor;
    
    MapboxVectorTileParser *imageTileParser,*vecTileParser;
}

- (instancetype _Nullable ) initWithLoader:(MaplyQuadImageLoader *)inLoader
                                imageStyle:(MapboxVectorStyleSet *__nonnull)inImageStyle
                               offlineRender:(MaplyRenderController *__nonnull)inOfflineRender
                                 vectorStyle:(MapboxVectorStyleSet *__nonnull)inVectorStyle
                                       viewC:(MaplyBaseViewController *__nonnull)inViewC
{
    if (inLoader.importanceScale != 1.0) {
        NSLog(@"MapboxVectorImageInterpreter works poorly with an importance scale.  Failing.");
        return nil;
    }

    self = [super init];
    loader = inLoader;
    imageStyle = inImageStyle;
    offlineRender = inOfflineRender;
    vecStyle = inVectorStyle;
    loader.baseDrawPriority = vecStyle.tileStyleSettings.baseDrawPriority;
    loader.drawPriorityPerLevel = vecStyle.tileStyleSettings.drawPriorityPerLevel;
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

- (void)parseData:(MaplyLoaderReturn * __nonnull)loadReturn
{
    MaplyTileID tileID = loadReturn.tileID;
    
    NSData *thisTileData = nil;
    if(loadReturn.tileData) {
      if([loadReturn.tileData isCompressed]) {
          thisTileData = [loadReturn.tileData uncompressGZip];
          if(!thisTileData.length) {
              loadReturn.error = [[NSError alloc] initWithDomain:@"MapboxVectorTilesImageDelegate" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Tile data was nil after decompression"}];
              return;
          }
      }
    }
    
    MaplyBoundingBox imageBBox;
    imageBBox.ll = MaplyCoordinateMake(0,0);  imageBBox.ur = MaplyCoordinateMake(offlineRender.getFramebufferSize.width,offlineRender.getFramebufferSize.height);
    MaplyBoundingBox localBBox,geoBBox;
    localBBox = [loader boundsForTile:tileID];
    geoBBox = [loader geoBoundsForTile:tileID];
    MaplyBoundingBox spherMercBBox;
    spherMercBBox.ll = [self toMerc:geoBBox.ll];
    spherMercBBox.ur = [self toMerc:geoBBox.ur];
    
    UIImage *image = nil;
    
    [viewC startChanges];
    
    // Parse the polygons and draw into an image
    // Note: Can we use multiple of these?
    @synchronized(offlineRender)
    {
        // Build the vector objects for use in the image tile
        MaplyVectorTileData *retData = nil;
        offlineRender.clearColor = backColor;
        if ((retData = [imageTileParser buildObjects:thisTileData tile:tileID bounds:imageBBox geoBounds:imageBBox]))
        {
            // Turn all those objects on
            [offlineRender enableObjects:retData.compObjs mode:MaplyThreadCurrent];
            
            image = [offlineRender renderToImage];
            
            // And then remove them all
            [offlineRender removeObjects:retData.compObjs mode:MaplyThreadCurrent];
        } else {
            NSString *errMsg = [NSString stringWithFormat:@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y];
            loadReturn.error = [[NSError alloc] initWithDomain:@"MapboxVectorTilesImageDelegate" code:0 userInfo:@{NSLocalizedDescriptionKey: errMsg}];
        }
    }
    
    // Parse everything else and turn into vectors
    MaplyVectorTileData *retData = nil;
    if ((retData = [vecTileParser buildObjects:thisTileData tile:tileID bounds:spherMercBBox geoBounds:geoBBox]))
    {
        // Success!
        [viewC endChanges];
    } else {
        [viewC endChanges];
        
        NSLog(@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
        loadReturn.error = [[NSError alloc] initWithDomain:@"MapboxVectorTilesImageDelegate" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to parse tile"}];
        return;
    }
    
    // Successful load
    MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:image];
    WhirlyKitLoadedTile *loadTile = [tileData wkTile:0 convertToRaw:true];
    loadReturn.image = loadTile;
    loadReturn.compObjs = retData.compObjs;
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
