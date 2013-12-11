/*
 *  QuadDisplayLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
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

#import <Foundation/Foundation.h>
#import <math.h>
#import "WhirlyVector.h"
#import "TextureGroup.h"
#import "Scene.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "GlobeMath.h"
#import "sqlhelpers.h"
#import "Quadtree.h"
#import "SceneRendererES.h"
#import "ScreenImportance.h"

/// @cond
@class WhirlyKitQuadDisplayLayer;
/// @endcond

namespace WhirlyKit
{
/// Quad tree Nodeinfo structures sorted by importance
typedef std::set<WhirlyKit::Quadtree::NodeInfo> QuadNodeInfoSet;
}

/** Quad tree based data structure.  Fill this in to provide structure and
    extents for the quad tree.
 */
@protocol WhirlyKitQuadDataStructure <NSObject>

/// Return the coordinate system we're working in
- (WhirlyKit::CoordSystem *)coordSystem;

/// Bounding box used to calculate quad tree nodes.  In local coordinate system.
- (WhirlyKit::Mbr)totalExtents;

/// Bounding box of data you actually want to display.  In local coordinate system.
/// Unless you're being clever, make this the same as totalExtents.
- (WhirlyKit::Mbr)validExtents;

/// Return the minimum quad tree zoom level (usually 0)
- (int)minZoom;

/// Return the maximum quad tree zoom level.  Must be at least minZoom
- (int)maxZoom;

/// Return an importance value for the given tile
- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *) viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs;

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown;

@optional

/// Called when the view state changes.  If you're caching info, do it here.
- (void)newViewState:(WhirlyKitViewState *)viewState;

@end

/** Loader protocol for quad tree changes.  Fill this in to be
    notified when the quad layer is adding and removing tiles.
    Presumably you'll want to add or remove geometry as well.
 */
@protocol WhirlyKitQuadLoader <NSObject>

/// Called when the layer first starts up.  Keep this around if you need it.
- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer;

/// The quad layer uses this to see if a loader is capable of loading
///  another tile.  Use this to track simultaneous loads
- (bool)isReady;

/// Called right before we start a series of updates
- (void)quadDisplayLayerStartUpdates:(WhirlyKitQuadDisplayLayer *)layer;

/// Called right after we finish a series of updates
- (void)quadDisplayLayerEndUpdates:(WhirlyKitQuadDisplayLayer *)layer;

/// The quad tree wants to load the given tile.
/// Call the layer back when the tile is loaded.
/// This is in the layer thread.
- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer loadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo;

/// Quad tree wants to unload the given tile immediately.
/// This is in the layer thread.
- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer unloadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo;

/// The layer is checking to see if it's allowed to traverse below the given tile.
/// If the loader is still trying to load that given tile (or has some other information about it),
///  then return false.  If the tile is loaded and the children may be valid, return true.
- (bool)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer canLoadChildrenOfTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo;

/// Called when the layer is about to shut down.  Clear out any drawables and caches.
- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene;

@optional
/// Called right before the view update to determine if we should even be paging
/// You can use this to temporarily suspend paging.
/// isInitial is set if this is the first time through
- (bool)shouldUpdate:(WhirlyKitViewState *)viewState initial:(bool)isInitial;

/// Normally we'd call an endUpdates, but if we're holding that open for a while
/// (e.g. matching frame boundaries), let's at least get all the work done.
- (void)updateWithoutFlush;

/// Number of network fetches outstanding.  Used by the pager for optimization.
- (int)networkFetches;

/// Number of local fetches outstanding.  Used by the pager for optimizaiton.
- (int)localFetches;

/// Dump some log info out to the console
- (void)log;

@end


/** This data layer displays image data organized in a quad tree.
    It will swap data in and out as required.
 */
@interface WhirlyKitQuadDisplayLayer : NSObject<WhirlyKitLayer,WhirlyKitQuadTreeImportanceDelegate>

/// Layer thread we're attached to
@property (nonatomic,weak,readonly) WhirlyKitLayerThread *layerThread;
/// Scene we're modifying
@property (nonatomic,readonly) WhirlyKit::Scene *scene;
/// Quad tree used for paging advice
@property (nonatomic,readonly) WhirlyKit::Quadtree *quadtree;
/// Coordinate system we're working in for tiling
@property (nonatomic,readonly) WhirlyKit::CoordSystem *coordSys;
/// Valid bounding box in local coordinates (coordSys)
@property (nonatomic,readonly) WhirlyKit::Mbr mbr;
/// Maximum number of tiles loaded in at once
@property (nonatomic,assign) int maxTiles;
/// Minimum screen area to consider for a pixel
@property (nonatomic,assign) float minImportance;
/// Draw lines instead of polygons, for demonstration.
@property (nonatomic,assign) bool lineMode;
/// If set, we print out way too much debugging info.
@property (nonatomic,assign) bool debugMode;
/// If set, we'll draw the empty tiles as lines
/// If not set, we'll just stop loading at that tile
// Note: Note unimplemented
@property (nonatomic,assign) bool drawEmpty;
/// How often this layer gets notified of view changes.  1s by default.
@property (nonatomic,assign) float viewUpdatePeriod;
/// How far the viewer has to move to force an update (if non-zero)
@property (nonatomic,assign) float minUpdateDist;
/// Data source for the quad tree structure
@property (nonatomic,strong,readonly) NSObject<WhirlyKitQuadDataStructure> *dataStructure;
/// Loader that may be creating and deleting data as the quad tiles load
///  and unload.
@property (nonatomic,strong,readonly) NSObject<WhirlyKitQuadLoader> *loader;
/// The renderer we need for frame sizes
@property (nonatomic,weak) WhirlyKitSceneRendererES *renderer;
/// If set we'll try to match the frame boundaries for our update
@property (nonatomic,assign) bool meteredMode;
/// If set, we'll try to completely load everything (local, at least) before switching
@property (nonatomic,assign) bool fullLoad;
/// If fullLoad is on, we need a timeout.  Otherwise changes just pile up until we run out of memory
@property (nonatomic,assign) NSTimeInterval fullLoadTimeout;

/// Construct with a renderer and data source for the tiles
- (id)initWithDataSource:(NSObject<WhirlyKitQuadDataStructure> *)dataSource loader:(NSObject<WhirlyKitQuadLoader> *)loader renderer:(WhirlyKitSceneRendererES *)renderer;

/// A loader calls this after successfully loading a tile.
/// Must be called in the layer thread.
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidLoad:(WhirlyKit::Quadtree::Identifier)tileIdent;

/// Loader calls this after a failed tile load.
/// Must be called in the layer thread.
- (void)loader:(NSObject<WhirlyKitQuadLoader> *)loader tileDidNotLoad:(WhirlyKit::Quadtree::Identifier)tileIdent;

/// Call this to force a reload for all existing tiles
- (void)refresh;

/// Call this to nudge the quad display layer awake.
- (void)wakeUp;

@end

