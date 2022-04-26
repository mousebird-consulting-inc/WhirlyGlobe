/*
 *  LoadedTileNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/18.
 *  Copyright 2011-2019 Saildrone Inc.
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

#import <math.h>
#import "WhirlyVector.h"
#import "Scene.h"
#import "GlobeMath.h"
#import "QuadTreeNew.h"
#import "SceneRenderer.h"

namespace WhirlyKit
{

/* Geometry settings passed to the tile generation method.
  */
struct TileGeomSettings
{
    // Set if we actually build geometry, rather than just track space
    bool buildGeom = true;
    
    // Whether the geometry is centered in its middle with an offset
    bool useTileCenters = true;
    // Base color for the tiles
    RGBAColor color = RGBAColor::white();
    // Shader to to use in rendering
    SimpleIdentity programID = EmptyIdentity;
    // Samples to generate in X and Y
    int sampleX = 10;
    int sampleY = 10;
    // Samples for the top level node
    int topSampleX = 10;
    int topSampleY = 10;
    // If set, viewable range
    double minVis = DrawVisibleInvalid;
    double maxVis = DrawVisibleInvalid;
    // The priority for the drawables
    int baseDrawPriority = 0;
    // Multiply the level by this and add it to the baseDrawPriority
    int drawPriorityPerLevel = 1;
    // If set, we'll just build lines for debugging
    bool lineMode = false;
    // If set, we'll include the elevation data
    bool includeElev = false;
    // If set, we'll enable/disable geometry associated with tiles.
    // Otherwise we'll just always leave it off, assuming someone else is instancing it
    bool enableGeom = true;
    // If set, we're building single level geometry, so no parent logic
    bool singleLevel = false;
};

struct TileGeomManager;

/* Wraps a single tile that we've loaded into memory.
  */
class LoadedTileNew
{
public:
    LoadedTileNew(const QuadTreeNew::ImportantNode &ident,const MbrD &mbr);
    
    // Make sure the tile exists in space
    bool isValidSpatial(TileGeomManager *geomManage) const;
    
    // Build the drawable(s) to represent this one tile
    void makeDrawables(SceneRenderer *sceneRender,TileGeomManager *geomManage,
                       const TileGeomSettings &geomSettings,ChangeSet &changes);

    // Utility routine to build skirts around the edges
    static void buildSkirt(const BasicDrawableBuilderRef &draw,const Point3dVector &pts,
                           const std::vector<TexCoord> &texCoords,double skirtFactor,
                           bool haveElev,const Point3d &theCenter);

    // Enable associated drawables
    void enable(const TileGeomSettings &geomSettings,ChangeSet &changes);

    // Disable associated drawables
    void disable(const TileGeomSettings &geomSettings,ChangeSet &changes);

    // Generate commands to remove the associated drawables
    void removeDrawables(ChangeSet &changes);
    
    // Information about a particular drawable that's useful for instancing it.
    typedef enum { DrawableGeom, DrawableSkirt, DrawablePole } DrawableKind;
    struct DrawableInfo
    {
        DrawableInfo(DrawableKind kind,SimpleIdentity drawID,int drawPriority, int64_t drawOrder)
            : kind(kind), drawID(drawID), drawPriority(drawPriority), drawOrder(drawOrder)
        { }
        DrawableKind kind;      // What this is.  Main geometry or edge.
        SimpleIdentity drawID;  // ID corresponding to the drawable created
        int drawPriority;       // Draw priority we gave it
        int64_t drawOrder;
    };
    bool enabled = false;
    QuadTreeNew::ImportantNode ident;
    MbrD mbr;
    std::vector<DrawableInfo> drawInfo;
    int64_t tileNumber;
    // The Draw Priority as set when created
    int drawPriority = 0;
};
typedef std::shared_ptr<LoadedTileNew> LoadedTileNewRef;
typedef std::vector<LoadedTileNewRef> LoadedTileVec;

/** Tile Builder builds individual tile geometry for use elsewhere.
    This is just the geometry.  If you want textures on it, you need to do those elsewhere.
  */
struct TileGeomManager
{
    TileGeomManager() = default;
    
    // Construct with the quad tree we're building off of, the coordinate system we're building from and the (valid) bounding box
    void setup(SceneRenderer *sceneRender,
               TileGeomSettings &geomSettings,
               QuadTreeNew *quadTree,
               CoordSystemDisplayAdapter *coordAdapter,
               CoordSystemRef coordSys,
               const MbrD &inMbr);
    
    // Keep track of nodes added, enabled and disabled
    struct NodeChanges
    {
        LoadedTileVec addedTiles;
        LoadedTileVec enabledTiles;
        LoadedTileVec disabledTiles;
    };
    
    // Add the tiles list in the node set
    NodeChanges addRemoveTiles(const QuadTreeNew::ImportantNodeSet &addTiles,const QuadTreeNew::NodeSet &removeTiles,ChangeSet &changes);
    
    // Return a list of tiles corresponding to the IDs
    std::vector<LoadedTileNewRef> getTiles(const QuadTreeNew::NodeSet &tiles);
    
    // Return a single node
    LoadedTileNewRef getTile(const QuadTreeNew::Node &ident);
    
    // Return all the nodes we have geometry for
    LoadedTileVec getAllTiles();
    
    // Remove the tiles given, if they're being represented
    //NodeChanges removeTiles(const QuadTreeNew::NodeSet &tiles,ChangeSet &changes);
    
    // Turn tiles on/off based on their children
    void updateParents(ChangeSet &changes,LoadedTileVec &enabledNodes,LoadedTileVec &disabledNodes);
    
    // Remove all the various geometry
    void cleanup(ChangeSet &changes);

protected:
    TileGeomSettings settings;

    SceneRenderer *sceneRender = nullptr;

public:
    QuadTreeNew *quadTree = nullptr;
    CoordSystemDisplayAdapter *coordAdapter = nullptr;

    // Coordinate system of the tiles (different from the scene)
    CoordSystemRef coordSys;
    
    // Build geometry to the poles
    bool coverPoles = false;
    
    // Color overrides for poles, if present
    bool useNorthPoleColor = false;
    bool useSouthPoleColor = false;
    RGBAColor northPoleColor = RGBAColor::white();
    RGBAColor southPoleColor = RGBAColor::white();

    // Build the skirts for edge matching
    bool buildSkirts = false;

    // Bounding box of the whole area
    MbrD mbr;
    
protected:
    std::map<QuadTreeNew::Node,LoadedTileNewRef> tileMap;
};

}
