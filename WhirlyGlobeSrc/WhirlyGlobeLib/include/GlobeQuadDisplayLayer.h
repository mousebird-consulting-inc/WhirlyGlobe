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
    
    /// Remove data from scene.  This just sets up the changes requests.
    /// They must still be passed to the scene
    void clearContents(WhirlyGlobeQuadDisplayLayer *layer,GlobeScene *scene,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Update what we're displaying based on the quad tree, particulary for children
    void updateContents(WhirlyKit::Quadtree *tree,WhirlyGlobeQuadDisplayLayer *layer,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
        
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
    
/// Utility function to calculate importance based on pixel screen size.
/// This would be used by the data source as a default.
float ScreenImportance(WhirlyGlobeViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyeVec,int pixelsSqare,WhirlyKit::CoordSystem *coordSys,WhirlyKit::Mbr nodeMbr);

}

/** Quad tree based data source.  Fill in this protocol to provide
    image tiles on demand.
  */
@protocol WhirlyGlobeQuadDataSource <NSObject>

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

/// If there is an image, return it.  Null otherwise.
- (NSData *)fetchImageForLevel:(int)level col:(int)col row:(int)row;

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
    
    /// Tiles we currently have loaded in the scene
    WhirlyGlobe::LoadedTileSet tileSet;
    
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

    /// Data source for the tiles
    NSObject<WhirlyGlobeQuadDataSource> *dataSource;
    
    /// State of the view the last time we were called
    WhirlyGlobeViewState *viewState;
}

@property (nonatomic,readonly) WhirlyKit::CoordSystem *coordSys;
@property (nonatomic,readonly) WhirlyKit::Mbr mbr;
@property (nonatomic,assign) int maxTiles;
@property (nonatomic,assign) float minTileArea;
@property (nonatomic,assign) bool lineMode;
@property (nonatomic,assign) bool debugMode;
@property (nonatomic,assign) bool drawEmpty;
@property (nonatomic,assign) float viewUpdatePeriod;
@property (nonatomic,strong) NSObject<WhirlyGlobeQuadDataSource> *dataSource;

/// Construct with a renderer and data source for the tiles
- (id)initWithDataSource:(NSObject<WhirlyGlobeQuadDataSource> *)dataSource renderer:(WhirlyKitSceneRendererES1 *)renderer;

@end

