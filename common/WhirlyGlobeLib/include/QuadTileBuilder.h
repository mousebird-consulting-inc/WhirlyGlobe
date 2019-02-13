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

namespace WhirlyKit
{

class QuadTileBuilder;
    
/** Quad tree based data structure.  Fill this in to provide structure and
 extents for the quad tree.
 */
class QuadDataStructure
{
public:
    QuadDataStructure();
    virtual ~QuadDataStructure();
    
    /// Return the coordinate system we're working in
    virtual CoordSystem *quadDataGetCoordSystem() = 0;
    
    /// Bounding box used to calculate quad tree nodes.  In local coordinate system.
    virtual Mbr quadDataGetTotalExtents() = 0;
    
    /// Bounding box of data you actually want to display.  In local coordinate system.
    /// Unless you're being clever, make this the same as totalExtents.
    virtual Mbr quadDataGetValidExtents() = 0;
    
    /// Return the minimum quad tree zoom level (usually 0)
    virtual int quadDataGetMinZoom() = 0;
    
    /// Return the maximum quad tree zoom level.  Must be at least minZoom
    virtual int quadDataGetMaxZoom() = 0;
    
    /// Return an importance value for the given tile
    virtual double quadDataImportanceForTile(const QuadTreeIdentifier &ident,
                                             const Mbr &mbr,
                                             ViewStateRef viewState,
                                             const Point2f &frameSize) = 0;
    
    /// Called when the view state changes.  If you're caching info, do it here.
    virtual void quadDataNewViewState(ViewStateRef viewState) = 0;
    
    /// Return true if the tile is visible, false otherwise
    virtual bool quadDataVisibilityForTile(const QuadTreeIdentifier &ident,
                                           const Mbr &mbr,
                                           ViewStateRef viewState,
                                           const Point2f &frameSize) = 0;
};

/** Quad Display
 */
class QuadDisplayInfo
{
public:
    /// Scene we're modifying
    Scene *scene;
    /// Quad tree used for paging advice
    QuadTreeNew *quadtree;
    /// Coordinate system we're working in for tiling
    CoordSystem *coordSys;
    /// Valid bounding box in local coordinates (coordSys)
    Mbr mbr;
    /// Maximum number of tiles loaded in at once
    int maxTiles;
    /// Minimum screen area to consider for a pixel per level
    std::vector<double> minImportancePerLevel;
    /// How often this layer gets notified of view changes.  1s by default.
    float viewUpdatePeriod;
    /// Load just the target level (and the lowest level)
    bool singleLevel;
    // Level offsets in single level mode
    std::vector<int> levelLoads;
    
    /// The renderer we need for frame sizes
    SceneRendererES2 *renderer;
};

/** The Quad Display Layer (New) calls an object with this protocol.
 Display layer does the geometric logic.  Up to you to do something with it.
 */
class QuadLoaderNew
{
public:
    QuadLoaderNew(QuadDisplayInfo *info) { displayInfo = info; }
    virtual ~QuadLoaderNew() { }
    
    /// Called when the layer first starts up.  Keep this around if you need it.
    virtual void setQuadDisplayInfo(QuadDisplayInfo *info) { displayInfo = info; }
    
    /// Load some tiles, unload others, and the rest had their importance values change
    /// Return the nodes we wanted to keep rather than delete
    virtual QuadTreeNew::NodeSet quadLoaderUpdate(const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
                                                  const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
                                                  const WhirlyKit::QuadTreeNew::ImportantNodeSet &updateTiles,
                                                  int targetLevel) = 0;
    
    /// Called right before the layer thread flushes its change requests
    virtual void quadLoaderPreSceenFlush() = 0;
    
    
    /// Called when a layer is shutting down (on the layer thread)
    virtual void quadLoaderShutdown() = 0;
    
protected:
    QuadDisplayInfo *displayInfo;
};

    
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

/// Protocol used by the tile builder to notify an interested party about what's
///  loaded.  If, for example, you want to attach textures to things.
class QuadTileBuilderDelegate
{
public:
    QuadTileBuilderDelegate();
    virtual ~QuadTileBuilderDelegate();
    
    /// Called when the builder first starts up.  Keep this around if you need it.
    virtual void builderSet(QuadTileBuilder *builder) = 0;
    
    /// Before we tell the delegate to unload tiles, see if they want to keep them around
    /// Returns the tiles we want to preserve after all
    virtual QuadTreeNew::NodeSet builderUnloadCheck(QuadTileBuilder *builder,
                                                  const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
                                                  const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
                                                  int targetLevel) = 0;
    
    /// Load the given group of tiles.  If you don't load them immediately, up to you to cancel any requests
    virtual void builderLoad(QuadTileBuilder *builder,
                           const WhirlyKit::TileBuilderDelegateInfo &updates,
                           ChangeSet &changes) = 0;
    
    /// Called right before the layer thread flushes all its current changes
    virtual void builderPreSceneFlush(QuadTileBuilder *builder) = 0;

    /// Shutdown called on the layer thread if you stuff to clean up
    virtual void builderShutdown(QuadTileBuilder *builder) = 0;
}

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
