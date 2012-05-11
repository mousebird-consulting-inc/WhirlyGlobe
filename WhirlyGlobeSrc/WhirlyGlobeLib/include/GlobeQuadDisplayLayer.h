/*
 *  GlobeQuadDisplayLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
 *  Copyright 2011 mousebird consulting
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
#import "GlobeScene.h"
#import "DataLayer.h"
#import "RenderCache.h"
#import "LayerThread.h"
#import "GlobeMath.h"
#import "sqlhelpers.h"
#import "Quadtree.h"
#import "SceneRendererES1.h"

/// @cond
@class WhirlyGlobeQuadDisplayLayer;
/// @endcond

namespace WhirlyGlobe
{
/// Quad tree Nodeinfo structures sorted by importance
typedef std::set<WhirlyKit::Quadtree::NodeInfo> QuadNodeInfoSet;    

/// Utility function to calculate importance based on pixel screen size.
/// This would be used by the data source as a default.
float ScreenImportance(WhirlyGlobeViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyeVec,int pixelsSqare,WhirlyKit::CoordSystem *coordSys,WhirlyKit::Mbr nodeMbr);
}

/** Quad tree based data structure.  Fill this in to provide structure and
    extents for the quad tree.
 */
@protocol WhirlyGlobeQuadDataStructure <NSObject>

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
- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyGlobeViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize;

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown;

@end

/** Loader protocol for quad tree changes.  Fill this in to be
    notified when the quad layer is adding and removing tiles.
    Presumably you'll want to add or remove geometry as well.
 */
@protocol WhirlyGlobeQuadLoader <NSObject>

/// Called when the layer first starts up.  Keep this around if you need it.
- (void)setQuadLayer:(WhirlyGlobeQuadDisplayLayer *)layer;

/// The quad layer uses this to see if a loader is capable of loading
///  another tile.  Use this to track simultaneous loads
- (bool)isReady;

/// Called right before we start a series of updates
- (void)quadDisplayLayerStartUpdates:(WhirlyGlobeQuadDisplayLayer *)layer;

/// Called right after we finish a series of updates
- (void)quadDisplayLayerEndUpdates:(WhirlyGlobeQuadDisplayLayer *)layer;

/// The quad tree wants to load the given tile.
/// Call the layer back when the tile is loaded.
/// This is in the layer thread.
- (void)quadDisplayLayer:(WhirlyGlobeQuadDisplayLayer *)layer loadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo;

/// Quad tree wants to unload the given tile immediately.
/// This is in the layer thread.
- (void)quadDisplayLayer:(WhirlyGlobeQuadDisplayLayer *)layer unloadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo;

/// The layer is checking to see if it's allowed to traverse below the given tile.
/// If the loader is still trying to load that given tile (or has some other information about it),
///  then return false.  If the tile is loaded and the children may be valid, return true.
- (bool)quadDisplayLayer:(WhirlyGlobeQuadDisplayLayer *)layer canLoadChildrenOfTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo;

/// Called when the layer is about to shut down.  Clear out any drawables and caches.
- (void)shutdownLayer:(WhirlyGlobeQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene;

@end


/** This data layer displays image data organized in a quad tree.
    It will swap data in and out as required.
 */
@interface WhirlyGlobeQuadDisplayLayer : NSObject<WhirlyKitLayer,WhirlyKitQuadTreeImportanceDelegate>
{
    /// Layer thread we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    
    /// Scene we're modifying
	WhirlyGlobe::GlobeScene *scene;
    
    /// The renderer we need for frame sizes
    WhirlyKitSceneRendererES1 * __weak renderer;
        
    /// Coordinate system we're working in for tiling
    /// The visual output is on the globe
    WhirlyKit::CoordSystem *coordSys;
    
    /// Valid bounding box in local coordinates (coordSys)
    WhirlyKit::Mbr mbr;
    
    /// [minZoom,maxZoom] range
    int minZoom,maxZoom;
    
    /// Quad tree used for paging advice
    WhirlyKit::Quadtree *quadtree;
        
    /// Nodes being evaluated for loading
    WhirlyGlobe::QuadNodeInfoSet nodesForEval;

    /// Maximum number of tiles loaded in at once
    int maxTiles;
    
    /// Minimum screen area to consider for a pixel
    float minTileArea;
    
    /// How often this layer gets notified of view changes.  1s by default.
    float viewUpdatePeriod;
    
    /// If set, we'll draw the empty tiles as lines
    /// If not set, we'll just stop loading at that tile
    // Note: Note implemented
    bool drawEmpty;
    
    /// Draw lines instead of polygons, for demonstration.
    bool lineMode;
    
    /// If set, we print out way too much debugging info.
    bool debugMode;    

    /// State of the view the last time we were called
    WhirlyGlobeViewState *viewState;

    /// Data source for the quad tree structure
    NSObject<WhirlyGlobeQuadDataStructure> *dataStructure;

    /// Loader that may be creating and deleting data as the quad tiles load
    ///  and unload.
    NSObject<WhirlyGlobeQuadLoader> *loader; 
}

@property (nonatomic,weak,readonly) WhirlyKitLayerThread *layerThread;
@property (nonatomic,readonly) WhirlyGlobe::GlobeScene *scene;
@property (nonatomic,readonly) WhirlyKit::Quadtree *quadtree;
@property (nonatomic,readonly) WhirlyKit::CoordSystem *coordSys;
@property (nonatomic,readonly) WhirlyKit::Mbr mbr;
@property (nonatomic,assign) int maxTiles;
@property (nonatomic,assign) float minTileArea;
@property (nonatomic,assign) bool lineMode;
@property (nonatomic,assign) bool debugMode;
@property (nonatomic,assign) bool drawEmpty;
@property (nonatomic,assign) float viewUpdatePeriod;
@property (nonatomic,strong,readonly) NSObject<WhirlyGlobeQuadDataStructure> *dataStructure;
@property (nonatomic,strong,readonly) NSObject<WhirlyGlobeQuadLoader> *loader;

/// Construct with a renderer and data source for the tiles
- (id)initWithDataSource:(NSObject<WhirlyGlobeQuadDataStructure> *)dataSource loader:(NSObject<WhirlyGlobeQuadLoader> *)loader renderer:(WhirlyKitSceneRendererES1 *)renderer;

/// A loader calls this after successfully loading a tile.
/// Must be called in the layer thread.
- (void)loader:(NSObject<WhirlyGlobeQuadLoader> *)loader tileDidLoad:(WhirlyKit::Quadtree::Identifier)tileIdent;

/// Loader calls this after a failed tile load.
/// Must be called in the layer thread.
- (void)loader:(NSObject<WhirlyGlobeQuadLoader> *)loader tileDidNotLoad:(WhirlyKit::Quadtree::Identifier)tileIdent;

@end

