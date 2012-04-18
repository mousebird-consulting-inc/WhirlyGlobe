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
class LocalSizeCalculator;

/** The Loaded Tile is used to track tiles that have been
    loaded in to memory, but may be in various states.  It's also
    used to fill in child outlines that may be missing.
 */
class LoadedTile
{
public:
    LoadedTile();
    ~LoadedTile() { }
    
    /// Build the data needed for a scene representation
    void addToScene(WhirlyGlobeQuadDisplayLayer *layer,GlobeScene *scene);
    
    /// Remove data from scene
    void clearContents(WhirlyGlobeQuadDisplayLayer *layer,GlobeScene *scene);
    
    /// Update what we're displaying based on the quad tree, particulary for children
    void updateContents(WhirlyKit::Quadtree *tree,WhirlyGlobeQuadDisplayLayer *layer);
    
    /// Dump out to the log
    void Print(WhirlyKit::Quadtree *tree);
    
    WhirlyKit::Quadtree::NodeInfo nodeInfo;
    bool isOn;
    WhirlyKit::SimpleIdentity drawId;
    bool childIsOn[4];
    WhirlyKit::SimpleIdentity childDrawIds[4];
    WhirlyKit::SimpleIdentity texId;
};
    
/// This is a comparison operator for sorting loaded tile pointers by
/// Quadtree node identifier.
typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const LoadedTile *a,const LoadedTile *b)
    {
        return a->nodeInfo.ident < b->nodeInfo.ident;
    }
} LoadedTileSorter;

/// A set that sorts loaded MB Tiles by Quad tree identifier
typedef std::set<LoadedTile *,LoadedTileSorter> LoadedTileSet;
    
/// Quad tree Nodeinfo structures sorted by importance
typedef std::set<WhirlyKit::Quadtree::NodeInfo> QuadNodeInfoSet;
}

/** Quad tree based data source.  Fill in this protocol to provide
    image tiles on demand.
  */
@protocol WhirlyGlobeQuadDataSource <NSObject>

/// Return the bounding box, geographic only
- (WhirlyKit::GeoMbr)geoExtents;

/// Return the minimum zoom level (usually 0)
- (int)minZoom;

/// Return the maximum zoom level.  Must be at least minZoom
- (int)maxZoom;

/// The number of pixels per tiles, square.  This is used for estimation
- (int)pixelsPerTile;

/// Scale factor for a given tile's importance.  Used to mess with things.
- (float)importanceScaleForTile:(WhirlyKit::Quadtree::Identifier)ident;

/// If there is an image, return it.  Null otherwise.
- (NSData *)fetchImageForLevel:(int)level col:(int)col row:(int)row;

@end

/** This data layer displays image data organized in a quad tree.
    It will swap data in and out as required.
 */
@interface WhirlyGlobeQuadDisplayLayer : NSObject<WhirlyKitLayer>
{
    /// Layer thread we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    
    /// Scene we're modifying
	WhirlyGlobe::GlobeScene *scene;
    
    /// The renderer we need for frame sizes
    WhirlyKitSceneRendererES1 * __weak renderer;
    
    /// Used to calculate how big a given tile is on screen
    WhirlyGlobe::LocalSizeCalculator *sizeCalc;
    
    /// Geographic bounding box
    WhirlyKit::GeoMbr geoMbr;
    
    /// [minZoom,maxZoom] range
    int minZoom,maxZoom;
    
    /// Quad tree used for paging advice
    WhirlyKit::Quadtree *quadtree;
    
    /// Nodes being evaluated for loading
    WhirlyGlobe::QuadNodeInfoSet nodesForEval;
    
    /// Tiles we currently have loaded in the scene
    WhirlyGlobe::LoadedTileSet tileSet;
    
    /// Maximum number of tiles loaded in at once
    int maxTiles;
    
    /// Minimum screen area to consider for a pixel
    float minTileArea;
    
    /// How often this layer gets notified of view changes.  1s by default.
    float viewUpdatePeriod;
    
    /// Draw lines instead of polygons, for demonstration.
    bool lineMode;
    
    /// If set, we print out way too much debugging info.
    bool debugMode;    

    /// Data source for the tiles
    NSObject<WhirlyGlobeQuadDataSource> *dataSource;
    
    WhirlyGlobe::GlobeCoordSystem geoSystem;
}

@property (nonatomic,assign) int maxTiles;
@property (nonatomic,assign) float minTileArea;
@property (nonatomic,assign) bool lineMode;
@property (nonatomic,assign) bool debugMode;
@property (nonatomic,strong) NSObject<WhirlyGlobeQuadDataSource> *dataSource;

/// Construct with a renderer and data source for the tiles
- (id)initWithDataSource:(NSObject<WhirlyGlobeQuadDataSource> *)dataSource renderer:(WhirlyKitSceneRendererES1 *)renderer;

@end

