/*
 *  QuadTileBuilder.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/29/18.
 *  Copyright 2012-2018 Saildrone Inc
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

#import "QuadDisplayLayerNew.h"
#import "LoadedTileNew.h"

@class WhirlyKitQuadTileBuilder;

namespace WhirlyKit
{
    
/**
    Tile Builder Delegate Info
 
    This is passed to a Tile Builder Delegate when changes are being made in the Tile Builder;
  */
class TileBuilderDelegateInfo {
public:
    int targetLevel;
    LoadedTileVec loadTiles;
    QuadTreeNew::NodeSet unloadTiles;
    LoadedTileVec enableTiles,disableTiles;
    QuadTreeNew::ImportantNodeSet changeTiles;
};
    
}

/// Protocol used by the tile builder to notify an interested party about what's
///  loaded.  If, for example, you want to attach textures to things.
@protocol WhirlyKitQuadTileBuilderDelegate<NSObject>

/// Called when the builder first starts up.  Keep this around if you need it.
- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)builder layer:(WhirlyKitQuadDisplayLayerNew * __nonnull)layer;

/// Before we tell the delegate to unload tiles, see if they want to keep them around
/// Returns the tiles we want to preserve after all
- (WhirlyKit::QuadTreeNew::NodeSet)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder
   loadTiles:(const WhirlyKit::QuadTreeNew::ImportantNodeSet &)loadTiles
    unloadTilesToCheck:(const WhirlyKit::QuadTreeNew::NodeSet &)unloadTiles
    targetLevel:(int)targetLevel;

/// Load the given group of tiles.  If you don't load them immediately, up to you to cancel any requests
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder
             update:(const WhirlyKit::TileBuilderDelegateInfo &)updates
            changes:(WhirlyKit::ChangeSet &)changes;

/// Called right before the layer thread flushes all its current changes
- (void)quadBuilderPreSceneFlush:(WhirlyKitQuadTileBuilder *__nonnull )builder;

/// Shutdown called on the layer thread if you stuff to clean up
- (void)quadBuilderShutdown:(WhirlyKitQuadTileBuilder *__nonnull )builder;

@end

/** The Quad Tile Builder generates geometric tiles based on
     a quad tree and coordinates an image builder on top of those.
  */
@interface WhirlyKitQuadTileBuilder : NSObject<WhirlyKitQuadLoaderNew>

// Coordinate system we're building the tiles in
@property (nonatomic) WhirlyKit::CoordSystem * __nonnull coordSys;

// If set, we'll cover the poles of a curved coordinate system (e.g. spherical mercator)
@property (nonatomic) bool coverPoles;

// If set, we'll generate skirts to match between levels of detail
@property (nonatomic) bool edgeMatching;

// Set the draw priority values for produced tiles
@property (nonatomic) int baseDrawPriority;

// Offset between levels for a calculated draw priority
@property (nonatomic) int drawPriorityPerLevel;

// Set if we're using single level loading logic
@property (nonatomic) bool singleLevel;

// If set, we'll print too much information
@property (nonatomic) bool debugMode;

// If set, we'll notify the delegate when tiles are loaded and unloaded
@property (nonatomic) NSObject<WhirlyKitQuadTileBuilderDelegate> * __nullable delegate;

- (id __nullable)initWithCoordSys:(WhirlyKit::CoordSystem * __nonnull )coordSys;

// Return a tile, if there is one
- (WhirlyKit::LoadedTileNewRef)getLoadedTile:(WhirlyKit::QuadTreeNew::Node)ident;

// Return all the tiles that should be loaded
- (WhirlyKit::TileBuilderDelegateInfo)getLoadingState;

@end
