/*
 *  MaplyMultiplexTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/5/13.
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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyRemoteTileSource.h"

/** @brief The multiplex tile source is for bunging other tile sources together.
    @details The Multiplex Tile source takes a bunch of other tiles source
    objects and switches between them.  Strictly speaking, it doesn't
    multiplex.  The quad image layer asks for all of its images at
    once for a given tile.  I just like the work 'multiplex'.
    Multiplex. Multiplex.  So futuristic in a 1970's way.  Multiplex.
    @details Anyway, this is useful for animating between different tile
    sources.
    @see MaplyTileSource
    @see MaplyQuadImageTilesLayer
  */
@interface MaplyMultiplexTileSource : NSObject<MaplyTileSource>

/** @brief Initialize with an array of MaplyRemoteTileInfo objects.
    @details To create one of these you need to pass in an NSArray of MaplyRemoteTileInfo objects.  Where this is useful is if you want to animate between them.  Check out the animation functionality in MaplyQuadImageTilesLayer.  To animate effectively, that layer must grab all the images for a tile at once, hence this object.
    @details A simple example would be taking 6 MaplyRemoteTileInfo objects pointing to MBTiles archives, sticking them in an array and passing them in here.  Each of them might represent a frame of animation in a video you want to show over the whole earth.  Because wow, man.  The whole earth.
    @return Returns a working MaplyMultiplexTileSource or nil if it can't.  That might happen if the input tile sources don't match up to each other.
  */
- (nullable instancetype)initWithSources:(NSArray *__nonnull)tileSources;

/** @brief Coordinate system for the tile source.
    @details The coordinate system is derived from the first tile source passed in.  They'd better all agree or the results will be odd.
  */
@property (nonatomic,readonly,nullable) MaplyCoordinateSystem *coordSys;

/** @brief A delegate for tile loads and failures.
    @details If set, you'll get callbacks when the various tiles load (or don't). You get called in all sorts of threads.  Act accordingly.
 */
@property (nonatomic,weak,nullable) NSObject<MaplyRemoteTileSourceDelegate> *delegate;

/** @brief If set, we'll let failures pass through.
    @details If you're fetching a very wide set of tiles, you may want to let a few failures happen and fill in the images yourself.
    @details To do that, the multiplex tile source needs to accept failures and store an NSNull in the appropriate entry.
  */
@property (nonatomic) bool acceptFailures;

/// @brief If set, we'll track the outstanding connections across all remote tile sources
+ (void)setTrackConnections:(bool)track;

/// @brief Number of outstanding connections across all tile sources
+ (int)numOutstandingConnections;

@end
