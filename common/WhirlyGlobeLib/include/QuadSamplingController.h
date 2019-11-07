/*
 *  QuadSamplingController.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
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

#import "QuadSamplingParams.h"
#import "QuadDisplayControllerNew.h"
#import "QuadTileBuilder.h"

namespace WhirlyKit
{

/** The Quad Sampling Controller runs the quad tree and related
    data structures, figuring out what to load and loading it.
    It needs a native interface to call its various methods.
 */
class QuadSamplingController : public QuadDataStructure, public QuadTileBuilderDelegate
{
public:
    QuadSamplingController();
    virtual ~QuadSamplingController();
    
    // Number of clients using this sampler
    int getNumClients();
    
    // Return the Display Controller we're using
    QuadDisplayControllerNewRef getDisplayControl();
    
    // Return the builder we're using
    QuadTileBuilderRef getBuilder() { return builder; }
    
    // Add a new builder delegate to watch tile related events
    // Returns true if we need to notify the delegate
    bool addBuilderDelegate(QuadTileBuilderDelegateRef delegate);
    
    // Remove the given builder delegate that was watching tile related events
    void removeBuilderDelegate(QuadTileBuilderDelegateRef delegate);

    // Called right before we start using the controller
    void start(const SamplingParams &params,Scene *scene,SceneRenderer *renderer);
    // Unhook everything and shut it down
    void stop();
    
    // Called on the layer thread to initialize a new builder
    void notifyDelegateStartup(SimpleIdentity delegateID,ChangeSet &changes);
    
    /// **** QuadDataStructure methods ****
    
    /// Return the coordinate system we're working in
    virtual CoordSystem *getCoordSystem();
    
    /// Bounding box used to calculate quad tree nodes.  In local coordinate system.
    virtual Mbr getTotalExtents();
    
    /// Bounding box of data you actually want to display.  In local coordinate system.
    /// Unless you're being clever, make this the same as totalExtents.
    virtual Mbr getValidExtents();
    
    /// Return the minimum quad tree zoom level (usually 0)
    virtual int getMinZoom();
    
    /// Return the maximum quad tree zoom level.  Must be at least minZoom
    virtual int getMaxZoom();
    
    /// Return an importance value for the given tile
    virtual double importanceForTile(const QuadTreeIdentifier &ident,
                                     const Mbr &mbr,
                                     ViewStateRef viewState,
                                     const Point2f &frameSize);
    
    /// Called when the view state changes.  If you're caching info, do it here.
    virtual void newViewState(ViewStateRef viewState);
    
    /// Return true if the tile is visible, false otherwise
    virtual bool visibilityForTile(const QuadTreeIdentifier &ident,
                                   const Mbr &mbr,
                                   ViewStateRef viewState,
                                   const Point2f &frameSize);
    
    /// **** QuadTileBuilderDelegate methods ****

    /// Called when the builder first starts up.  Keep this around if you need it.
    virtual void setBuilder(QuadTileBuilder *builder,QuadDisplayControllerNew *control);
    
    /// Before we tell the delegate to unload tiles, see if they want to keep them around
    /// Returns the tiles we want to preserve after all
    virtual QuadTreeNew::NodeSet builderUnloadCheck(QuadTileBuilder *builder,
                                                    const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,
                                                    const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,
                                                    int targetLevel);
    
    /// Load the given group of tiles.  If you don't load them immediately, up to you to cancel any requests
    virtual void builderLoad(QuadTileBuilder *builder,
                             const WhirlyKit::TileBuilderDelegateInfo &updates,
                             ChangeSet &changes);
    
    /// Called right before the layer thread flushes all its current changes
    virtual void builderPreSceneFlush(QuadTileBuilder *builder,ChangeSet &changes);
    
    /// Shutdown called on the layer thread if you have stuff to clean up
    virtual void builderShutdown(QuadTileBuilder *builder,ChangeSet &changes);

    /// Quick loading status check
    virtual bool builderIsLoading();

protected:
    bool debugMode;

    std::mutex lock;
    
    SamplingParams params;
    QuadDisplayControllerNewRef displayControl;

    WhirlyKit::Scene *scene;
    SceneRenderer *renderer;

    QuadTileBuilderRef builder;
    std::vector<QuadTileBuilderDelegateRef> builderDelegates;
    
    bool builderStarted;
    bool valid;
};
    
}
