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

static int BackImageWidth = 16, BackImageHeight = 16;

@implementation MapboxVectorImageInterpreter
{
    MaplyQuadImageLoader * __weak loader;
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    MapboxVectorStyleSet *imageStyle,*vecStyle;
    MaplySphericalMercator *coordSys;
    MaplyRenderController *offlineRender;
    MapboxTransColor *backColor;
    
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

- (instancetype _Nullable ) initWithLoader:(MaplyQuadImageLoader *__nonnull)inLoader
                                     style:(MapboxVectorStyleSet *__nonnull)inVectorStyle
                                     viewC:(MaplyBaseViewController *__nonnull)inViewC
{
    self = [super init];
    loader = inLoader;
    vecStyle = inVectorStyle;

    loader.baseDrawPriority = vecStyle.tileStyleSettings.baseDrawPriority;
    loader.drawPriorityPerLevel = vecStyle.tileStyleSettings.drawPriorityPerLevel;
    viewC = inViewC;
    coordSys = [[MaplySphericalMercator alloc] initWebStandard];

    vecTileParser = [[MapboxVectorTileParser alloc] initWithStyle:vecStyle viewC:viewC];
    MapboxVectorLayerBackground *backLayer = vecStyle.layersByName[@"background"];
    backColor = backLayer.paint.color;
        
    return self;
}

// Flip data in an NSData object that we know to be an image
- (NSData *)flipVertically:(NSData *)data width:(int)width height:(int)height
{
    if (!data)
        return nil;
    
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

- (void)parseData:(MaplyLoaderReturn * __nonnull)loadReturn
{
    MaplyTileID tileID = loadReturn.tileID;
    std::vector<NSData *> pbfDatas;
    std::vector<UIImage *> images;
    
    // Uncompress any of the data we recieved
    for (unsigned int ii=0;ii<[loadReturn.multiTileData count];ii++) {
        NSData *thisTileData = [loadReturn.multiTileData objectAtIndex:ii];
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
            offlineRender.clearColor = [backColor colorForZoom:tileID.level];

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

    // Rendered image goes in first
    NSMutableArray *outImages = [NSMutableArray array];
    if (offlineRender) {
        MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRawImage:imageData width:offlineRender.getFramebufferSize.width height:offlineRender.getFramebufferSize.height];
        WhirlyKitLoadedTile *loadTile = [tileData wkTile:0 convertToRaw:true];
        if (loadTile)
            [outImages addObject:loadTile];
    } else {
        if (images.empty()) {
            // Make a single color background image
            // We have to do this each time because it can change per level
            // TODO: Cache this per level or something
            NSData *backImageData = [[NSMutableData alloc] initWithLength:4*BackImageWidth*BackImageHeight];
            unsigned int *data = (unsigned int *)[backImageData bytes];
            CGFloat red,green,blue,alpha;
            UIColor *thisBackColor = [backColor colorForZoom:tileID.level];
            [thisBackColor getRed:&red green:&green blue:&blue alpha:&alpha];
            unsigned int pixel = 0xff << 24 | (int)(blue * 255) << 16 | (int)(green * 255) << 8 | (int)(red * 255);
            for (unsigned int pix=0;pix<BackImageWidth*BackImageHeight;pix++) {
                *data = pixel;
                data++;
            }

            MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRawImage:backImageData width:BackImageWidth height:BackImageHeight];
            WhirlyKitLoadedTile *loadTile = [tileData wkTile:0 convertToRaw:true];
            [outImages addObject:loadTile];
        }
    }
    
    // Any additional images are tacked on
    for (UIImage *image : images) {
        MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithImage:image];
        WhirlyKitLoadedTile *loadTile = [tileData wkTile:0 convertToRaw:true];
        if (loadTile)
            [outImages addObject:loadTile];
    }
    loadReturn.images = outImages;

    if ([ovlCompObjs count] > 0) {
        loadReturn.ovlCompObjs = ovlCompObjs;
        [compObjs removeObjectsInArray:ovlCompObjs];
        loadReturn.compObjs = compObjs;
    } else
        loadReturn.compObjs = compObjs;
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
