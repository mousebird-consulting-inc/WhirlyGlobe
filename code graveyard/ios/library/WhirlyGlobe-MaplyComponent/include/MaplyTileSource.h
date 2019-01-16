/*
 *  MaplyTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/7/13.
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

#import <UIKit/UIKit.h>
#import "MaplyImageTile.h"
#import "MaplyCoordinate.h"
#import "MaplyCoordinateSystem.h"

/** 
    Status information for each frame's loading status.
 
    When loading animated frames, this contains the status of a single frame.
 */
@interface MaplyFrameStatus : NSObject

@property (nonatomic) int numTilesLoaded;
@property (nonatomic) bool fullyLoaded;
@property (nonatomic) int currentFrame;

@end

/** The remote tile info encapsulates settings for a remote tile source.
 It describes where the tile source is and presents URLs for getting the data,
 and information about local caching.
 */
@interface MaplyRemoteTileInfo : NSObject

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
 
 This is not set by default.  If set, we'll use this value as the timeout on the NSMutableURLRequest we use for fetching tiles.  This lets you extent it where appropriate or shorten it if you like.
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

/**
 Added to the NSURLRequest as an "x-auth-token" if present.
 */
@property (nonatomic,nullable) NSString *xAuthToken;

@end
