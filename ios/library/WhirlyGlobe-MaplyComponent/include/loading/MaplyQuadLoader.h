/*
 *  MaplyQuadLoader.h
 *
 *  Created by Steve Gifford on 2/12/19.
 *  Copyright 2012-2019 Saildrone Inc
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
#import "control/MaplyControllerLayer.h"
#import "math/MaplyCoordinateSystem.h"
#import "loading/MaplyTileSourceNew.h"
#import "control/MaplyRenderController.h"
#import "loading/MaplyQuadSampler.h"
#import "loading/MaplyRemoteTileFetcher.h"

@class MaplyQuadLoaderBase;

/**
 Passed in to and returned by the Loader Interpreter.
 
 We pass this into the interpreter with the unparsed data.  It parses it and passes that
 data back, possibly with an error.
 */
@interface MaplyLoaderReturn : NSObject

/// Initialize with the loader this will be attached to
- (id __nonnull)initWithLoader:(MaplyQuadLoaderBase * __nonnull)loader;

/// Tile this is the data for
@property (nonatomic) MaplyTileID tileID;

/// If set, the frame.  -1 by default
@property (nonatomic) int frame;

/// Data returned from a tile request.  Unparsed.
/// You can add multiple of these, but the interpreter should be expecting that
- (void)addTileData:(NSData *__nonnull) tileData;

/// Return the tile NSData objects as an array
- (NSArray<NSData *> * __nonnull)getTileData;

/// Return the first data object.  You're probably only expecting the one.
- (NSData * __nullable)getFirstData;

/// If this is set, the tile failed to parse
/// You can set it and the system will deal with the results
@property (nonatomic,strong) NSError * __nullable error;

@end

/**
 Loader Interpreter converts raw data into images and objects.
 
 Converts data returned from a remote source (or cache) into images and/or
 MaplyComponentObjects that have already been added to the view (disabled).
 */
@protocol MaplyLoaderInterpreter<NSObject>

/** Set when the loader first starts up.
 
    If you need to tweak loader settings, do it here.
  */
- (void)setLoader:(MaplyQuadLoaderBase * __nonnull)loader;

/**
 Parse the data coming back from a remote request and turn it into something we can use.
 
 Convert the NSData passed in to image and component objects (e.g. add stuff to the view controller).
 Everything added should be disabled to start.
 */
- (void)dataForTile:(MaplyLoaderReturn * __nonnull)loadReturn loader:(MaplyQuadLoaderBase * __nonnull)loader;

/**
  Notification that the tile was unloaded by the system.  If you're tracking your own resources, you may need this.
 */
- (void)tileUnloaded:(MaplyTileID)tileID;

@end

/** Base class for the quad loaders.

    The image, frame, and data paging loaders all share much of the same functionality.
 */
@interface MaplyQuadLoaderBase : NSObject

/**
 Control how tiles are indexed, either from the lower left or the upper left.
 
 If set, we'll use the OSM approach (also Google Maps) to y indexing.  That's that default and it's normally what you're run into.
 
 Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if your tile source looks odd, try setting this to false.
 
 Default value is true.
 */
@property (nonatomic) bool flipY;

/// Set for a lot of debugging output
@property (nonatomic,assign) bool debugMode;

/// View controller this is attached to.
/// Useful for delegate calls that might not be tracking that.
@property (nonatomic,readonly,weak,nullable) NSObject<MaplyRenderControllerProtocol> *viewC;

/// If set, we'll call the interpreter on this queue
@property (nonatomic,nullable,strong) dispatch_queue_t queue;

// True if the loader is not currently loading anything
- (bool)isLoading;

/**
 Calculate the bounding box for a single tile in geographic.
 
 This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
 
 @param tileID The ID for the tile we're interested in.
 
 @return The lower left and upper right corner of the tile in geographic coordinates. Returns kMaplyNullBoundingBox in case of error
 */
- (MaplyBoundingBox)geoBoundsForTile:(MaplyTileID)tileID;

/**
 Calculate the bounding box for a single tile in geographic using doubles.
 
 This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
 
 @param tileID The ID for the tile we're interested in.
 
 @return The lower left and upper right corner of the tile in geographic coordinates. Returns kMaplyNullBoundingBoxD in case of error
 */
- (MaplyBoundingBoxD)geoBoundsForTileD:(MaplyTileID)tileID;

/**
 Calculate the bounding box for a single tile in the local coordinate system.
 
 This utility method calculates the bounding box for a tile in the coordinate system used for the layer.
 
 @param tileID The ID for the tile we're interested in.
 
 @return The lower left and upper right corner of the tile in local coordinates.
 */
- (MaplyBoundingBox)boundsForTile:(MaplyTileID)tileID;

/**
 Calculate the bounding box for a single tile in the local coordinate system using doubles.
 
 This utility method calculates the bounding box for a tile in the coordinate system used for the layer.
 
 @param tileID The ID for the tile we're interested in.
 
 @return The lower left and upper right corner of the tile in geographic coordinates.
 */
- (MaplyBoundingBoxD)boundsForTileD:(MaplyTileID)tileID;

/**
 Return the center of the tile in display coordinates.
 
 @param tileID The ID for the tile we're interested in.
 
 @return Return the center in display space for the given tile.
 */
- (MaplyCoordinate3d)displayCenterForTile:(MaplyTileID)tileID;


/// Use a specific tile fetcher rather than the one shared by everyone else
- (void)setTileFetcher:(NSObject<MaplyTileFetcher> * __nonnull)tileFetcher;

/// Set the interpreter for the data coming back.  If you're just getting images, don't set this.
- (void)setInterpreter:(NSObject<MaplyLoaderInterpreter> * __nonnull)interp;

/**
 Change the interpreter and reload all the data.
 <br>
 You can change the tile interpreter being used to build objects and images.
 This will then force a reload of the tiles (hopefully from cache) and the
 visuals will change as everything comes in.
 */
- (void)changeInterpreter:(NSObject<MaplyLoaderInterpreter> *__nonnull)interp;

/**
  Force a reload of the data.
  <br>
  All the current loads will be cancelled, any in flight will be ignored
  and the loader will ask for a whole new set of data.
  */
- (void)reload;

/** Turn off the loader and shut things down.
 This unregisters us with the sampling layer and shuts down the various objects we created.
 */
- (void)shutdown;

@end
