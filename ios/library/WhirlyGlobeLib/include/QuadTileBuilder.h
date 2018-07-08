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

/// Protocol used by the tile builder to notify an interested party about what's
///  loaded.  If, for example, you want to attach textures to things.
@protocol WhirlyKitQuadTileBuilderDelegate<NSObject>

/// Called when the builder first starts up.  Keep this around if you need it.
- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)builder layer:(WhirlyKitQuadDisplayLayerNew * __nonnull)layer;

/// Load the given group of tiles.  If you don't load them immediately, up to you to cancel any requests
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder loadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes;

/// The given tiles should be enabled
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder enableTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes;

/// The given tiles should be disabled
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder disableTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes;

/// Unload the given tiles.
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder unLoadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes;

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

// If set, we'll print too much information
@property (nonatomic) bool debugMode;

// If set, we'll notify the delegate when tiles are loaded and unloaded
@property (nonatomic) NSObject<WhirlyKitQuadTileBuilderDelegate> * __nullable delegate;

- (id __nullable)initWithCoordSys:(WhirlyKit::CoordSystem * __nonnull )coordSys;

// Return a tile, if there is one
- (WhirlyKit::LoadedTileNewRef)getLoadedTile:(WhirlyKit::QuadTreeNew::Node)ident;

@end
