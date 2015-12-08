/*
 *  MaplyMapnikVectorTiles.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Copyright 2011-2015 mousebird consulting
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

/** @brief Geometry type for data found within PBF files.
    @details These are the geometry types supported within Mapnik PBF files.
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

/** @brief Protocol for styling the vectors.
    @details You pass in an object which adheres to this protocol and will style
    the vectors read by a MaplyMapnikVectorTiles object.  In general, this will be
    a parsed Mapnik vector file, but you can substitute your own logic as well.
  */
@protocol MaplyVectorStyleDelegate <NSObject>

/** @brief Return the styles that apply to the given feature (attributes).
  */
- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                      onTile:(MaplyTileID)tileID
                    inLayer:(NSString *__nonnull)layer
                       viewC:(MaplyBaseViewController *__nonnull)viewC;

/// @brief Return true if the given layer is meant to display for the given tile (zoom level)
- (BOOL)layerShouldDisplay:(NSString *__nonnull)layer tile:(MaplyTileID)tileID;

/// @brief Return the style associated with the given UUID.
- (nullable MaplyVectorTileStyle *)styleForUUID:(NSString *__nonnull)uiid viewC:(MaplyBaseViewController *__nonnull)viewC;

@end

/** @brief Container for data parsed out of a vector tile.
  */
@interface MaplyVectorTileData : NSObject

/// @brief Component objects already added to the display, but not yet visible.
@property (nonatomic,strong,nullable) NSArray *compObjs;

/// @brief If there were any raster layers, they're here by name
@property (nonatomic,strong,nullable) NSDictionary *rasterLayers;

@end

/** @brief Handles the actual data parsing for an individual vector tile after it comes in.
    @details It you're letting the toolkit do the paging, use a MaplyMapnikVectorTiles which will create one of these.  You only use this directly if you're fetching the data on your own.
  */
@interface MaplyMapnikVectorTileParser : NSObject

/// @brief Initialize with the style delegate
- (nonnull instancetype)initWithStyle:(NSObject<MaplyVectorStyleDelegate> *__nonnull)styleDelegate viewC:(MaplyBaseViewController *__nonnull)viewC;

/// @brief The styling delegate turns vector data into visible objects in the toolkit
@property (nonatomic, strong, nonnull) NSObject<MaplyVectorStyleDelegate> *styleDelegate;

/// @brief Maply view controller we're adding this data to
@property (nonatomic, weak, nullable) MaplyBaseViewController *viewC;

@property (nonatomic, assign) BOOL debugLabel;
@property (nonatomic, assign) BOOL debugOutline;

/// @brief Construct the visible objects for the given tile
/// @param bbox is in the local coordinate system (likely Spherical Mercator)
- (nullable MaplyVectorTileData *)buildObjects:(NSData *__nonnull)data tile:(MaplyTileID)tileID bounds:(MaplyBoundingBox)bbox;

@end

/// @brief The various types of style that will work with Mapnik vector tiles
typedef NS_ENUM(NSInteger, MapnikStyleType) {
	MapnikXMLStyle,
	MapnikMapboxGLStyle
};

/** @brief Provides on demand creation for Mapnik style vector tiles.
    @details Create one of these to read Mapnik PBF style tiles from a remote
    or local source.  This handles the geometry creation, calls a delegate
    for the styling and can read from remote or local data files.
  */
@interface MaplyMapnikVectorTiles : NSObject <MaplyPagingDelegate>

/// @brief One or more tile sources to fetch data from per tile
@property (nonatomic, readonly, nullable) NSArray *tileSources;

/// @brief Access token to use with the remote service
@property (nonatomic, strong, nonnull) NSString *accessToken;

/// @brief Handles the actual Mapnik vector tile parsing
@property (nonatomic, strong, nullable) MaplyMapnikVectorTileParser *tileParser;

/// @brief Minimum zoom level available
@property (nonatomic, assign) int minZoom;

/// @brief Maximum zoom level available
@property (nonatomic, assign) int maxZoom;

/** @brief A convenience method that fetches all the relevant files and creates a vector tiles object.
    @details This method will fetch all the relevant config files necessary to start a Mapnik vector tile object and the call you back to set up the actual layer.
    @param tileSpec Either a local filename or a URL to the remote JSON tile spec.
    @param accessToken The access key provided by your service.
    @param styleFile Either a local file name or a URL for the Mapnik XML style file.
    @param cacheDir Where to cache the vector tiles, or nil for no caching.
    @param viewC View controller the data will be associated with.
    @param successBlock This block is called with the vector tiles object on success.  You'll need to create the paging layer and attach the vector tiles to it.
    @param failureBlock This block is called if any of the loading fails.
  */
+ (void) StartRemoteVectorTilesWithTileSpec:(NSString *__nonnull)tileSpec accessToken:(NSString *__nonnull)accessToken style:(NSString *__nonnull)styleFile styleType:(MapnikStyleType)styleType cacheDir:(NSString *__nonnull)cacheDir viewC:(MaplyBaseViewController *__nonnull)viewC success:(void (^__nonnull)(MaplyMapnikVectorTiles *__nonnull vecTiles))successBlock failure:(void (^__nonnull)(NSError *__nonnull error))failureBlock;

/** @brief A convenience method that fetches all the relevant files and creates a vector tiles object.
    @details This method will fetch all the relevant config files necessary to start a Mapnik vector tile object and the call you back to set up the actual layer.
    @param tileURL The URL to fetch vector tiles from.
    @param accessToken The access key provided by your service.
    @param ext The tile extension to use.
    @param minZoom The minimum zoom level to use.
    @param maxZoom The maximum zoom level to use
    @param styleFile Either a local file name or a URL for the Mapnik XML style file.
    @param cacheDir Where to cache the vector tiles, or nil for no caching.
    @param viewC View controller the data will be associated with.
    @param successBlock This block is called with the vector tiles object on success.  You'll need to create the paging layer and attach the vector tiles to it.
    @param failureBlock This block is called if any of the loading fails.
 */
+ (void) StartRemoteVectorTilesWithURL:(NSString *__nonnull)tileURL ext:(NSString *__nonnull)ext minZoom:(int)minZoom maxZoom:(int)maxZoom accessToken:(NSString *__nonnull)accessToken style:(NSString *__nonnull)styleFile styleType:(MapnikStyleType)styleType cacheDir:(NSString *__nonnull)cacheDir viewC:(MaplyBaseViewController *__nonnull)viewC success:(void (^__nonnull)(MaplyMapnikVectorTiles *__nonnull vecTiles))successBlock failure:(void (^__nonnull)(NSError *__nonnull error))failureBlock;

/** @brief Init with a single remote tile source.
  */
- (nonnull instancetype)initWithTileSource:(NSObject<MaplyTileSource> *__nonnull)tileSource style:(NSObject<MaplyVectorStyleDelegate> *__nonnull)style viewC:(MaplyBaseViewController *__nonnull)viewC;

/** @brief Init with a list of tile sources.
    @details These are MaplyRemoteTileInfo objects and will be combined by the
    MaplyMapnikVectorTiles object for display.
*/
- (nonnull instancetype)initWithTileSources:(NSArray *__nonnull)tileSources style:(NSObject<MaplyVectorStyleDelegate> *__nonnull)style viewC:(MaplyBaseViewController *__nonnull)viewC;

/** @brief Init with the filename of an MBTiles archive containing PBF tiles.
    @details This will read individual tiles from an MBTiles archive containging PBF.
    @details The file should be local.
  */
- (nonnull instancetype)initWithMBTiles:(MaplyMBTileSource *__nonnull)tileSource style:(NSObject<MaplyVectorStyleDelegate> *__nonnull)style viewC:(MaplyBaseViewController *__nonnull)viewC;

@end
