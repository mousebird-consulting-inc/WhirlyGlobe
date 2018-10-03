/*
 *  MaplyMapnikVectorTiles.h
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


#import <Foundation/Foundation.h>
#import "MaplyQuadPagingLayer.h"
#import "MaplyTileSource.h"
#import "MaplyCoordinate.h"
#import "MaplyVectorStyle.h"

/** 
    Geometry type for data found within PBF files.
    
    These are the geometry types supported within Mapnik PBF files.
  */
typedef NS_ENUM(NSInteger, MapnikGeometryType) {
  GeomTypeUnknown = 0,
  GeomTypePoint = 1,
  GeomTypeLineString = 2,
  GeomTypePolygon = 3
};

typedef NS_ENUM(NSInteger, MapnikCommandType) {
  SEG_END    = 0,
  SEG_MOVETO = 1,
  SEG_LINETO = 2,
  SEG_CLOSE = (0x40 | 0x0f)
};

@class MaplyVectorTileStyle;
@class MaplyMBTileSource;
@class MaplyRemoteTileInfo;

/** 
    Container for data parsed out of a vector tile.
  */
@interface MaplyVectorTileData : NSObject

/// Component objects already added to the display, but not yet visible.
@property (nonatomic,strong,nullable) NSArray *compObjs;

/// If there were any raster layers, they're here by name
@property (nonatomic,strong,nullable) NSDictionary *rasterLayers;

/// If we asked to preserve the vector objects, these are them
@property (nonatomic,strong,nullable) NSArray *vecObjs;

/// If there are any wkcategory tags, we'll sort the component objects into groups
@property (nonatomic,strong,nullable) NSDictionary *categories;

@end

/** 
    Handles the actual data parsing for an individual vector tile after it comes in.
    
    It you're letting the toolkit do the paging, use a MaplyMapnikVectorTiles which will create one of these.  You only use this directly if you're fetching the data on your own.
  */
@interface MapboxVectorTileParser : NSObject

/// Initialize with the style delegate
- (nonnull instancetype)initWithStyle:(NSObject<MaplyVectorStyleDelegate> *__nonnull)styleDelegate viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

/// The styling delegate turns vector data into visible objects in the toolkit
@property (nonatomic, strong, nonnull) NSObject<MaplyVectorStyleDelegate> *styleDelegate;

/// Maply view controller we're adding this data to
@property (nonatomic, weak, nullable) NSObject<MaplyRenderControllerProtocol> * __weak viewC;

/// If set, we'll parse into local coordinates as specified by the bounding box, rather than geo coords
@property (nonatomic, assign) bool localCoords;

/// Keep the vector objects around as we parse them
@property (nonatomic, assign) bool keepVectors;

/// Parse everything, even if there's no style for it
@property (nonatomic, assign) bool parseAll;

@property (nonatomic, assign) BOOL debugLabel;
@property (nonatomic, assign) BOOL debugOutline;

/** 
 Construct the visible objects for the given tile

 @param bbox is in the local coordinate system (likely Spherical Mercator)
 */
- (nullable MaplyVectorTileData *)buildObjects:(NSData *__nonnull)data tile:(MaplyTileID)tileID bounds:(MaplyBoundingBox)bbox geoBounds:(MaplyBoundingBox)geoBbox;

@end

