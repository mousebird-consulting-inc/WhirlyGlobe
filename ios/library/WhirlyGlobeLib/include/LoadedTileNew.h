/*
 *  LoadedTileNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/18.
 *  Copyright 2011-2018 Saildrone Inc.
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
#import "QuadTreeNew.h"
#import "SceneRendererES.h"

namespace WhirlyKit
{

/* Geometry settings passed to the tile generation method.
  */
class TileGeomSettings
{
public:
    TileGeomSettings();
    
    // Whether the geometry is centered in its middle with an offset
    bool useTileCenters;
    // Base color for the tiles
    RGBAColor color;
    // Shader to to use in rendering
    SimpleIdentity programID;
    // Samples to generate in X and Y
    int sampleX,sampleY;
    // If set, viewable range
    double minVis,maxVis;
    // The priority for the drawables
    int drawPriority;
    // If set, we'll just build lines for debugging
    bool lineMode;
    // If set, we'll include the elevation data
    bool includeElev;
};

class TileGeomManager;

/* Wraps a single tile that we've loaded into memory.
  */
class LoadedTileNew
{
public:
    LoadedTileNew(QuadTreeNew::Node &ident);
    
    // Build the drawable(s) to represent this one tile
    void makeDrawables(TileGeomManager *geomManage,TileGeomSettings &geomSettings,ChangeSet &changes);

    // Utility routine to build skirts around the edges
    void buildSkirt(BasicDrawable *draw,std::vector<Point3d> &pts,std::vector<TexCoord> &texCoords,double skirtFactor,bool haveElev,const Point3d &theCenter);

    // Enable associated drawables
    void enableDrawables(ChangeSet &changes);

    // Disable associated drawables
    void disableDrawables(ChangeSet &changes);

    // Generate commands to remove the associated drawables
    void removeDrawables(ChangeSet &changes);

    QuadTreeNew::Node ident;
    // Active drawable IDs associated with this tile
    SimpleIDSet drawIDs;
};
typedef std::shared_ptr<LoadedTileNew> LoadedTileNewRef;

/** Tile Builder builds individual tile geometry for use elsewhere.
  */
class TileGeomManager
{
public:
    TileGeomManager();
    
    void setup(QuadTreeNew *quadTree,CoordSystemDisplayAdapter *coordAdapter,CoordSystem *coordSys,MbrD inMbr);
    
    // Add the tiles list in the node set
    void addTiles(TileGeomSettings &geomSettings,const QuadTreeNew::NodeSet &tiles,ChangeSet &changes);
    
    // Remove the tiles given, if they're being represented
    void removeTiles(const QuadTreeNew::NodeSet &tiles,ChangeSet &changes);
    
    QuadTreeNew *quadTree;
    CoordSystemDisplayAdapter *coordAdapter;
    
    // Coordinate system of the tiles (different from the scene)
    CoordSystem *coordSys;
    
    // Build geometry to the poles
    bool coverPoles;
    
    // Color overrides for poles, if present
    bool useNorthPoleColor,useSouthPoleColor;
    RGBAColor northPoleColor,southPoleColor;

    // Build the skirts for edge matching
    bool buildSkirts;
    
    // Bounding box of the whole area
    MbrD mbr;
    
    std::map<QuadTreeNew::Node,LoadedTileNewRef> tileMap;
};

}
