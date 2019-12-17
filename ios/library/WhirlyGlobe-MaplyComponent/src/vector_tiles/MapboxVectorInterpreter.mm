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

#import "vector_tiles/MapboxVectorInterpreter.h"
#import "vector_tiles/MapboxVectorTiles.h"
#import "loading/MaplyTileSourceNew.h"
#import "vector_styles/MapboxVectorStyleSet.h"
#import "vector_styles/MapboxVectorStyleBackground.h"
#import "loading/MaplyQuadImageFrameLoader.h"
#import "MaplyImageTile_private.h"
#import "MapboxVectorTiles_private.h"
#import "MaplyQuadLoader_private.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <set>
#import <Accelerate/Accelerate.h>

#import "CoordSystem.h"
#import "vector_styles/MaplyVectorStyle.h"
#import "MaplyVectorObject_private.h"
#import "visual_objects/MaplyScreenLabel.h"
#import "UIKit/NSData+Zlib.h"

#import "vector_tile.pb.h"
#import "VectorData.h"
#import "vector_styles/MapnikStyleSet.h"
#import "MaplyRenderController_private.h"

using namespace WhirlyKit;

static double MAX_EXTENT = 20037508.342789244;

static int BackImageWidth = 16, BackImageHeight = 16;

@implementation MapboxVectorInterpreter
{
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    MapboxVectorStyleSet *imageStyle;
    NSObject<MaplyVectorStyleDelegate> *vecStyle;
    MaplySphericalMercator *coordSys;
    MaplyRenderController *offlineRender;
    MapboxTransColor *backColor;

    MapboxVectorTileParser_iOSRef imageTileParser,vecTileParser;
}

- (instancetype) initWithImageStyle:(MapboxVectorStyleSet *)inImageStyle
                      offlineRender:(MaplyRenderController *)inOfflineRender
                        vectorStyle:(NSObject<MaplyVectorStyleDelegate> *)inVectorStyle
                              viewC:(NSObject<MaplyRenderControllerProtocol> *)inViewC
{
    self = [super init];
    imageStyle = inImageStyle;
    offlineRender = inOfflineRender;
    vecStyle = inVectorStyle;
    viewC = inViewC;
    coordSys = [[MaplySphericalMercator alloc] initWebStandard];

    offlineRender.clearColor = [UIColor blueColor];
    imageTileParser = MapboxVectorTileParser_iOSRef(new MapboxVectorTileParser_iOS(imageStyle,offlineRender));
    imageTileParser->localCoords = true;
    vecTileParser = MapboxVectorTileParser_iOSRef(new MapboxVectorTileParser_iOS(vecStyle,viewC));

    MapboxVectorLayerBackground *backLayer = imageStyle.layersByName[@"background"];
    backColor = backLayer.paint.color;
    
    return self;
}

- (instancetype) initWithVectorStyle:(NSObject<MaplyVectorStyleDelegate> *)inVectorStyle
                               viewC:(NSObject<MaplyRenderControllerProtocol> *)inViewC
{
    self = [super init];
    vecStyle = inVectorStyle;
    viewC = inViewC;
    
    vecTileParser = MapboxVectorTileParser_iOSRef(new MapboxVectorTileParser_iOS(vecStyle,viewC));
    
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
    if (!data)
        return nil;
    
    NSMutableData *retData = [[NSMutableData alloc] initWithBytes:[data bytes] length:[data length]];

    unsigned int tmpData[width];
    unsigned int *rawData = (unsigned int *)[retData mutableBytes];
    for (unsigned int iy=0;iy<height/2;iy++) {
        unsigned int *rowA = &rawData[iy*width];
        unsigned int *rowB = &rawData[(height-iy-1)*width];
        memcpy(tmpData, rowA, 4*width);
        memcpy(rowA, rowB, 4*width);
        memcpy(rowB, tmpData, 4*width);
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
    MaplyBoundingBoxD imageBBox;
    imageBBox.ll = MaplyCoordinateDMake(0,0);  imageBBox.ur = MaplyCoordinateDMake(offlineRender.getFramebufferSize.width,offlineRender.getFramebufferSize.height);
    MaplyBoundingBoxD localBBox,geoBBox;
    localBBox = [loader boundsForTileD:tileID];
    geoBBox = [loader geoBoundsForTileD:tileID];
    MaplyBoundingBoxD spherMercBBox;
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
            offlineRender.clearColor = [backColor colorForZoom:tileID.level];
            MaplyVectorTileData *vecTileReturn;

            for (NSData *thisTileData : pbfDatas) {
                RawNSDataReader thisTileDataWrap(thisTileData);
                vecTileReturn = [[MaplyVectorTileData alloc] initWithID:tileID bbox:imageBBox geoBBox:geoBBox];
                imageTileParser->parse(&thisTileDataWrap, vecTileReturn->data.get());
                
//                if (vecTileReturn) {
//                } else {
//                    NSString *errMsg = [NSString stringWithFormat:@"Failed to parse tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y];
//                    loadReturn.error = [[NSError alloc] initWithDomain:@"MapboxVectorTilesImageDelegate" code:0 userInfo:@{NSLocalizedDescriptionKey: errMsg}];
//                }
            }
            
            NSArray *compObjs = [vecTileReturn componentObjects];
            if (!loadReturn.error && compObjs) {
                
                // Turn all those objects on
                [offlineRender enableObjects:compObjs mode:MaplyThreadCurrent];
                
                NSData *renderData = [offlineRender renderToImageData];
                // Flip for OpenGL, but not for Metal
                if (offlineRender->renderType == SceneRenderer::RenderGLES) {
                    imageData = [self flipVertically:renderData
                                           width:offlineRender.getFramebufferSize.width
                                          height:offlineRender.getFramebufferSize.height];
                } else {
                    // For Metal, we have to swizzle the channels
                    vImage_Buffer srcBuff;
                    srcBuff.width = offlineRender.getFramebufferSize.width;
                    srcBuff.height = offlineRender.getFramebufferSize.height;
                    srcBuff.rowBytes = srcBuff.width * 4;
                    srcBuff.data = (void *)[renderData bytes];
                    
                    NSMutableData *newImageData = [NSMutableData dataWithLength:[renderData length]];
                    vImage_Buffer destBuff;
                    destBuff.width = srcBuff.width;
                    destBuff.height = srcBuff.height;
                    destBuff.rowBytes = srcBuff.rowBytes;
                    destBuff.data = [newImageData mutableBytes];
                    const uint8_t map[4] = { 2, 1, 0, 3 };
                    vImagePermuteChannels_ARGB8888(&srcBuff, &destBuff, map, kvImageNoFlags);
                    
                    imageData = newImageData;
                }
                
                // And then remove them all
                [offlineRender removeObjects:compObjs mode:MaplyThreadCurrent];
            }
        }
    }
    
    // Parse everything else and turn into vectors
    std::vector<ComponentObjectRef> compObjs,ovlCompObjs;
    for (NSData *thisTileData : pbfDatas) {
        RawNSDataReader thisTileDataWrap(thisTileData);
        MaplyVectorTileData *vecTileReturn = [[MaplyVectorTileData alloc] initWithID:tileID bbox:spherMercBBox geoBBox:geoBBox];
        vecTileParser->parse(&thisTileDataWrap,vecTileReturn->data.get());
        
        if (!vecTileReturn->data->compObjs.empty())
            compObjs.insert(compObjs.end(),vecTileReturn->data->compObjs.begin(),vecTileReturn->data->compObjs.end());
        
        auto it = vecTileReturn->data->categories.find("overlay");
        if (it != vecTileReturn->data->categories.end()) {
            auto ids = it->second;
            ovlCompObjs.insert(ovlCompObjs.end(),ids.begin(),ids.end());
        }
    }

    [viewC endChanges];

    if ([loadReturn isKindOfClass:[MaplyImageLoaderReturn class]]) {
        if (offlineRender) {
            // Rendered image goes in first
            MaplyImageTile *tileImage = [[MaplyImageTile alloc] initWithRawImage:imageData width:offlineRender.getFramebufferSize.width height:offlineRender.getFramebufferSize.height viewC:viewC];
            [loadReturn addImageTile:tileImage];
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

                MaplyImageTile *tileImage = [[MaplyImageTile alloc] initWithRawImage:backImageData width:BackImageWidth height:BackImageHeight viewC:viewC];
                [loadReturn addImageTile:tileImage];
            }
        }

        // Any additional images are tacked on
        for (UIImage *image : images) {
            MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithImage:image viewC:viewC];
            [loadReturn addImageTile:tileData];
        }
    }
        
    if (!ovlCompObjs.empty()) {
        std::vector<ComponentObjectRef> minusOvls;
        
        // Need a list of component objects without overlays
        for (auto compObj : compObjs) {
            // Look for it in overlays
            bool found = false;
            for (auto ovlCompObj : ovlCompObjs)
                if (ovlCompObj->getId() == compObj->getId()) {
                    found = true;
                    break;
                }
            if (!found) {
                minusOvls.push_back(compObj);
            }
        }
        
        loadReturn->loadReturn->compObjs = minusOvls;
        loadReturn->loadReturn->ovlCompObjs = ovlCompObjs;
    } else {
        loadReturn->loadReturn->compObjs = compObjs;
    }
}

- (void)tileUnloaded:(MaplyTileID)tileID {
}


/**
 Convert a coordinate from lat/lon radians to epsg:3785
 Verified output with "cs2cs +init=epsg:4326 +to +init=epsg:3785", correct within .5 meters,
 but frequently off by .4
 */
- (MaplyCoordinateD)toMerc:(MaplyCoordinateD)coord {
    //  MaplyCoordinate orig = coord;
    coord.x = RadToDeg(coord.x) * MAX_EXTENT / 180;
    coord.y = 3189068.5 * log((1.0 + sin(coord.y)) / (1.0 - sin(coord.y)));
    //  NSLog(@"%f %f -> %.2f %.2f", RadToDeg(orig.x), RadToDeg(orig.y), coord.x, coord.y);
    return coord;
}

@end
