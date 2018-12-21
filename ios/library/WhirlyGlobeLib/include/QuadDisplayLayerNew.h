/*
 *  QuadDisplayLayerNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/26/18.
 *  Copyright 2011-2018 mousebird consulting
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
#import "SceneRendererES2.h"
#import "ScreenImportance.h"
#import "QuadDisplayLayer.h"
#import "QuadTreeNew.h"

@class WhirlyKitQuadDisplayLayerNew;

/** The Quad Display Layer (New) calls an object with this protocol.
    Display layer does the geometric logic.  Up to you to do something with it.
  */
@protocol WhirlyKitQuadLoaderNew <NSObject>

/// Called when the layer first starts up.  Keep this around if you need it.
- (void)setQuadLayer:(WhirlyKitQuadDisplayLayerNew * __nonnull)layer;

/// Load some tiles, unload others, and the rest had their importance values change
/// Return the nodes we wanted to keep rather than delete
- (WhirlyKit::QuadTreeNew::NodeSet)quadDisplayLayer:(WhirlyKitQuadDisplayLayerNew * __nonnull)layer
               loadTiles:(const WhirlyKit::QuadTreeNew::ImportantNodeSet &)tiles
             unLoadTiles:(const WhirlyKit::QuadTreeNew::NodeSet &)tiles
             updateTiles:(const WhirlyKit::QuadTreeNew::ImportantNodeSet &)updateTiles
             targetLevel:(int)targetLevel;

/// Called right before the layer thread flushes its change requests
- (void)quadDisplayLayerPreSceneFlush:(WhirlyKitQuadDisplayLayerNew * __nonnull)layer;

/// Called when a layer is shutting down (on the layer thread)
- (void)quadDisplayLayerShutdown:(WhirlyKitQuadDisplayLayerNew * __nonnull)layer;

@end

// Reusing WhirlyKitQuadDataStructure for the size of the quad tree

/** This layer turns view state updates into quad tree tiles to load.
  */
@interface WhirlyKitQuadDisplayLayerNew : NSObject<WhirlyKitLayer>

/// Layer thread we're attached to
@property (nonatomic,weak,readonly,nullable) WhirlyKitLayerThread *layerThread;
/// Scene we're modifying
@property (nonatomic,readonly,nullable) WhirlyKit::Scene *scene;
/// Quad tree used for paging advice
@property (nonatomic,readonly,nullable) WhirlyKit::QuadTreeNew *quadtree;
/// Coordinate system we're working in for tiling
@property (nonatomic,readonly,nullable) WhirlyKit::CoordSystem *coordSys;
/// Valid bounding box in local coordinates (coordSys)
@property (nonatomic,readonly) WhirlyKit::Mbr mbr;
/// Maximum number of tiles loaded in at once
@property (nonatomic,assign) int maxTiles;
/// Minimum screen area to consider for a pixel per level
@property (nonatomic,assign) std::vector<double> minImportancePerLevel;
/// How often this layer gets notified of view changes.  1s by default.
@property (nonatomic,assign) float viewUpdatePeriod;
/// Data source for the quad tree structure
@property (nonatomic,strong,nullable) NSObject<WhirlyKitQuadDataStructure> *dataStructure;
/// Loader responds to our requests to load and unload tiles
@property (nonatomic,strong,nullable) NSObject<WhirlyKitQuadLoaderNew> *loader;
/// The renderer we need for frame sizes
@property (nonatomic,weak,nullable) WhirlyKitSceneRendererES2 *renderer;
/// On by default.  If you turn this off we won't evaluate any view changes.
@property (nonatomic,assign) bool enable;
/// Load just the target level (and the lowest level)
@property (nonatomic,assign) bool singleLevel;
// Level offsets in single level mode
@property (nonatomic,nullable) NSArray<NSNumber *> *levelLoads;

/// Construct with a renderer and data source for the tiles
- (nonnull)initWithDataSource:(NSObject<WhirlyKitQuadDataStructure> * __nonnull)dataStructure loader:(NSObject<WhirlyKitQuadLoaderNew> * __nonnull)loader renderer:(WhirlyKitSceneRendererES * __nonnull)renderer;

@end
