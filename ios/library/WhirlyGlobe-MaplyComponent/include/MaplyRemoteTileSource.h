/*
 *  MaplyRemoteTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/4/13.
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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"

@protocol MaplyRemoteTileInfoProtocol

/** 
    The coordinate system the image pyramid is in.
 
    This is typically going to be MaplySphericalMercator
 with the web mercator extents.  That's what you'll get from
 OpenStreetMap and, often, MapBox.  In other cases it might
 be MaplyPlateCarree, which covers the whole earth.  Sometimes
 it might even be something unique of your own.
 */
- (MaplyCoordinateSystem * _Nullable) coordSys;

/** 
    The minimum zoom level available.
 
    This is the lowest level we'll try to fetch.  Any levels below that will be filled in with placeholders.  Those are empty, but they allow us to load tiles beneath.
 */
- (int) minZoom;

/** 
    The maximum zoom level available.
 
    This is the highest level (e.g. largest) that we'll
 fetch for a given pyramid tile source.  The source can sparse,
 so you are not required to have these tiles available, but this
 is as high as the MaplyQuadImageTilesLayer will fetch.
 */
- (int)maxZoom;

/** 
    Number of pixels on a side for any given tile.
 
    This is the number of pixels on any side for a
 given tile and it's typically 128 or 256.  This is largely
 a hint for the screen space based pager.  In most cases you
 are not required to actually return an image of the size
 you specify here, but it's a good idea.
 */
- (int) pixelsPerSide;

/** 
    Generate the request for a given tile.
 
    If someone outside of this request wants to fetch the data directly, they can do so by using this NSURLRequest.
 
    @param tileID The tile we'd like the NSURLRequest for.
 
    @return An NSURLRequest object you can use to fetch data for the tile.
 */
- (nullable NSURLRequest *)requestForTile:(MaplyTileID)tileID;

/** 
    Check if a given tile is stored in the local cache.
 
    This checks if the given tile ID is represented in the local cache directory.
 
    @param tileID The tile we'd like to check for.
 
    @param frame If you're loading individual frames this will be the frame.  Otherwise, -1.
 */
- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame;

/** 
    Check if we should even try to load a given tile.
 
    Check whether tile level is within zoom limits for the source, and if the tile is within any MBRs that have been added.
 
    @param tileID The tile we're asking about.
 
    @param bbox The bounding box of the tile we're asking about, for convenience.
 
    @return True if the tile is loadable, false if not.
 */
- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox)bbox;

/** 
    Read a tile from your local cache.
    
    If you have a cache, read the tile from that cache and return it.  This is a synchronous call.
    
    If you don't have a cache, just return nil.
    
    @param tileID The tile to read from your cache.
  */
- (NSData * _Nullable)readFromCache:(MaplyTileID)tileID;

/** 
    Write a given data tile to your local cache.
    
    With this call, you're supposed to write the tile, which has presumably been fetched remotely to your own cache.
    
    If you don't have a cache, don't do anything.
    
    @param tileID Tile to write to cache.
    
    @param tileData Data for tile.
  */
- (void)writeToCache:(MaplyTileID)tileID tileData:(NSData * _Nonnull)tileData;

@end

/** The remote tile info encapsulates settings for a remote tile source.
    It describes where the tile source is and presents URLs for getting the data,
    and information about local caching.
 */
@interface MaplyRemoteTileInfo : NSObject<MaplyRemoteTileInfoProtocol>

/** 
    Initialize with enough information to fetch remote tiles.
 
    This version of the init method takes all the explicit
 information needed to fetch remote tiles.  This includes the
 base URL, file extension (e.g. image type), and min and max zoom levels.
 
    @param baseURL The base URL for fetching TMS tiles.
 
    @param ext Extension for the images we'll be fetching, typically @"png" or @"jpg"
 
    @param minZoom The minimum zoom level to fetch.  This really should be 0.
 
    @param maxZoom The maximum zoom level to fetch.
 
    @return The MaplyRemoteTileSource object or nil on failure.
 */
- (nonnull instancetype)initWithBaseURL:(NSString *__nonnull)baseURL ext:(NSString *__nullable)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

/** 
    Initialize from a remote tile spec.
 
    This version of the initializer takes an NSDictionary parsed
 from a JSON tile spec.  Take a look at the tile spec itself
 here (https://github.com/mapbox/tilejson-spec).  Basically
 it defines the available URLs (there can be multiple), the
 min and max zoom levels, coordinate system (not really) and
 file extension.  In many cases the coordinate system extents can't
 be trusted.
 
    @param jsonSpec An NSDictionary parsed from the JSON tile spec.
 */
- (nullable instancetype)initWithTilespec:(NSDictionary *__nonnull)jsonSpec;

/** 
    The base URL we're fetching from.
 
    This is typically the top of the pyramid and we'll
 tack on the level, row, and column to form a full URL.
 */
@property (nonatomic,readonly,nonnull) NSString *baseURL;

/** 
    The minimum zoom level available.
 
    This is the lowest level we'll try to fetch.  Any levels below that will be filled in with placeholders.  Those are empty, but they allow us to load tiles beneath.
 */
@property (nonatomic) int minZoom;

/** 
    The maximum zoom level available.
 
    This is the highest level (e.g. largest) that we'll
 fetch for a given pyramid tile source.  The source can sparse,
 so you are not required to have these tiles available, but this
 is as high as the MaplyQuadImageTilesLayer will fetch.
 */
@property (nonatomic) int maxZoom;

/** 
    The image type and file extension for the tiles.
 
    This is the filename extension, which implies the
 image type.  It's typically @"png" or @"jpg", but it
 can be anything that UIImage will recognize.
 */
@property (nonatomic, strong, nonnull) NSString *ext;

/** 
    The timeout assigned to the NSMutableURLRequest we're using to fetch tiles.
 
    This is non set by default.  If set, we'll use this value as the timeout on the NSMutableURLRequest we use for fetching tiles.  This lets you extent it where appropriate or shorten it if you like.
 */
@property (nonatomic,assign) float timeOut;

/** 
    Number of pixels on a side for any given tile.
 
    This is the number of pixels on any side for a
 given tile and it's typically 128 or 256.  This is largely
 a hint for the screen space based pager.  In most cases you
 are not required to actually return an image of the size
 you specify here, but it's a good idea.
 */
@property (nonatomic) int pixelsPerSide;

/** 
    The coordinate system the image pyramid is in.
 
    This is typically going to be MaplySphericalMercator
 with the web mercator extents.  That's what you'll get from
 OpenStreetMap and, often, MapBox.  In other cases it might
 be MaplyPlateCarree, which covers the whole earth.  Sometimes
 it might even be something unique of your own.
 */
@property (nonatomic,strong,nullable) MaplyCoordinateSystem *coordSys;

/** 
    The cache directory for image tiles.
 
    In general, we want to cache.  The globe, in particular,
    is going to fetch the same tiles over and over, quite a lot.
 The cacheing behavior is a little dumb.  It will just write
 files to the given directory forever.  If you're interacting
 with a giant image pyramid, that could be problematic.
 */
@property (nonatomic, strong,nullable) NSString *cacheDir;

/** 
    The maximum age of a cached file in seconds.
      
    If set, tiles in the cache older than this number of
      seconds will not be used; rather, a new copy of the tile will be
      retrieved from the remote source, and the locally cached tile
      will be updated. This is useful for tiles that represent
      impermanent data, such as weather information.
  */
@property (nonatomic) int cachedFileLifetime;

/** 
    Query string to add after the URL we're fetching from.
    
    Add your access tokens and other query arguments.
  */
@property (nonatomic,strong,nullable) NSString *queryStr;

/// If set, we've decided this is a replacement URL with {x},{y} and {z} in the baseURL string
@property (nonatomic) bool replaceURL;

/** 
    Add a bounding box tiles are valid within.
 
    By default all areas within the coordinate system are valid for paging tiles.  If you call this, then only the bounding boxes you've added are valid.  You can call this method multiple times.
 
    @param bbox Bounding box for valid tiles in the local coordinate system.
 */
- (void)addBoundingBox:(MaplyBoundingBox *__nonnull)bbox;

/** 
    Add a bounding box tiles are valid within in geo coordinates.
 
    By default all areas within the coordinate system are valid for paging tiles.  If you call this, then only the bounding boxes you've added are valid.  You can call this method multiple times.
 
    @param bbox Bounding box for valid tiles in geo coordinates (radians).
 */
- (void)addGeoBoundingBox:(MaplyBoundingBox *__nonnull)bbox;

/** 
    Generate the request for a given tile.
 
    If someone outside of this request wants to fetch the data directly, they can do so by using this NSURLRequest.
 
    @param tileID The tile we'd like the NSURLRequest for.
 
    @return An NSURLRequest object you can use to fetch data for the tile.
 */
- (nullable NSURLRequest *)requestForTile:(MaplyTileID)tileID;

/** 
    The full path for a cached tile.
 
    This returns the full path to where a tile is or where a tile would be if it were cached.
 
    We don't check if the tile is there or not.
 
    @param tileID The tile we need the filename for.
 */
- (nullable NSString *)fileNameForTile:(MaplyTileID)tileID;

/** 
    Check if a given tile is stored in the local cache.
    
    This checks if the given tile ID is represented in the local cache directory.
    
    @param tileID The tile we'd like to check for.
    
    @param frame If you're loading individual frames this will be the frame.  Otherwise, -1.
  */
- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame;

/** 
    Check if we should even try to load a given tile.
 
    Check whether tile level is within zoom limits for the source, and if the tile is within any MBRs that have been added.
 
    @param tileID The tile we're asking about.
 
    @param bbox The bounding box of the tile we're asking about, for convenience.
 
    @return True if the tile is loadable, false if not.
 */
- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox)bbox;

/** 
    Assign a user object here to get it passed back to you.
    
    Optional property used by developers.
  */
@property (nonatomic,strong) id _Nullable userObject;

@end

@class MaplyRemoteTileSource;

/** 
    A delegate called during various parts of the tile loading and display operation.
    
    The remote tile source delegate provides feedback on which
    tiles loaded and which didn't.  You'll be called in all sorts of
    random threads here, so act accordingly.
    
    This delegate interface can also be used to modify data as it comes in.
  */
@protocol MaplyRemoteTileSourceDelegate <NSObject>

@optional

/** 
    The tile successfully loaded.
    
    @param tileSource the remote tile source that loaded the tile.
    
    @param tileID The ID of the tile we loaded.
  */
- (void) remoteTileSource:(id __nonnull)tileSource tileDidLoad:(MaplyTileID)tileID;

/** 
    Modify the tile data after it's been read.
    
    This method is useful for messing with tile sources that may not be images, but can be turned into images.
  */
- (nonnull NSData *) remoteTileSource:(id __nonnull)tileSource modifyTileReturn:(NSData *__nonnull)tileData forTile:(MaplyTileID)tileID;

/** 
    The tile failed to load.
    
    @param tileSource The remote tile source that tried to load the tile.
    
    @param tileID The tile ID of the tile that failed to load.
    
    @param error The NSError message, probably from the network routine.
  */
- (void) remoteTileSource:(id __nonnull)tileSource tileDidNotLoad:(MaplyTileID)tileID error:(NSError *__nullable)error;

/** 
    Called when the tile is disabled.
 */
- (void)remoteTileSource:(id __nonnull)tileSource tileDisabled:(MaplyTileID)tileID;

/** 
    Called when the tile is enabled.
 */
- (void)remoteTileSource:(id __nonnull)tileSource tileEnabled:(MaplyTileID)tileID;

/** 
    Called when the tile is unloaded.
    
    Normally you won't get called when an image or vector tile is unloaded from memory.  If you set this, you will.
    
    You're not required to do anything, but you can clean up data of your own if you like.
    
    You will be called on another thread, so act accordingly.
    
    @param tileID The tile that that just got unloaded.
 */
- (void)remoteTileSource:(id __nonnull)tileSource tileUnloaded:(MaplyTileID)tileID;

@end

/** 
    The remote tile source knows how to fetch remote image pyramids.
    
    This is the MaplyTileSource compliant object that communicates with remote servers and fetches individual tiles as needed by the MaplyQuadImageTileLayer.
    
    It can be initialized in a couple of different ways depending on the information you have available.  Either you explicitly provide the baseURL, min and max levels and such, or hand in an NSDictionary that's been parsed from a tile spec.
    
    The remote tile source also handles cacheing if it you give it a cacheDir to work in.  By default cacheing is off (so be careful).
    @see MaplyQuadImageTilesLayer
 */
@interface MaplyRemoteTileSource : NSObject<MaplyTileSource>

/** 
    Initialize with enough information to fetch remote tiles.
    
    This version of the init method takes all the explicit
     information needed to fetch remote tiles.  This includes the
     base URL, file extension (e.g. image type), and min and max zoom levels.
    
    @param baseURL The base URL for fetching TMS tiles.
    
    @param ext Extension for the images we'll be fetching, typically @"png" or @"jpg"
    
    @param minZoom The minimum zoom level to fetch.  This really should be 0.
    
    @param maxZoom The maximum zoom level to fetch.
    
    @return The MaplyRemoteTileSource object or nil on failure.
  */
- (nullable instancetype)initWithBaseURL:(NSString *__nonnull)baseURL ext:(NSString *__nullable)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

/** 
    Initialize from a remote tile spec.
    
    This version of the initializer takes an NSDictionary parsed
    from a JSON tile spec.  Take a look at the tile spec itself
    here (https://github.com/mapbox/tilejson-spec).  Basically
    it defines the available URLs (there can be multiple), the
    min and max zoom levels, coordinate system (not really) and
    file extension.  In many cases the coordinate system extents can't
    be trusted.
    
    @param jsonSpec An NSDictionary parsed from the JSON tile spec.
  */
- (nullable instancetype)initWithTilespec:(NSDictionary *__nonnull)jsonSpec;

/** 
    Read the image, but only if it's in the cache.
    
    MaplyRemoteTileSource uses asynchronous fetching.  This method is inherently
    synchronous.  You expect it to return the image for a tile right now.  So this method
    only works if the tile is in the cache.  Be sure to check on that first.
    
    @param tileID The tile (that should be in the cache) you want to read.
  */
- (nullable id)imageForTile:(MaplyTileID)tileID;

/** 
    Initialize with remote tile info.
    
    The MaplyRemoteTileInfo object stores all the specifics about remote tile data.
    We actually created one of those for the other init calls in this objects.  Fill that object
    in and then call this initializaer.
    
    @param info The MaplyRemoteTileInfo describing where to fetch the tiles.
  */
- (nullable instancetype)initWithInfo:(NSObject<MaplyRemoteTileInfoProtocol> *__nonnull)info;

/** 
    Description of where we fetch the tiles from and where to cache them.
  */
@property (nonatomic,readonly,nonnull) NSObject<MaplyRemoteTileInfoProtocol> *tileInfo;

/** 
    A delegate for tile loads and failures.
    
    If set, you'll get callbacks when the various tiles load (or don't). You get called in all sorts of threads.  Act accordingly.
  */
@property (nonatomic,weak,nullable) NSObject<MaplyRemoteTileSourceDelegate> *delegate;

/// Passes through the coord system from the MaplyRemoteTileInfo
@property (nonatomic,strong,nonnull) MaplyCoordinateSystem *coordSys;

/// Passes through the cacheDir from the MaplyRemoteTileInfo
@property (nonatomic,strong,nullable) NSString *cacheDir;

/// If set, we'll track the outstanding connections across all remote tile sources
+ (void)setTrackConnections:(bool)track;

/// Number of outstanding connections across all remote tile sources
+ (int)numOutstandingConnections;

@end
