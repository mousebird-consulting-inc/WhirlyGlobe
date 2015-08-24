/*
 *  MaplyQuadPagingLayer.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/20/13.
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

#import "MaplyComponentObject.h"
#import "MaplyViewControllerLayer.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyTileSource.h"
#import "MaplyBaseViewController.h"

@class MaplyQuadPagingLayer;

/** @brief The paging delegate is used by the paging layer to load tiles.
    @details The Maply Paging Delegate is used by the MaplyQuadPagingLayer to do feature (e.g. not image) paging.  You set up an object that implements this protocol and talks to the paging layer.  This is how you load things like vector tiles.
    @details It's up to you to do the actual loading of tiles and turn the data into Maply features.  Once you do that, the paging layer will handle the rest.
    @see MaplyQuadPagingLayer
  */
@protocol MaplyPagingDelegate

/** @brief Minimum zoom level (e.g. 0)
    @details The minimum zoom level you can provide.  If you've only got one level of data, just return minZoom and maxZoom with the same value.
  */
- (int)minZoom;

/** @brief Maximum zoom level (e.g. 17)
    @details The maximum zoom level you can provide.
  */
- (int)maxZoom;

/** @brief Start fetching data for the given tile.
    @details The paging layer calls this method to let you know you should start fetching data for a given tile. This will not be called on the main thread so be prepared for that.
    @details You should immediately do an async call to your own loading logic and then merge in your results. If you do your loading calls in line you'll slow down the loading thread. After you're done you MUST call tileDidLoad in the layer, even for failures.
    @details Once you've loaded your data, you need to create the corresponding Maply objects in the view controller and pass them back to the paging layer for tracking.
    @param tileID The tile the system wants you to start loading.
    @param layer The quad paging layer you'll hand the data over to when it's loaded.
  */
- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *__nonnull)layer;

@optional

/** @brief An optional callback when a tile is unloaded.
    @details If filled in, you'll be called when a tile is unloaded.
  */
- (void)tileDidUnload:(MaplyTileID)tileID;

@end

typedef NS_ENUM(NSInteger, MaplyQuadPagingDataStyle) {
	MaplyDataStyleAdd,
	MaplyDataStyleReplace,
};

/** @brief The Quad Paging Layer is for loading things like vector tile sets.
    @details The Maply Quad Paging Layer implements a general purpose paging interface for quad tree based data sources.  This is different from the MaplyQuadImageTilesLayer in that it's meant for paging things like vector tiles, or really any other features that are not images.
    @details You set up an object that implements the MaplyPagingDelegate protocol, create the MaplyQuadPagingLayer and respond to the startFetchForTile:forLayer: method.
    @details Once you've fetched your data for a given tile, you'll need to call tileDidLoad: or tileFailedToLoad:.  Then you'll want to create the visual objects in the MaplyViewController or WhirlyGlobeViewController and pass them in to addData:forTile:
    @details This is how the paging layer keeps track of which objects you've created for a given tile and can then remove them when the time comes.
    @details Objects must be created with @"enable" set to @(NO) in the description dictionary.  Tiles are paged before they are needed for dipslay and so the paging layer must have control over when data is displayed. It uses the enable/disable options for the various Maply objects in the view controllers.
  */
@interface MaplyQuadPagingLayer : MaplyViewControllerLayer

/** @brief Number of fetches allowed at once.
    @details Change the number of fetches allowed at once.  This means you'll have at most numSimultaneousFetches dispatch queues going.
  */
@property (nonatomic,assign) int numSimultaneousFetches;

/** @brief The importance cutoff below which we won't bother to page a tile.
    @details The paging layer will evaluate tiles based on screen space they take up.  This is the cutoff we use to evaluate when a tile is worth paging in.  It's the number of pixels a given tile would take up on the screen.
    @details Typical values would be on the order of 256*256.
  */
@property (nonatomic,assign) float importance;

/** @brief Control how tiles are indexed, either from the lower left or the upper left.
 @details If set, we'll use the OSM approach (also Google Maps) to y indexing.  This is off by default.
 @details Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if you're tile source looks odd, try setting this to false.
 @details Default value is false.
 */
@property (nonatomic) bool flipY;

/** @brief The view controller this paging layer is associated with.
    @details This view controller is the one you should create visual objects in.
  */
@property (nonatomic,weak,readonly,nullable) MaplyBaseViewController *viewC;

/** @brief Initialize with coordinate system and delegate for paging.
    @details This initializer takes the coordinate system we're working in and the MaplyPagingDelegate object.  Fill out that to do the real work.
    @param coordSys The coordinate system we're working in.
    @param tileSource The tile source that will fetch data and create visible objects.
    @return Returns a MaplyViewControllerLayer that can be added to the MaplyBaseViewController.
  */
- (nullable instancetype)initWithCoordSystem:(MaplyCoordinateSystem *__nonnull)coordSys delegate:(NSObject<MaplyPagingDelegate> *__nonnull)tileSource;

/** @brief Use the target zoom level shortcut when possible.
    @details This turns on the target zoom level shortcut as described in targetZoomLevel.  When on we'll calculate tile importance that way, that is based on a target zoom level rather than the more complex screen space calculations.
    @details It's on by default and will activate only when this layer's coordinate system is the same as the display system and there's no view matrix (e.g. tilt) set.
 */
@property (nonatomic) bool useTargetZoomLevel;

/** @brief Only load a single level at a time.
    @details When set, we'll only load one level of tiles at once.  This is very efficient for memory and fast for loading, but you'll see flashing as you move between levels.
    @details This mode only works with flat maps and is off by default.
 */
@property (nonatomic) bool singleLevelLoading;

/** @brief The target zoom level for this layer given the current view settings.
    @details Calculates the target zoom level for the middle of the screen.
    @details This only makes sense for flat maps that use the same coordinate system we're using in this tile source.  In addition, the viewer can't have a tilt or any non-2D transform in the view matrix.  If it does, this is meaningless, but it'll return a number anyway.
    @details If all those conditions are met then we can say we're only displaying a single zoom level and this is that.
 */
- (int)targetZoomLevel;

/** @brief This controls how the importance of tiles is calculated.  Either individually (false) or with the parent (true),
    @details This is a low level laoding control parameter.  Don't change unless you know why.
    @details By default this is true.
 */
@property (nonatomic) bool groupChildrenWithParent;

/** @brief You call this from your MaplyPagingDelegate with an array of data you've created for a tile.
    @details This method is called by your MaplyPagingDelegate to add MaplyComponentObject's to the data for a given tile.  Please create them disabled by putting @"enable": @(NO) in the description dictionary.  The paging layer will then be responsible for cleaning them up when needed as well as turning them on and off as the user moves around.
    @details The call is thread safe.
    @param dataObjects An NSArray of MaplyComponentObject objects.
    @param tileID The tile ID for the data we're handing over.
  */
- (void)addData:(NSArray *__nonnull)dataObjects forTile:(MaplyTileID)tileID;

/** @brief You call this from your MaplyPagingDelegate with an array of data you've created for a tile.
    @details This method is called by your MaplyPagingDelegate to add MaplyComponentObject's to the data for a given tile.  Please create them disabled by putting @"enable": @(NO) in the description dictionary.  The paging layer will then be responsible for cleaning them up when needed as well as turning them on and off as the user moves around.
    @details The call is thread safe.
    @param dataObjects An NSArray of MaplyComponentObject objects.
    @param tileID The tile ID for the data we're handing over.
    @param style If set to MaplyDataStyleReplace the data at this level will replace data at lower levels.  This is the default.  If set to MaplyDataStyleAdd then the data at this level adds to data above and below this level.
 */
- (void)addData:(NSArray *__nonnull)dataObjects forTile:(MaplyTileID)tileID style:(MaplyQuadPagingDataStyle)dataStyle;

/** @brief Called from your MaplyPagingDelegate when a tile fails to load.
    @details If you fail to load your tile data in your MaplyPagingDelegate, you need to let the paging layer know with this call.  Otherwise the paging layer assumes the tile is still loading.
  */
- (void)tileFailedToLoad:(MaplyTileID)tileID;

/** @brief Called from your MaplyPagingDelegate when a tile loads.
    @details When you've finished loading the data for a tile in your MaplyPagingDelegate, but before you've called addData:forTile: call this method to let the paging layer know you succeeded in loading the tile.
 */
- (void)tileDidLoad:(MaplyTileID)tileID;

/** @brief If you're loading a number of parts of a tile from different sources, tell the layer about it.
    @details Rather than a single tileDidLoad: call, you can break it up into the number of parts you're actually loading.  This is convenient if you're fetching data from multiple sources.
  */
- (void)tile:(MaplyTileID)tileID hasNumParts:(int)numParts;

/** @brief If you're loading your tile in parts, let the layer know which part just got loaded.
    @details If you've set the number of parts with tile:hasNumParts: this is how you let the layer know which parts you've loaded.
  */
- (void)tileDidLoad:(MaplyTileID)tileID part:(int)whichPart;

/** @brief Calculate the bounding box for a single tile in geographic.
    @details This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
    @param tileID The ID for the tile we're interested in.
    @return The lower left and upper right corner of the tile in geographic coordinates. Returns kMaplyNullBoundingBox in case of error
  */
- (MaplyBoundingBox)geoBoundsForTile:(MaplyTileID)tileID;

/** @brief Calculate the bounding box for a single tile in geographic.
 @details This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
 @param tileID The ID for the tile we're interested in.
 @param ll The lower left corner of the tile in geographic coordinates.
 @param ur The upper right corner of the tile in geographic coordinates.
 */
- (void)geoBoundsforTile:(MaplyTileID)tileID ll:(MaplyCoordinate *__nonnull)ll ur:(MaplyCoordinate *__nonnull)ur;


/** @brief Calculate the bounding box for a single tile in geographic using doubles.
 @details This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
 @param tileID The ID for the tile we're interested in.
 @return The lower left and upper right corner of the tile in geographic coordinates. Returns kMaplyNullBoundingBoxD in case of error
 */
- (MaplyBoundingBoxD)geoBoundsForTileD:(MaplyTileID)tileID;

/** @brief Calculate the bounding box for a single tile in geographic using doubles.
 @details This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
 @param tileID The ID for the tile we're interested in.
 @param ll The lower left corner of the tile in geographic coordinates.
 @param ur The upper right corner of the tile in geographic coordinates.
 */
- (void)geoBoundsForTileD:(MaplyTileID)tileID ll:(MaplyCoordinateD *__nonnull)ll ur:(MaplyCoordinateD *__nonnull)ur;

/** @brief Calculate the bounding box for a single tile in the local coordinate system.
 @details This utility method calculates the bounding box for a tile in the coordinate system used for the layer.
 @param tileID The ID for the tile we're interested in.
 @return The lower left and upper right corner of the tile in geographic coordinates.
 */
- (MaplyBoundingBox)boundsForTile:(MaplyTileID)tileID;

/** @brief Calculate the bounding box for a single tile in the local coordinate system.
    @details This utility method calculates the bounding box for a tile in the coordinate system used for the layer.
    @param tileID The ID for the tile we're interested in.
    @param ll The lower left corner of the tile in local coordinates.
    @param ur The upper right corner of the tile in local coordinates.
  */
- (void)boundsforTile:(MaplyTileID)tileID ll:(MaplyCoordinate *__nonnull)ll ur:(MaplyCoordinate *__nonnull)ur;

/** @brief Return the center of the tile in display coordinates.
    @param tileID The ID for the tile we're interested in.
    @return Return the center in display space for the given tile.
  */
- (MaplyCoordinate3d)displayCenterForTile:(MaplyTileID)tileID;

/** @brief Maximum number of tiles to load in at once.
 @details This is the maximum number of tiles the pager will have loaded into memory at once.  The default is 256 and that's generally good enough.  However, if your tile size is small, you may want to load in more.
 @details Tile loading can get out of control when using elevation data.  The toolkit calculates potential sceen coverage for each tile so elevation data makes all tiles more important.  As a result the system will happily page in way more data than you may want.  The limit becomes important in elevation mode, so leave it at 128 unless you need to change it.
 */
@property (nonatomic) int maxTiles;

/** @brief Set the minimum tile height (0.0 by default).
    @details When paging tiles containing 3D geometry it's helpful to know the min and max height.  This is the minimum height and defaults to 0.0.
  */
@property (nonatomic) float minTileHeight;

/** @brief Set the maximum tile height (0.0 off by default)
    @details When paging tiles containing 3D geometry it's helpful to know the min and max height.  This is the maximum height and it defaults to 0.0.  When min and max are the same, they are ignored.
  */
@property (nonatomic) float maxTileHeight;

/** @brief Reload the paging layer contents.
    @details This asks the paging layer to clean out its current data and reload everything from scratch.
  */
- (void)reload;


- (nullable NSObject<MaplyPagingDelegate> *)pagingDelegate;

@end
