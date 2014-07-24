/*
 *  MaplyTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/7/13.
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

#import <UIKit/UIKit.h>
#import "MaplyImageTile.h"
#import "MaplyCoordinate.h"
#import "MaplyCoordinateSystem.h"

/** @typedef struct MaplyTileID
    @brief This represents the indentifier for a unique tile in the pyramid.
    @details Each tile in an image (or vector tile) pyramid can be uniquely
    identified by an x, y, and level.  The levels start at zero and run to
    a maximum.  x and y also start at zero and run to 2^level-1.
 
    @details How these are interpreted is up to the tile source itself.  For example, some tile sources start at the lower left for (0,0) and run to the upper left.  Others do it the opposite way.  There's a flipY option in the MaplyQuadImageTileLayer to deal with this, but the system doesn't care all that much as long as you are consistent.
    @see MaplyTileSource
    @see MaplyQuadPagingLayer
    @see MaplyQuadImageTilesLayer
  */
typedef struct
{
    int x, y, level;
} MaplyTileID;

/// @brief Convert a MaplyTileID to an NSString
NSString *MaplyTileIDString(MaplyTileID tileID);

/** @brief The protocol for a Maply Tile Source.  
    @details Fill out this protocol and you can pass in your own data tile by tile. This protocol is used by the MaplyQuadImageTilesLayer to pull in image data per tile.  This can be one or more images, they can be local, remote or even generated on the fly.  It's up to the object itself to return suitable data as requested or indicate failure (by returning nil).
    @details The tile source should know its coordinate system, which is handed to Maply separately.
    @see MaplyQuadImageTilesLayer
    @see MaplyMBTileSource
    @see MaplyRemoteTileSource
  */
@protocol MaplyTileSource

/// @return Returns the minimum allowable zoom layer.  Ideally, this is 0.
- (int)minZoom;

/// @return Returns the maximum allowable zoom layer.  Typically no more than 18,
///          but it depends on your implementation.
- (int)maxZoom;

/** @brief Number of pixels on the side of a single tile (e.g. 128, 256).
    @details We use this for screen space calculation, so you don't have to
     return the exact number of pixels in the imageForTile calls.  It's
     easier if you do, though.
    @return Returns the number of pixels on a side for a single tile.
  */
- (int)tileSize;

/** @brief Can the given tile be fetched locally or do we need a network call?
    @details We may ask the tile source if the tile is local or needs to be fetched over the network.  This is a hint for the loader.  Don't return true in error, though, that'll hold up the paging.
    @return Return true for local tile sources or if you have the tile cached.
  */
- (bool)tileIsLocal:(MaplyTileID)tileID;

/** @brief The coordinate system the image pyramid is in.
 @details This is typically going to be MaplySphericalMercator
 with the web mercator extents.  That's what you'll get from
 OpenStreetMap and, often, MapBox.  In other cases it might
 be MaplyPlateCarree, which covers the whole earth.  Sometimes
 it might even be something unique of your own.
 */
- (MaplyCoordinateSystem*)coordSys;

@optional

/** @brief Check if we should even try to load a given tile.
    @details Tile pyramids can be sparse.  If you know where your pyramid is sparse, you can short circuit the fetch and simply return false here.
    @details If this method isn't filled in, everything defaults to true.
    @details tileID The tile we're asking about.
    @details bbox The bounding box of the tile we're asking about, for convenience.
    @return True if the tile is loadable, false if not.
  */
- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox;

/** @brief For tiles of variable sizes, return the pixel size we'll use to evaluate this particular tile.
    @details If you have tiles with variable sizes... first of all why?  Seriously, why are you doing that?  Stop it.
    @details But if you must do variable sized tiles (Why?) fill in this method to give the importance function some clue as to what you're doing.  This will be called per tile to figure out when to load things.
    @details Variable sized tiles will screw up other things.  SO DON'T DO IT.
 */
- (int)tileSizeForTile:(MaplyTileID)tileID;

/** @brief Fetch the image for a given tile.
    @details For this method, you can return either a full UIImage or a MaplyImageTile.
    @details If you fail to load the image, just return nil.  At that point the paging won't page in tiles below this image, assuming that image pyramid is truncated at that point.
    @details If you don't have an image to load (because there isn't one) and you want the layer to keep paging below that, you should pass in a MaplyImageTile set up as a placeholder.  The visual tile will be blank, but you'll have the opportunity to provide higher resolution tiles.
    @return Return an NSData*.
  */
- (id)imageForTile:(MaplyTileID)tileID;

/** @brief Fetch the image for a given frame of a given tile.  These are for animation.
    @details For this method, you can return either a full UIImage or a MaplyImageTile.
    @details If you fail to load the image, just return nil.  At that point the paging won't page in tiles below this image, assuming that image pyramid is truncated at that point.
    @details If you don't have an image to load (because there isn't one) and you want the layer to keep paging below that, you should pass in a MaplyImageTile set up as a placeholder.  The visual tile will be blank, but you'll have the opportunity to provide higher resolution tiles.
    @param tileID Tile to load.
    @param frame Frame of tile animation to load.
    @return Return an NSData*.
 */
- (id)imageForTile:(MaplyTileID)tileID frame:(int)frame;

/** @brief Start fetching the given tile, probably with your own threads.
    @details If this is filled in that means the layer is expecting you to do your own asynchronous fetch.  You'll be called on a random thread here, so act accordingly.
    @details If you're using a MaplyQuadImageTilesLayer, when you're done fetching (successful or otherwise) call loadedImagesForTile: with the results.
    @param layer This is probably a MaplyQuadImageTilesLayer, but others use this protocol as well.  Your tile source should know.
    @param tileID The tile you should start fetching.
  */
- (void)startFetchLayer:(id)layer tile:(MaplyTileID)tileID;

/** @brief Start fetching the given tile, but just the given frame.  This is for multi-frame tiles (e.g. animations).
    @details If this is filled in that means the layer is expecting you to do your own asynchronous fetch.  You'll be called on a random thread here, so act accordingly.
    @details If you're using a MaplyQuadImageTilesLayer, when you're done fetching (successful or otherwise) call loadedImagesForTile: with the results.
    @param layer This is probably a MaplyQuadImageTilesLayer, but others use this protocol as well.  Your tile source should know.
    @param tileID The tile you should start fetching.
    @param frame The individual frame (of an animation) to fetch.
  */
- (void)startFetchLayer:(id)layer tile:(MaplyTileID)tileID frame:(int)frame;

/** @brief Called when the tile is unloaded.
    @details Normally you won't get called when an image or vector tile is unloaded from memory.  If you set this, you will.
    @details You're not required to do anything, but you can clean up data of your own if you like.
    @details You will be called on another thread, so act accordingly.
    @param tileID The tile tha that just got unloaded.
  */
- (void)tileUnloaded:(MaplyTileID)tileID;

@end
