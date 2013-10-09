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

@optional

/** @brief Fetch the image for a given tile.
    @details You must fill in either imageForTile: or imagesForTile:numImages:
    if you're doing more than one image per tile.
 
    @details For this method, you can return either a full UIImage or an NSData
    containing an image that UIImage will recognize.  Typically that's
    a PNG or JPEG image of the typical size (128 or 256 pixels on a side).
 
    @details If you fail to load the image, just return nil.  At that point the paging
    won't page in tiles below this image, assuming that image pyramid is
    truncated at that point.
 
    @return Return a UIImage or an NSData containing raw PNG or JPEG data.
  */
- (id)imageForTile:(MaplyTileID)tileID;

/** @brief Fetch an array of images for the given tile.
    @details You must fill in either imageForTile: or imagesForTile:numImages:
     if you're doing more than one image per tile.
 
    @details This method is required if you've told the MaplyQuadImageTilesLayer that
    the depth is greater than 1.  That is, it's expecting more than one image
    per tile.  You'd do this when you want to animate between them, for instance.
 
    @details You must return an NSArray, but the array can contain UIImage or NSData
    entries (or both).  For NSData we're expecting a raw PNG or JPEG data,
    or anything that UIImage would interpret correctly.
 
    @return An NSArray of UIImage or NSData objects.
  */
- (NSArray *)imagesForTile:(MaplyTileID)tileID numImages:(unsigned int)numImages;

@end
