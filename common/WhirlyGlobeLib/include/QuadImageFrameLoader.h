/*
 *  QuadImageFrameLoader.h
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

#import "QuadSamplingController.h"
#import "QuadLoaderReturn.h"
#import "ComponentManager.h"

namespace WhirlyKit
{
    
class QuadImageFrameLoader;
    
// We store cancels and adds to do them all at once
// The subclass passes its own version of this around
class QIFBatchOps
{
public:
    QIFBatchOps();
    virtual ~QIFBatchOps();

    // Tiles we deleted for callback later
    std::vector<QuadTreeIdentifier> deletes;
};

// Assets and status associated with a single tile's frame
class QIFFrameAsset
{
public:
    typedef enum {Empty,Loaded,Loading} State;
    
    QIFFrameAsset(QuadFrameInfoRef frameInfo);
    virtual ~QIFFrameAsset();
    
    // What the frame is doing
    State getState();
    
    // Load priority
    int getPriority();
    
    // Texture ID (if loaded)
    const std::vector<SimpleIdentity> &getTexIDs();
    
    // Return information about which frame this is
    QuadFrameInfoRef getFrameInfo();

    // Just sets the state to loading
    virtual void setupFetch(QuadImageFrameLoader *loader);
    
    // Clear out the texture and reset
    virtual void clear(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes);

    // Update priority for an existing fetch request
    virtual bool updateFetching(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,int newPriority,double newImportance);
    
    // Cancel an outstanding fetch
    virtual void cancelFetch(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps);

    // Keep track of the texture ID
    virtual void loadSuccess(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,const std::vector<Texture *> &texs);
    
    // Clear out state
    virtual void loadFailed(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader);
    
    // We're not bothering to load it, but pretend like it succeeded
    virtual void loadSkipped();

    // Store the raw data for use later
    virtual void setLoadReturn(const RawDataRef &data);
    
    // Store the data used by the interpreter for processing
    virtual void setLoadReturnRef(const QuadLoaderReturnRef &loadReturnRef);
    
    // Return the load return
    virtual RawDataRef getLoadReturn();
    
    // True if a load return was set at one point
    virtual bool hasLoadReturn();
    
    // Clear out any temp stored data
    virtual void clearLoadReturn();
    
protected:
    State state;
    
    int priority;
    double importance;
    
    // Which frame this is on the tile side
    QuadFrameInfoRef frameInfo;
    
    // If set, the texture ID for this asset
    std::vector<SimpleIdentity> texIDs;
    
    // When fetching a single frame that has multiple data sources, we store the data here
    bool loadReturnSet;
    RawDataRef loadReturn;
    // Loader return passed to Interpreter
    QuadLoaderReturnRef loadReturnRef;
};

typedef std::shared_ptr<QIFFrameAsset> QIFFrameAssetRef;

// Holds all the frame assets and manages various loading state
class QIFTileAsset
{
    friend class QuadImageFrameLoader;
public:
    QIFTileAsset(const QuadTreeNew::ImportantNode &ident);
    void setupFrames(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,int numFrames);
    virtual ~QIFTileAsset();
    
    typedef enum {Waiting,Active} State;
    
    State getState();
    
    bool getShouldEnable();
    void setShouldEnable(bool newVal);
    
    QuadTreeNew::ImportantNode getIdent();
    
    const std::vector<SimpleIdentity> &getInstanceDrawIDs(int focusID);
    const SimpleIDSet &getCompObjs() { return compObjs; }
    const SimpleIDSet &getOvlCompObjs() { return ovlCompObjs; }
    
    // Number of frames in this tile
    int getNumFrames() { return (int)frames.size(); }
    
    // Return the frame asset corresponding to the frame ID
    QIFFrameAssetRef getFrame(int frameID);
    
    // Find the frame corresponding to the given source
    virtual QIFFrameAssetRef findFrameFor(QuadFrameInfoRef frameInfo);

    // True if any of the frames are in the process of loading
    virtual bool anyFramesLoading(QuadImageFrameLoader *loader);

    // Check if we're in the process of loading any of the given frames
    virtual bool anyFramesLoading(const std::set<QuadFrameInfoRef> &frameInfos);

    // True if any frames have loaded
    virtual bool anyFramesLoaded(QuadImageFrameLoader *loader);
    
    // Check if we've already loaded any of the given frames
    virtual bool anyFramesLoaded(const std::set<QuadFrameInfoRef> &frameInfos);

    // True if the given frame is loading
    virtual bool isFrameLoading(QuadFrameInfoRef frameInfo);
    
    // Set the loader return used by the interpreter so we can cancel it
    virtual void setLoadReturnRef(QuadFrameInfoRef frame,QuadLoaderReturnRef loadReturnRef);
    
    // True if all frames are loaded (just for single frame + multiple source mode)
    virtual bool allFramesLoaded();
    
    // True if any frames are loading
    virtual bool anythingLoading();
    
    // Importance value changed, so update the fetcher
    virtual void setImportance(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,double import);
    
    // Clear out the individual frames, loads and all
    virtual void clearFrames(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes);
    
    // Clear out geometry and all the frame info
    virtual void clear(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps, ChangeSet &changes);
    
    // Start fetching data for this tile
    virtual void startFetching(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QuadFrameInfoRef frameToLoad,QIFBatchOps *batchOps, ChangeSet &changes);

    // Set up the geometry for this tile
    virtual void setupContents(QuadImageFrameLoader *loader,
                               LoadedTileNewRef loadedTile,
                               int defaultDrawPriority,
                               const std::vector<SimpleIdentity> &shaderIDs,
                               ChangeSet &changes);
    
    // Change the color immediately
    virtual void setColor(QuadImageFrameLoader *loader,
                          const RGBAColor &newColor,
                          ChangeSet &changes);
    
    // Cancel any outstanding fetches
    virtual void cancelFetches(PlatformThreadInfo *threadInfo,
                               QuadImageFrameLoader *loader,
                               QuadFrameInfoRef frameToCancel,
                               QIFBatchOps *batchOps);
    
    // A single frame loaded successfully
    virtual bool frameLoaded(PlatformThreadInfo *threadInfo,
                             QuadImageFrameLoader *loader,
                             QuadLoaderReturn *loadReturn,
                             std::vector<Texture *> &texs,
                             ChangeSet &changes);
    
    // A single frame failed to load
    virtual void frameFailed(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QuadLoaderReturn *loadReturn,ChangeSet &changes);
    
    // Keep track of the load return data (just for single frame + multiple source mode)
    virtual void mergeLoadedFrame(QuadImageFrameLoader *loader,QuadFrameInfoRef frameInfo,const RawDataRef &data);
    
    // Return all the low level data (and reset it) if we're in that mode
    virtual void getLoadedData(std::vector<RawDataRef> &allData);
            
protected:
    // Specialized frame asset
    virtual QIFFrameAssetRef makeFrameAsset(PlatformThreadInfo *threadInfo,
                                            QuadFrameInfoRef frameInfo,
                                            QuadImageFrameLoader *) = 0;

    State state;
    QuadTreeNew::ImportantNode ident;
    
    // Set if the sampling layer thinks this should be on
    bool shouldEnable;
    
    // One set of instance IDs per focus
    std::vector<std::vector<SimpleIdentity> > instanceDrawIDs;
    
    std::vector<QIFFrameAssetRef> frames;
    
    // Component objects associated with this tile (not frame)
    SimpleIDSet compObjs,ovlCompObjs;
    
    int drawPriority;
};

typedef std::shared_ptr<QIFTileAsset> QIFTileAssetRef;
typedef std::map<QuadTreeNew::Node,QIFTileAssetRef> QIFTileAssetMap;

// Information about a single tile and its current state
class QIFTileState
{
public:
    QIFTileState(int numFrames,const QuadTreeNew::Node &node);
    
    QuadTreeNew::Node node;
    
    // Set if this should be enabled
    bool enable;
    
    // The geometry used to represent the tile
    std::vector<std::vector<SimpleIdentity> > instanceDrawIDs;
    
    // Information about each frame
    class FrameInfo {
    public:
        FrameInfo();
        
        bool enabled; // Set it on or off
        // Node we're using a texture from (could be this one)
        QuadTreeNew::Node texNode;
        std::vector<SimpleIdentity> texIDs;
    };
    
    // Component objects associated with the tile
    SimpleIDSet compObjs,ovlCompObjs;
    
    // A texture ID per frame
    std::vector<FrameInfo> frames;
};
typedef std::shared_ptr<QIFTileState> QIFTileStateRef;
    
// Used to track loading state and hand it over to the main thread
class QIFRenderState
{
public:
    QIFRenderState();
    QIFRenderState(int numFocus,int numFrames);
    
    std::map<QuadTreeNew::Node,QIFTileStateRef> tiles;

    int texSize,borderSize;
    
    // Number of tiles loaded for each frame
    std::vector<int> tilesLoaded;
    // Number of tiles at the lowest level loaded for each frame
    std::vector<bool> topTilesLoaded;
    
    bool hasUpdate(const std::vector<double> &curFrames);
    
    std::vector<double> lastCurFrames;
    TimeInterval lastRenderTime;
    TimeInterval lastUpdate;
    
    // Update what the scene is looking at.  Ideally not every frame.
    void updateScene(Scene *scene,
                     const std::vector<double> &curFrames,
                     TimeInterval now,
                     bool flipY,
                     const RGBAColor &color,
                     ChangeSet &changes);
};
    
/** Quad Image Frame Loader
 
    This quad loader handles the logic for a multi-frame set of images.
  */
class QuadImageFrameLoader : public QuadTileBuilderDelegate, public ActiveModel
{
public:
    typedef enum {SingleFrame,MultiFrame,Object} Mode;
    
    QuadImageFrameLoader(const SamplingParams &params,Mode);
    virtual ~QuadImageFrameLoader();
    
    /// Add a focus
    void addFocus();
    
    /// Number of focus points (usually 1)
    int getNumFocus();

    /// Loading mode we're currently supporting
    Mode getMode();
    
    /// If set, we'll see way too much output
    void setDebugMode(bool newMode);
    bool getDebugMode();
    
    typedef enum {Broad,Narrow} LoadMode;
    
    /// Set loading mode to Broad (load lowest level first) and Narrow (load current frame first)
    void setLoadMode(LoadMode newMode);
    
    /// True if there's loading going on, false if it's settled
    bool getLoadingStatus();
    
    // Calculate the load priority for a given tile, respecting the rules
    int calcLoadPriority(const QuadTreeNew::ImportantNode &ident,int frame);

    /// Recalculate the loading default priorites
    void updatePriorityDefaults();

    /// Set/Change the sampling parameters.
    virtual void setSamplingParams(const SamplingParams &params);
    virtual const SamplingParams &getSamplingParams();
    
    /// Set a separate set of zoom limits
    virtual void setZoomLimits(int minZoom,int maxZoom);
    
    // We potentially have a separate set of zoom limits
    virtual int getMinZoom() { return minZoom; }
    virtual int getMaxZoom() { return maxZoom; }

    /// Set if we need the top tiles to load before we'll display a frame
    virtual void setRequireTopTilesLoaded(bool newVal);

    /// Return the quad display controller this is attached to
    QuadDisplayControllerNew *getController();

    /// Color for polygons created during loading
    void setColor(RGBAColor &inColor,ChangeSet *changes);
    const RGBAColor &getColor();
    
    /// Render target for the geometry being created
    void setRenderTarget(int focusID,SimpleIdentity renderTargetID);
    SimpleIdentity getRenderTarget(int focusID);
    
    /// Shader ID for geometry being created
    void setShaderID(int focusID,SimpleIdentity shaderID);
    SimpleIdentity getShaderID(int focusID);
    
    /// Return the uniform block (if there is one) that's to be set on all geometry created
    void setUniBlock(BasicDrawable::UniformBlock &uniBlock);
    BasicDrawable::UniformBlock &getUniBlock();
    
    // Set a block of data to be passed in as uniform to each DrawInstance created
    void setUniBlock(const WhirlyKit::BasicDrawable::UniformBlock &uniBlock);

    /// In-memory texture type
    void setTexType(TextureType texType);
    
    /// If we're using border pixels, set the individual texture size and border size
    void setTexSize(int texSize,int borderSize);
    
    /// Control draw priority assigned to basic drawable instances
    void setBaseDrawPriority(int newPrior);
    void setDrawPriorityPerLevel(int newPrior);
    
    // What part of the animation we're displaying
    void setCurFrame(int focusID,double curFrame);
    double getCurFrame(int focusID);
    
    // Need to know how we're loading the tiles to calculate the render state
    void setFlipY(bool newFlip);
    
    // Need to know how we're loading the tiles to calculate the render state
    bool getFlipY();
    
    /// Number of frames we're representing
    virtual int getNumFrames();
    
    // Return the frame info object for a given index
    virtual QuadFrameInfoRef getFrameInfo(int which);
    
    // Reset all the frames at once
    virtual void setFrames(const std::vector<QuadFrameInfoRef> &newFrames);
    
    /// Current generation of the loader (used for reload lagging)
    int getGeneration();
    
    /// Reload the given frame (or everything)
    virtual void reload(PlatformThreadInfo *threadInfo,int frame, ChangeSet &changes);

    /// Reload matching tiles in the given frame (or everything)
    virtual void reload(PlatformThreadInfo *threadInfo,int frame,const Mbr *bound,int boundCount,ChangeSet &changes);

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
    virtual void builderLoad(PlatformThreadInfo *threadInfo,
                             QuadTileBuilder *builder,
                             const WhirlyKit::TileBuilderDelegateInfo &updates,
                             ChangeSet &changes);
    
    /// Called within builderLoad to let subclasses do other things
    virtual void builderLoadAdditional(PlatformThreadInfo *threadInfo,
                                       QuadTileBuilder *builder,
                                       const WhirlyKit::TileBuilderDelegateInfo &updates,
                                       ChangeSet &changes);
    
    /// Called right before the layer thread flushes all its current changes
    virtual void builderPreSceneFlush(QuadTileBuilder *builder,ChangeSet &changes);
    
    /// Shutdown called on the layer thread if you stuff to clean up
    virtual void builderShutdown(PlatformThreadInfo *threadInfo,QuadTileBuilder *builder,ChangeSet &changes);
    
    /// Returns true if we're in the middle of loading things
    virtual bool builderIsLoading();
    
    /// **** Active Model methods ****

    /// Returns true if there's an update to process
    virtual bool hasUpdate();
    
    /// Process the update
    virtual void updateForFrame(RendererFrameInfo *frameInfo);
    
    /** ----- **/
    
    /**
     Instantaneous per-frame stats for the quad image frame loader.
     */
    class FrameStats
    {
    public:
        FrameStats();
        
        // Number of tiles this frame is present in
        int totalTiles;
        // Tiles yet to load for this frame
        int tilesToLoad;
    };

    /**
     Instantaneous stats for the whole loader.
     */
    class Stats
    {
    public:
        Stats();
        
        // Total number of tiles being managed
        int numTiles;
        
        // Per frame stats
        std::vector<FrameStats> frameStats;
    };

    /// Return the stats (thread safe)
    Stats getStats();
    
    /// Shut everything down and remove our geometry and resources
    void cleanup(PlatformThreadInfo *threadInfo,ChangeSet &changes);
    
    /// Check if a frame is in the process of loading
    bool isFrameLoading(const QuadTreeIdentifier &ident,QuadFrameInfoRef frame);
    
    /// Set the loader return ref for cancelling while parsing
    void setLoadReturnRef(const QuadTreeIdentifier &ident,QuadFrameInfoRef frame,QuadLoaderReturnRef loadReturnRef);
    
    /// Called when the data for a frame comes back
    /// Returns true if that tile is ready for merging
    bool mergeLoadedFrame(const QuadTreeIdentifier &ident,QuadFrameInfoRef frameInfo,const RawDataRef &data,std::vector<RawDataRef> &allData);
    
    /// Builds the render state *and* send it over to the main thread via the scene changes
    virtual void buildRenderState(ChangeSet &changes);

    /// Update the rendering state from the layer thread.  Used in non-frame mode
    virtual void updateRenderState(ChangeSet &changes);
    
    // Run on the layer thread.  Merge the loaded tile into the data.
    virtual void mergeLoadedTile(PlatformThreadInfo *threadInfo,QuadLoaderReturn *loadReturn,ChangeSet &changes);

    ComponentManager *compManager;

protected:
    // Return a set of the active frames
    virtual const std::set<QuadFrameInfoRef> getActiveFrames();

    void updateLoadingStatus();

    std::mutex statsLock;
    Stats stats;
    
    // Periodically generates the stats
    void makeStats();
    
    // Construct a platform specific tile/frame assets in the subclass
    virtual QIFTileAssetRef makeTileAsset(PlatformThreadInfo *threadInfo,const QuadTreeNew::ImportantNode &ident) = 0;
    
    // Contruct a platform specific BatchOps for passing to tile fetcher
    // (we don't know about tile fetchers down here)
    virtual QIFBatchOps *makeBatchOps(PlatformThreadInfo *threadInfo) = 0;
    
    // Process whatever ops we batched up during the load phase
    virtual void processBatchOps(PlatformThreadInfo *threadInfo,QIFBatchOps *) = 0;
        
    virtual void removeTile(PlatformThreadInfo *threadInfo,const QuadTreeNew::Node &ident, QIFBatchOps *batchOps, ChangeSet &changes);
    QIFTileAssetRef addNewTile(PlatformThreadInfo *threadInfo,const QuadTreeNew::ImportantNode &ident,QIFBatchOps *batchOps,ChangeSet &changes);
    
    Mode mode;
    LoadMode loadMode;
    
    bool debugMode;
    
    SamplingParams params;
    int minZoom,maxZoom;

    // Set if we require the top tiles to be loaded before we'll display a frame
    bool requiringTopTilesLoaded;
    
    TextureType texType;
    int texSize,borderSize;

    // Number of focus points (1 by default)
    int numFocus;

    // One per focus point
    std::vector<WhirlyKit::SimpleIdentity> shaderIDs;

    // One per focus point
    std::vector<double> curFrames;
    
    bool flipY;

    int baseDrawPriority,drawPriorityPerLevel;

    bool colorChanged;
    RGBAColor color;
    
    // One per focus
    std::vector<SimpleIdentity> renderTargetIDs;
    BasicDrawable::UniformBlock uniBlock;

    // Tiles in various states of loading or loaded
    QIFTileAssetMap tiles;
    
    // The builder this is a delegate of
    QuadDisplayControllerNew *control;
    QuadTileBuilder *builder;
    
    // Tile rendering info supplied from the layer thread
    QIFRenderState renderState;
    
    bool changesSinceLastFlush;
    
    // We number load requests so we can catch old ones after doing a reload
    int generation;
    
    // Last target level set in quadBuilder:update: callback
    int targetLevel;
    
    // This is set to the targetLevel when all targetLevel tiles are loaded
    int curOvlLevel;
    
    // If set, used to signal when we're done to a RunRequest in process
    std::shared_ptr<bool> lastRunReqFlag;
    
    // True if we're trying to load something, false if we're not
    bool loadingStatus;
    
    // Default load priority values.  Used to assign loading priorities
    int topPriority;        // Top nodes, if they're special.  -1 if not
    int nearFramePriority;  // Frames next to the current one, -1 if not
    int restPriority;       // Everything else
    
    // Information about each frame.  Subclasses do more interesting things with this
    std::vector<QuadFrameInfoRef> frames;
};
    
}
