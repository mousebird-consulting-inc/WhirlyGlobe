/*
 *  QuadDisplayControllerNew.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/13/19.
 *  Copyright 2011-2019 mousebird consulting
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
#import "SceneRenderer.h"
#import "ScreenImportance.h"
#import "WhirlyKitView.h"
#import "QuadTreeNew.h"

namespace WhirlyKit
{
class QuadDisplayControllerNew;

/** Quad tree based data structure.  Fill this in to provide structure and
 extents for the quad tree.
 */
class QuadDataStructure
{
public:
    QuadDataStructure();
    virtual ~QuadDataStructure();
    
    /// Return the coordinate system we're working in
    virtual CoordSystem *getCoordSystem() = 0;
    
    /// Bounding box used to calculate quad tree nodes.  In local coordinate system.
    virtual Mbr getTotalExtents() = 0;
    
    /// Bounding box of data you actually want to display.  In local coordinate system.
    /// Unless you're being clever, make this the same as totalExtents.
    virtual Mbr getValidExtents() = 0;
    
    /// Return the minimum quad tree zoom level (usually 0)
    virtual int getMinZoom() = 0;
    
    /// Return the maximum quad tree zoom level.  Must be at least minZoom
    virtual int getMaxZoom() = 0;
    
    /// Return an importance value for the given tile
    virtual double importanceForTile(const QuadTreeIdentifier &ident,
                                             const Mbr &mbr,
                                             ViewStateRef viewState,
                                             const Point2f &frameSize) = 0;
    
    /// Called when the view state changes.  If you're caching info, do it here.
    virtual void newViewState(ViewStateRef viewState) = 0;
    
    /// Return true if the tile is visible, false otherwise
    virtual bool visibilityForTile(const QuadTreeIdentifier &ident,
                                           const Mbr &mbr,
                                           ViewStateRef viewState,
                                           const Point2f &frameSize) = 0;
};

/** The Quad Display Layer (New) calls an object with this protocol.
 Display layer does the geometric logic.  Up to you to do something with it.
 */
class QuadLoaderNew
{
public:
    QuadLoaderNew() { }
    virtual ~QuadLoaderNew() { }
    
    /// Called when the layer first starts up.  Keep this around if you need it.
    virtual void setController(QuadDisplayControllerNew *inControl) { control = inControl; }
    
    /// Load some tiles, unload others, and the rest had their importance values change
    /// Return the nodes we wanted to keep rather than delete
    virtual QuadTreeNew::NodeSet quadLoaderUpdate(PlatformThreadInfo *threadInfo,
                                                  const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
                                                  const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
                                                  const WhirlyKit::QuadTreeNew::ImportantNodeSet &updateTiles,
                                                  int targetLevel,
                                                  ChangeSet &changes) = 0;
    
    /// Called right before the layer thread flushes its change requests
    virtual void quadLoaderPreSceenFlush(ChangeSet &changes) = 0;
    
    /// Called when a layer is shutting down (on the layer thread)
    virtual void quadLoaderShutdown(PlatformThreadInfo *threadInfo,ChangeSet &changes) = 0;
    
protected:
    QuadDisplayControllerNew *control;
};

/** Quad Display Controller (New)
 
    This class is meant to be used by a LayerThread (or equivalent) and is called
    to manage the tile loading of a data set.  Other objects actually do the tile loading (QuadLoaderNew).
  */
class QuadDisplayControllerNew : public QuadTreeNew
{
public:
    QuadDisplayControllerNew(QuadDataStructure *dataStructure,QuadLoaderNew *loader,SceneRenderer *renderer);
    virtual ~QuadDisplayControllerNew();
    
    /// Scene we're modifying
    Scene *getScene();
    /// The renderer we need for frame sizes
    SceneRenderer *getRenderer();
    /// Quad tree used for paging advice
    QuadTreeNew *getQuadTree();
    /// Coordinate system we're using
    CoordSystem *getCoordSys();
    
    /// Maximum number of tiles loaded in at once
    int getMaxTiles();
    void setMaxTiles(int);
    
    /// How often this layer gets notified of view changes.  1s by default.
    TimeInterval getViewUpdatePeriod();
    void setViewUpdatePeriod(TimeInterval);

    /// Load just the target level (and the lowest level)
    bool getSingleLevel();
    void setSingleLevel(bool);
    
    /// Do we always throw the min level into the mix or not
    void setKeepMinLevel(bool newVal,double height);
        
    /// Level offsets in single level mode
    std::vector<int> getLevelLoads();
    void setLevelLoads(const std::vector<int> &);
    
    /// Minimum screen area to consider for a tile per level
    std::vector<double> getMinImportancePerLevel();
    void setMinImportancePerLevel(const std::vector<double> &imports);
    
    /// Return the geometry information being used
    QuadDataStructure *getDataStructure();
    
    /// Return the current view state, if there is one
    ViewStateRef getViewState();
    
    // Notify any attached loaders and generally get ready to party
    virtual void start();
    
    // Called when the layer thread is tearing down
    virtual void stop(PlatformThreadInfo *threadInfo,ChangeSet &changes);
    
    // Called when the view updates.  Does the heavy lifting.
    // Returns true if it wants to be called again in a bit
    virtual bool viewUpdate(PlatformThreadInfo *threadInfo,ViewStateRef viewState,ChangeSet &changes);
    
    // Called right before we flush the layer thread changes to the scene
    virtual void preSceneFlush(ChangeSet &changes);
    
protected:
    // QuadTreeNew overrides
    double importance(const Node &node);
    bool visible(const Node &node);
    
    QuadDataStructure *dataStructure;
    QuadLoaderNew *loader;

    Scene *scene;
    SceneRenderer *renderer;
    CoordSystem *coordSys;
    Mbr mbr;
    int maxTiles;
    std::vector<double> minImportancePerLevel;
    int minZoom,maxZoom;
    TimeInterval viewUpdatePeriod;
    bool keepMinLevel;
    double keepMinLevelHeight;
    bool singleLevel;
    std::vector<int> levelLoads;

    QuadTreeNew::ImportantNodeSet currentNodes;

    ViewStateRef viewState;
};
    
typedef std::shared_ptr<QuadDisplayControllerNew> QuadDisplayControllerNewRef;
    
}

