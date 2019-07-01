/*
 *  MaplyMultiplexTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/5/13.
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
#import "MaplyRemoteTileSource.h"

/** 
    A delegate called during various parts of the tile loading and display operation.
 
    The remote tile source delegate provides feedback on which
 tiles loaded and which didn't.  You'll be called in all sorts of
 random threads here, so act accordingly.
 
    This delegate interface can also be used to modify data as it comes in.
 */
@protocol MaplyMultiplexTileSourceDelegate <NSObject>

@optional

/** 
    The tile successfully loaded.
 
    @param tileInfo TileInfo corresponding to the data that was loaded.
 
    @param tileID The ID of the tile we loaded.
 
    @param frame The frame loaded.
 */
- (void) remoteTileInfo:(id __nonnull)tileInfo tileDidLoad:(MaplyTileID)tileID frame:(int)frame;

/** 
    Modify the tile data after it's been read.
 
    This method is useful for messing with tile sources that may not be images, but can be turned into images.
 */
- (nonnull NSData *) remoteTileInfo:(id __nonnull)tileInfo modifyTileReturn:(NSData *__nonnull)tileData forTile:(MaplyTileID)tileID frame:(int)frame;

/** 
    The tile failed to load.
 
    @param tileInfo TileInfo corresponding to the data that was loaded.
 
    @param tileID The tile ID of the tile that failed to load.
 
    @param error The NSError message, probably from the network routine.
 */
- (void) remoteTileInfo:(id __nonnull)tileInfo tileDidNotLoad:(MaplyTileID)tileID frame:(int)frame error:(NSError *__nonnull)error;

/** 
    Called when the tile is unloaded.
 
    Normally you won't get called when an image or vector tile is unloaded from memory.  If you set this, you will.
 
    You're not required to do anything, but you can clean up data of your own if you like.
 
    You will be called on another thread, so act accordingly.
 
    @param tileID The tile that that just got unloaded.
 */
- (void)remoteTileInfo:(id __nonnull)tileInfo tileUnloaded:(MaplyTileID)tileID frame:(int)frame;

@end


/** 
    The multiplex tile source is for bunging other tile sources together.
    
    The Multiplex Tile source takes a bunch of other tiles source
    objects and switches between them.  Strictly speaking, it doesn't
    multiplex.  The quad image layer asks for all of its images at
    once for a given tile.  I just like the word 'multiplex'.
    Multiplex. Multiplex.  So futuristic in a 1970's way.  Multiplex.
    
    Anyway, this is useful for animating between different tile
    sources.
    @see MaplyTileSource
    @see MaplyQuadImageTilesLayer
  */
@interface MaplyMultiplexTileSource : NSObject<MaplyTileSource>

/** 
    Initialize with an array of objects that conform to MaplyRemoteTileInfoProtocol
    
    To create one of these you need to pass in an NSArray of MaplyRemoteTileInfoProtocol objects.  Where this is useful is if you want to animate between them.  Check out the animation functionality in MaplyQuadImageTilesLayer.  To animate effectively, that layer must grab all the images for a tile at once, hence this object.
    
    A simple example would be taking 6 MaplyRemoteTileInfoProtocol objects pointing to MBTiles archives, sticking them in an array and passing them in here.  Each of them might represent a frame of animation in a video you want to show over the whole earth.  Because wow, man.  The whole earth.
    
    @return Returns a working MaplyMultiplexTileSource or nil if it can't.  That might happen if the input tile sources don't match up to each other.
  */
- (nullable instancetype)initWithSources:(NSArray *__nonnull)tileSources;

/** 
    Coordinate system for the tile source.
    
    The coordinate system is derived from the first tile source passed in.  They'd better all agree or the results will be odd.
  */
@property (nonatomic,readonly,nonnull) MaplyCoordinateSystem *coordSys;

/** 
    A delegate for tile loads and failures.
    
    If set, you'll get callbacks when the various tiles load (or don't). You get called in all sorts of threads.  Act accordingly.
 */
@property (nonatomic,weak,nullable) NSObject<MaplyMultiplexTileSourceDelegate> *delegate;

/** 
    If set, we'll let failures pass through.
    
    If you're fetching a very wide set of tiles, you may want to let a few failures happen and fill in the images yourself.
    
    To do that, the multiplex tile source needs to accept failures and store an NSNull in the appropriate entry.
  */
@property (nonatomic) bool acceptFailures;

/// If set, we'll track the outstanding connections across all remote tile sources
+ (void)setTrackConnections:(bool)track;

/// Number of outstanding connections across all tile sources
+ (int)numOutstandingConnections;

@end
