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

#import "QuadDisplayControllerNew.h"
#import "LoadedTileNew.h"

namespace WhirlyKit
{

class QuadTileBuilder;
    
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
class QuadTileBuilderDelegate : public Identifiable
{
public:
    QuadTileBuilderDelegate();
    virtual ~QuadTileBuilderDelegate();
    
    /// Called when the builder first starts up.  Keep this around if you need it.
    virtual void setBuilder(QuadTileBuilder *builder,QuadDisplayControllerNew *control) = 0;
    
    /// Before we tell the delegate to unload tiles, see if they want to keep them around
    /// Returns the tiles we want to preserve after all
    virtual QuadTreeNew::NodeSet builderUnloadCheck(QuadTileBuilder *builder,
                                                  const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
                                                  const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
                                                  int targetLevel) = 0;
    
    /// Load the given group of tiles.  If you don't load them immediately, up to you to cancel any requests
    virtual void builderLoad(PlatformThreadInfo *threadInfo,
                             QuadTileBuilder *builder,
                           const WhirlyKit::TileBuilderDelegateInfo &updates,
                           ChangeSet &changes) = 0;
    
    /// Called right before the layer thread flushes all its current changes
    virtual void builderPreSceneFlush(QuadTileBuilder *builder,ChangeSet &changes) = 0;

    /// Shutdown called on the layer thread if you stuff to clean up
    virtual void builderShutdown(PlatformThreadInfo *threadInfo,QuadTileBuilder *builder,ChangeSet &changes) = 0;
    
    /// Simple status check.  Is this builder in the process of loading something?
    virtual bool builderIsLoading() = 0;
};
    
typedef std::shared_ptr<QuadTileBuilderDelegate> QuadTileBuilderDelegateRef;

/** The Quad Tile Builder generates geometric tiles based on
 a quad tree and coordinates an image builder on top of those.
 */
class QuadTileBuilder : public QuadLoaderNew
{
public:
    QuadTileBuilder(CoordSystemRef coordSys,QuadTileBuilderDelegate *delegate);
    virtual ~QuadTileBuilder();
    
    // Return a tile, if there is one
    LoadedTileNewRef getLoadedTile(const QuadTreeNew::Node &ident);
    
    // Return all the tiles that should be loaded
    TileBuilderDelegateInfo getLoadingState();

    // Coordinate system we're building the tiles in
    CoordSystemRef getCoordSystem();
    
    // If set, we'll actually build geometry for the drawables
    // The generic quad paging case doesn't use this
    void setBuildGeom(bool);
    bool getBuildGeom() const;

    // If set, we'll cover the poles of a curved coordinate system (e.g. spherical mercator)
    void setCoverPoles(bool);
    bool getCoverPoles() const;

    // If set, we'll generate skirts to match between levels of detail
    void setEdgeMatching(bool);
    bool getEdgeMatching() const;

    // Set the draw priority values for produced tiles
    void setBaseDrawPriority(int);
    int getBaseDrawPriority() const;

    // Offset between levels for a calculated draw priority
    void setDrawPriorityPerLevel(int);
    int getDrawPriorityPerLevel() const;
    
    // Set if we're using single level loading logic
    void setSingleLevel(bool);
    bool getSingleLevel() const;
    
    // Set the color for the underlying geometry
    void setColor(const RGBAColor &color);
    const RGBAColor &getColor() const;
    
    // Shader ID used to render the geometry
    void setShaderID(SimpleIdentity programID);
    SimpleIdentity getProgramID() const;
    
    // If set, we'll print too much information
    void setDebugMode(bool);
    bool getDebugMode() const;

protected:
    /// Called when the layer first starts up.  Keep this around if you need it.
    virtual void setController(QuadDisplayControllerNew *inControl);
    
    /// Load some tiles, unload others, and the rest had their importance values change
    /// Return the nodes we wanted to keep rather than delete
    virtual QuadTreeNew::NodeSet quadLoaderUpdate(PlatformThreadInfo *threadInfo,
                                                  const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
                                                  const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
                                                  const WhirlyKit::QuadTreeNew::ImportantNodeSet &updateTiles,
                                                  int targetLevel,
                                                  ChangeSet &changes);
    
    /// Called right before the layer thread flushes its change requests
    virtual void quadLoaderPreSceenFlush(ChangeSet &changes);
    
    /// Called when a layer is shutting down (on the layer thread)
    virtual void quadLoaderShutdown(PlatformThreadInfo *threadInfo,ChangeSet &changes);
    
    bool debugMode;
    
    TileGeomManager geomManage;
    TileGeomSettings geomSettings;
    
    QuadTileBuilderDelegate *delegate;
};
    
typedef std::shared_ptr<QuadTileBuilder> QuadTileBuilderRef;
    
}

