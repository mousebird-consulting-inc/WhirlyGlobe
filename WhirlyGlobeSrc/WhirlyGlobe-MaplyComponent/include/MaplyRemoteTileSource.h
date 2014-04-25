/*
 *  MaplyRemoteTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/4/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"

/** The remote tile info encapsulates settings for a remote tile source.
    It describes where the tile source is and presents URLs for getting the data,
    and information about local caching.
 */
@interface MaplyRemoteTileInfo : NSObject

/** @brief Initialize with enough information to fetch remote tiles.
 @details This version of the init method takes all the explicit
 information needed to fetch remote tiles.  This includes the
 base URL, file extension (e.g. image type), and min and max zoom levels.
 @param baseURL The base URL for fetching TMS tiles.
 @param ext Extension for the images we'll be fetching, typically @"png" or @"jpg"
 @param minZoom The minimum zoom level to fetch.  This really should be 0.
 @param maxZoom The maximum zoom level to fetch.
 @return The MaplyRemoteTileSource object or nil on failure.
 */
- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

/** @brief Initialize from a remote tile spec.
 @details This version of the initializer takes an NSDictionary parsed
 from a JSON tile spec.  Take a look at the tile spec itself
 here (https://github.com/mapbox/tilejson-spec).  Basically
 it defines the available URLs (there can be multiple), the
 min and max zoom levels, coordinate system (not really) and
 file extension.  In many cases the coordinate system extents can't
 be trusted.
 @param jsonSpec An NSDictionary parsed from the JSON tile spec.
 */
- (id)initWithTilespec:(NSDictionary *)jsonSpec;

/** @brief The base URL we're fetching from.
 @details This is typically the top of the pyramid and we'll
 tack on the level, row, and column to form a full URL.
 */
@property (nonatomic,readonly) NSString *baseURL;

/** @brief The minimum zoom level available.
 @details This is the lowest level we'll try to fetch.  Any levels below that will be filled in with placeholders.  Those are empty, but they allow us to load tiles beneath.
 */
@property (nonatomic) int minZoom;

/** @brief The maximum zoom level available.
 @details This is the highest level (e.g. largest) that we'll
 fetch for a given pyramid tile source.  The source can sparse,
 so you are not required to have these tiles available, but this
 is as high as the MaplyQuadImageTilesLayer will fetch.
 */
@property (nonatomic) int maxZoom;

/** @brief The image type and file extension for the tiles.
 @details This is the filename extension, which implies the
 image type.  It's typically @"png" or @"jpg", but it
 can be anything that UIImage will recognize.
 */
@property (nonatomic, strong) NSString *ext;

/** @brief The timeout assigned to the NSMutableURLRequest we're using to fetch tiles.
 @details This is non set by default.  If set, we'll use this value as the timeout on the NSMutableURLRequest we use for fetching tiles.  This lets you extent it where appropriate or shorten it if you like.
 */
@property (nonatomic,assign) float timeOut;

/** @brief Number of pixels on a side for any given tile.
 @details This is the number of pixels on any side for a
 given tile and it's typically 128 or 256.  This is largely
 a hint for the screen space based pager.  In most cases you
 are not required to actually return an image of the size
 you specify here, but it's a good idea.
 */
@property (nonatomic) int pixelsPerSide;

/** @brief The coordinate system the image pyramid is in.
 @details This is typically going to be MaplySphericalMercator
 with the web mercator extents.  That's what you'll get from
 OpenStreetMap and, often, MapBox.  In other cases it might
 be MaplyPlateCarree, which covers the whole earth.  Sometimes
 it might even be something unique of your own.
 */
@property (nonatomic,strong) MaplyCoordinateSystem *coordSys;

/** @brief The cache directory for image tiles.
 @details In general, we want to cache.  The globe, in particular,
    is going to fetch the same tiles over and over, quite a lot.
 The cacheing behavior is a little dumb.  It will just write
 files to the given directory forever.  If you're interacting
 with a giant image pyramid, that could be problematic.
 */
@property (nonatomic, strong) NSString *cacheDir;

/** @brief The maximum age of a cached file in seconds.
      @details If set, tiles in the cache older than this number of
      seconds will not be used; rather, a new copy of the tile will be
      retrieved from the remote source, and the locally cached tile
      will be updated. This is useful for tiles that represent
      impermanent data, such as weather information.
  */
@property (nonatomic) int cachedFileLifetime;

/** @brief Add a bounding box tiles are valid within.
 @details By default all areas within the coordinate system are valid for paging tiles.  If you call this, then only the bounding boxes you've added are valid.  You can call this method multiple times.
 @param bbox Bounding box for valid tiles in the local coordinate system.
 */
- (void)addBoundingBox:(MaplyBoundingBox *)bbox;

/** @brief Add a bounding box tiles are valid within in geo coordinates.
 @details By default all areas within the coordinate system are valid for paging tiles.  If you call this, then only the bounding boxes you've added are valid.  You can call this method multiple times.
 @param bbox Bounding box for valid tiles in geo coordinates (radians).
 */
- (void)addGeoBoundingBox:(MaplyBoundingBox *)bbox;

/** @brief Generate the request for a given tile.
 @details If someone outside of this request wants to fetch the data directly, they can do so by using this NSURLRequest.
 @param tileID The tile we'd like the NSURLRequest for.
 @return An NSURLRequest object you can use to fetch data for the tile.
 */
- (NSURLRequest *)requestForTile:(MaplyTileID)tileID;

/** @brief The full path for a cached tile.
 @details This returns the full path to where a tile is or where a tile would be if it were cached.
 @details We don't check if the tile is there or not.
 @param tileID The tile we need the filename for.
 */
- (NSString *)fileNameForTile:(MaplyTileID)tileID;

/** @brief Check if a given tile is stored in the local cache.
    @details This checks if the given tile ID is represented in the local cache directory.
    @param tileID The tile we'd like to check for.
  */
- (bool)tileIsLocal:(MaplyTileID)tileID;

/** @brief Check if we should even try to load a given tile.
 @details Check whether tile level is within zoom limits for the source, and if the tile is within any MBRs that have been added.
 @param tileID The tile we're asking about.
 @param bbox The bounding box of the tile we're asking about, for convenience.
 @return True if the tile is loadable, false if not.
 */
- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox;

@end

@class MaplyRemoteTileSource;

/** The remote tile source delegate provides feedback on which
    tiles loaded and which didn't.  You'll be called in all sorts of
    random threads here, so act accordingly.
  */
@protocol MaplyRemoteTileSourceDelegate <NSObject>

@optional

/** The tile successfully loaded.
    @param tileSource the remote tile source that loaded the tile.
    @param tileID The ID of the tile we loaded.
  */
- (void) remoteTileSource:(id)tileSource tileDidLoad:(MaplyTileID)tileID;

/** The tile failed to load.
    @param tileSource The remote tile source that tried to load the tile.
    @param tileID The tile ID of the tile that failed to load.
    @param error The NSError message, probably from the network routine.
  */
- (void) remoteTileSource:(id)tileSource tileDidNotLoad:(MaplyTileID)tileID error:(NSError *)error;

@end

/** @brief The remote tile source knows how to fetch remote image pyramids.
    @details This is the MaplyTileSource compliant object that communicates with remote servers and fetches individual tiles as needed by the MaplyQuadImageTileLayer.
    @details It can be initialized in a couple of different ways depending on the information you have available.  Either you explicitly provide the baseURL, min and max levels and such, or hand in an NSDictionary that's been parsed from a tile spec.
    @details The remote tile source also handles cacheing if it you give it a cacheDir to work in.  By default cacheing is off (so be careful).
    @see MaplyQuadImageTilesLayer
 */
@interface MaplyRemoteTileSource : NSObject<MaplyTileSource>

/** @brief Initialize with enough information to fetch remote tiles.
    @details This version of the init method takes all the explicit
     information needed to fetch remote tiles.  This includes the
     base URL, file extension (e.g. image type), and min and max zoom levels.
    @param baseURL The base URL for fetching TMS tiles.
    @param ext Extension for the images we'll be fetching, typically @"png" or @"jpg"
    @param minZoom The minimum zoom level to fetch.  This really should be 0.
    @param maxZoom The maximum zoom level to fetch.
    @return The MaplyRemoteTileSource object or nil on failure.
  */
- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

/** @brief Initialize from a remote tile spec.
    @details This version of the initializer takes an NSDictionary parsed
    from a JSON tile spec.  Take a look at the tile spec itself
    here (https://github.com/mapbox/tilejson-spec).  Basically
    it defines the available URLs (there can be multiple), the
    min and max zoom levels, coordinate system (not really) and
    file extension.  In many cases the coordinate system extents can't
    be trusted.
    @param jsonSpec An NSDictionary parsed from the JSON tile spec.
  */
- (id)initWithTilespec:(NSDictionary *)jsonSpec;

/** @brief Read the image, but only if it's in the cache.
    @details MaplyRemoteTileSource uses asynchronous fetching.  This method is inherently
    synchronous.  You expect it to return the image for a tile right now.  So this method
    only works if the tile is in the cache.  Be sure to check on that first.
    @param tileID The tile (that should be in the cache) you want to read.
  */
- (id)imageForTile:(MaplyTileID)tileID;

/** @brief Initialize with remote tile info.
    @details The MaplyRemoteTileInfo object stores all the specifics about remote tile data.
    We actually created one of those for the other init calls in this objects.  Fill that object
    in and then call this initializaer.
    @param info The MaplyRemoteTileInfo describing where to fetch the tiles.
  */
- (id)initWithInfo:(MaplyRemoteTileInfo *)info;

/** @brief Description of where we fetch the tiles from and where to cache them.
  */
@property (nonatomic,readonly) MaplyRemoteTileInfo *tileInfo;

/** @brief A delegate for tile loads and failures.
    @details If set, you'll get callbacks when the various tiles load (or don't). You get called in all sorts of threads.  Act accordingly.
  */
@property (nonatomic,weak) NSObject<MaplyRemoteTileSourceDelegate> *delegate;

/// @brief Passes through the coord system from the MaplyRemoteTileInfo
@property (nonatomic,strong) MaplyCoordinateSystem *coordSys;

/// @brief Passes through the cacheDir from the MaplyRemoteTileInfo
@property (nonatomic,strong) NSString *cacheDir;

@end
