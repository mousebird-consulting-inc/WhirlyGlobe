/*
 *  MapboxVectorTilesImageDelegate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on January 24 2018
 *  Copyright 2011-2019 Saildrone
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

#import "MapboxVectorInterpreter.h"
#import "MapboxVectorTiles.h"
#import "MaplyTileSourceNew.h"
#import "MapboxVectorStyleSet.h"
#import "MapboxVectorStyleBackground.h"
#import "MaplyQuadImageFrameLoader.h"
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


@implementation MapboxVectorInterpreter
{
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    MapboxVectorStyleSet *imageStyle;
    NSObject<MaplyVectorStyleDelegate> *vecStyle;
    MaplySphericalMercator *coordSys;
    MaplyRenderController *offlineRender;
    UIColor *backColor;
    
    MapboxVectorTileParser *imageTileParser,*vecTileParser;
}

- (instancetype) initWithImageStyle:(MapboxVectorStyleSet *)inImageStyle
                      offlineRender:(MaplyRenderController *)inOfflineRender
                        vectorStyle:(NSObject<MaplyVectorStyleDelegate> *)inVectorStyle
                              viewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
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

- (instancetype) initWithVectorStyle:(NSObject<MaplyVectorStyleDelegate> *)inVectorStyle
                               viewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    vecStyle = inVectorStyle;
    viewC = inViewC;
    
    vecTileParser = [[MapboxVectorTileParser alloc] initWithStyle:vecStyle viewC:viewC];
    
    return self;
}

- (void)setLoader:(MaplyQuadLoaderBase *)inLoader
{
    if ([inLoader isKindOfClass:[MaplyQuadImageLoaderBase class]]) {
        MaplyQuadImageLoaderBase *loader = (MaplyQuadImageLoaderBase *)inLoader;

        if ([vecStyle isKindOfClass:[MapboxVectorStyleSet class]]) {
            MapboxVectorStyleSet *mapboxVecStyle = (MapboxVectorStyleSet *)vecStyle;

            loader.baseDrawPriority = mapboxVecStyle.tileStyleSettings.baseDrawPriority;
            loader.drawPriorityPerLevel = mapboxVecStyle.tileStyleSettings.drawPriorityPerLevel;
        }
    }
}

// Flip data in an NSData object that we know to be an image
- (NSData *)flipVertically:(NSData *)data width:(int)width height:(int)height
{
    NSMutableData *retData = [[NSMutableData alloc] initWithData:data];

    int rowSize = 4*width;
    unsigned char tmpData[rowSize];
    unsigned char *rawData = (unsigned char *)[retData mutableBytes];
    for (unsigned int iy=0;iy<height/2;iy++) {
        unsigned char *rowA = &rawData[iy*rowSize];
        unsigned char *rowB = &rawData[(height-iy-1)*rowSize];
        memcpy(tmpData, rowA, rowSize);
        memcpy(rowA, rowB, rowSize);
        memcpy(rowB, tmpData, rowSize);
    }
    
    return retData;
}

- (void)dataForTile:(MaplyImageLoaderReturn *)loadReturn loader:(MaplyQuadLoaderBase *)loader
{
    MaplyTileID tileID = loadReturn.tileID;
    std::vector<NSData *> pbfDatas;
    std::vector<UIImage *> images;
    
    // Uncompress any of the data we recieved
    NSArray *tileData = [loadReturn getTileData];
    for (unsigned int ii=0;ii<[tileData count];ii++) {
        NSData *thisTileData = [tileData objectAtIndex:ii];
        if(thisTileData) {
          if([thisTileData isCompressed]) {
              thisTileData = [thisTileData uncompressGZip];
              if(!thisTileData.length) {
                  continue;
              }
          }
        }
        // Might be an image
        UIImage *image = [UIImage imageWithData:thisTileData];
        if (image)
            images.push_back(image);
        else
            pbfDatas.push_back(thisTileData);
    }
    
    if (pbfDatas.empty() && images.empty()) {
        loadReturn.error = [[NSError alloc] initWithDomain:@"MapboxVectorTilesImageDelegate" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Tile data was nil after decompression"}];
        return;
    }
    
    // Coordinates for the coming data
    MaplyBoundingBox imageBBox;
    imageBBox.ll = MaplyCoordinateMake(0,0);  imageBBox.ur = MaplyCoordinateMake(offlineRender.getFramebufferSize.width,offlineRender.getFramebufferSize.height);
    MaplyBoundingBox localBBox,geoBBox;
    localBBox = [loader boundsForTile:tileID];
    geoBBox = [loader geoBoundsForTile:tileID];
    MaplyBoundingBox spherMercBBox;
    spherMercBBox.ll = [self toMerc:geoBBox.ll];
    spherMercBBox.ur = [self toMerc:geoBBox.ur];
    
    NSData *imageData = nil;
    
    [viewC startChanges];
    
    if (offlineRender) {
        // Parse the polygons and draw into an image
        // Note: Can we use multiple of these for speed?
        @synchronized(offlineRender)
        {
            // Build the vector objects for use in the image tile
            NSMutableArray *compObjs = [NSMutableArray array];
            offlineRender.clearColor = backColor;

            for (NSData *thisTileData : pbfDatas) {
                MaplyVectorTileData *retData = [imageTileParser buildObjects:thisTileData tile:tileID bounds:imageBBox geoBounds:geoBBox];
                if (retData) {
                    [compObjs addObjectsFromArray:retData.compObjs];
                } else {
                    NSString *errMsg = [NSString stringWithFormat:@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y];
                    loadReturn.error = [[NSError alloc] initWithDomain:@"MapboxVectorTilesImageDelegate" code:0 userInfo:@{NSLocalizedDescriptionKey: errMsg}];
                }
            }
            
            if (!loadReturn.error) {
                // Turn all those objects on
                [offlineRender enableObjects:compObjs mode:MaplyThreadCurrent];
                
                imageData = [self flipVertically:[offlineRender renderToImageData]
                                           width:offlineRender.getFramebufferSize.width
                                          height:offlineRender.getFramebufferSize.height];
                
                // And then remove them all
                [offlineRender removeObjects:compObjs mode:MaplyThreadCurrent];
            }
        }
    }
    
    // Parse everything else and turn into vectors
    NSMutableArray *compObjs = [NSMutableArray array];
    NSMutableArray *ovlCompObjs = [NSMutableArray array];
    for (NSData *thisTileData : pbfDatas) {
        MaplyVectorTileData *retData = [vecTileParser buildObjects:thisTileData tile:tileID bounds:spherMercBBox geoBounds:geoBBox];
        if (retData) {
            [compObjs addObjectsFromArray:retData.compObjs];
            NSArray *ovl = [retData.categories objectForKey:@"overlay"];
            if (ovl)
                [ovlCompObjs addObjectsFromArray:ovl];
        } else {
            NSLog(@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
            loadReturn.error = [[NSError alloc] initWithDomain:@"MapboxVectorTilesImageDelegate" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to parse tile"}];
        }
    }

    [viewC endChanges];

    if (imageData) {
        // Rendered image goes in first
        MaplyImageTile *tileImage = [[MaplyImageTile alloc] initWithRawImage:imageData width:offlineRender.getFramebufferSize.width height:offlineRender.getFramebufferSize.height ];
        [loadReturn addImageTile:tileImage];
    }
    
    // Any additional images are tacked on
    for (UIImage *image : images) {
        MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithImage:image];
        [loadReturn addImageTile:tileData];
    }

    if ([ovlCompObjs count] > 0) {
        [loadReturn addOvlCompObjs:ovlCompObjs];
        [compObjs removeObjectsInArray:ovlCompObjs];
        [loadReturn addCompObjs:compObjs];
    } else
        [loadReturn addCompObjs:compObjs];
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
