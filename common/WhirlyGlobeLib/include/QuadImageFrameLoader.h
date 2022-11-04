/*  QuadImageFrameLoader.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
 *  Copyright 2011-2022 mousebird consulting
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
    QIFBatchOps() = default;
    virtual ~QIFBatchOps() = default;

    // Tiles we deleted for callback later
    std::vector<QuadTreeIdentifier> deletes;
};

// Assets and status associated with a single tile's frame
class QIFFrameAsset
{
public:
    typedef enum {Empty,Loaded,Loading} State;
    
    QIFFrameAsset(QuadFrameInfoRef frameInfo);
    virtual ~QIFFrameAsset() = default;
    
    // What the frame is doing
    State getState() const { return state; }
    
    // Load priority
    int getPriority() const { return priority; }
    
    // Texture ID (if loaded)
    const std::vector<SimpleIdentity> &getTexIDs() const { return texIDs; }
    
    // Return information about which frame this is
    QuadFrameInfoRef getFrameInfo() const { return frameInfo; }

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
    virtual void setLoadReturn(RawDataRef data);
    
    // Store the data used by the interpreter for processing
    virtual void setLoadReturnRef(const QuadLoaderReturnRef &loadReturnRef);

    // Store the data used by the interpreter for processing
    virtual void setLoadReturnRef(QuadLoaderReturnRef &&loadReturnRef);

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
    virtual ~QIFTileAsset() = default;
    
    typedef enum {Waiting,Active} State;
    
    State getState() const { return state; }
    
    bool getShouldEnable() const { return shouldEnable; }
    void setShouldEnable(bool newVal) { shouldEnable = newVal; }
    
    QuadTreeNew::ImportantNode getIdent() const { return ident; }
    
    const std::vector<SimpleIdentity> &getInstanceDrawIDs(int focusID) const;
    const SimpleIDSet &getCompObjs() { return compObjs; }
    const SimpleIDSet &getOvlCompObjs() { return ovlCompObjs; }
    
    // Number of frames in this tile
    int getNumFrames() const { return (int)frames.size(); }
    
    // Return the frame asset corresponding to the frame ID
    QIFFrameAssetRef getFrame(int frameID) const;

    // Find the frame corresponding to the given source
    virtual QIFFrameAssetRef findFrameFor(SimpleIdentity frameID);
    // Find the frame corresponding to the given source
    virtual QIFFrameAssetRef findFrameFor(const QuadFrameInfoRef &frameInfo);

    // True if any of the frames are in the process of loading
    virtual bool anyFramesLoading(QuadImageFrameLoader *loader);

    // Check if we're in the process of loading any of the given frames
    virtual bool anyFramesLoading(const std::set<QuadFrameInfoRef> &frameInfos);

    // True if any frames have loaded
    virtual bool anyFramesLoaded(QuadImageFrameLoader *loader);
    
    // Check if we've already loaded any of the given frames
    virtual bool anyFramesLoaded(const std::set<QuadFrameInfoRef> &frameInfos);

    // True if the given frame is loading
    virtual bool isFrameLoading(const QuadFrameInfoRef &frameInfo);

    // True if the given frame is loading
    virtual bool isFrameLoading(SimpleIdentity frameID);

    // Set the loader return used by the interpreter so we can cancel it
    virtual void setLoadReturnRef(SimpleIdentity frameID, const QuadLoaderReturnRef &loadReturnRef);

    // Set the loader return used by the interpreter so we can cancel it
    virtual void setLoadReturnRef(const QuadFrameInfoRef &frame, const QuadLoaderReturnRef &loadReturnRef);

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
    virtual void startFetching(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,
                               const QuadFrameInfoRef &frameToLoad,QIFBatchOps *batchOps, ChangeSet &changes);

    // Set up the geometry for this tile
    virtual void setupContents(QuadImageFrameLoader *loader,
                               const LoadedTileNewRef &loadedTile,
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
                               const QuadFrameInfoRef &frameToCancel,
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
    virtual void mergeLoadedFrame(QuadImageFrameLoader *loader,SimpleIdentity frameID, RawDataRef data);

    // Keep track of the load return data (just for single frame + multiple source mode)
    virtual void mergeLoadedFrame(QuadImageFrameLoader *loader,const QuadFrameInfoRef &frameInfo, RawDataRef data);

    // Return all the low level data (and reset it) if we're in that mode
    virtual void getLoadedData(std::vector<RawDataRef> &allData);
            
protected:
    // Specialized frame asset
    virtual QIFFrameAssetRef makeFrameAsset(PlatformThreadInfo *threadInfo,
                                            const QuadFrameInfoRef &frameInfo,
                                            QuadImageFrameLoader *) = 0;

    State state = Waiting;
    QuadTreeNew::ImportantNode ident;
    
    // Set if the sampling layer thinks this should be on
    bool shouldEnable = false;
    
    // One set of instance IDs per focus
    std::vector<std::vector<SimpleIdentity> > instanceDrawIDs;
    
    std::vector<QIFFrameAssetRef> frames;
    
    // Component objects associated with this tile (not frame)
    SimpleIDSet compObjs,ovlCompObjs;
    
    int drawPriority = 0;
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
        
        bool enabled = false; // Set it on or off
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
    QIFRenderState() = default;
    QIFRenderState(int numFocus, int numFrames);
    
    std::map<QuadTreeNew::Node,QIFTileStateRef> tiles;

    int texSize = 0;
    int borderSize = 0;
    
    // Number of tiles loaded for each frame
    std::vector<int> tilesLoaded;
    // Number of tiles at the lowest level loaded for each frame
    std::vector<bool> topTilesLoaded;
    
    bool hasUpdate(const std::vector<double> &curFrames,bool masterUpdate) const;
    
    std::vector<double> lastCurFrames;
    TimeInterval lastRenderTime = 0.0;
    TimeInterval lastUpdate = 0.0;
    bool lastMasterEnable = false;
    
    // Update what the scene is looking at.  Ideally not every frame.
    void updateScene(Scene *scene,
                     const std::vector<double> &curFrames,
                     TimeInterval now,
                     bool flipY,
                     const RGBAColor &color,
                     bool masterEnable,
                     ChangeSet &changes);
};
    
/** Quad Image Frame Loader
 
    This quad loader handles the logic for a multi-frame set of images.
  */
class QuadImageFrameLoader : public QuadTileBuilderDelegate, public ActiveModel
{
public:
    typedef enum {SingleFrame,MultiFrame,Object} Mode;
    typedef enum {Broad,Narrow} LoadMode;
    typedef enum {All,Current} FrameLoadMode;

    QuadImageFrameLoader(const SamplingParams &, Mode, FrameLoadMode = FrameLoadMode::All);
    virtual ~QuadImageFrameLoader() = default;
    
    /// Add a focus
    void addFocus();
    
    /// Number of focus points (usually 1)
    int getNumFocus() const { return numFocus; }

    /// Loading mode we're currently supporting
    Mode getMode() const { return mode; }

    /// If set, we'll see way too much output
    void setDebugMode(bool newMode);
    bool getDebugMode() const { return debugMode; }

    void setLabel(const char* lbl) { label.clear(); if (lbl) label = lbl; }
    void setLabel(std::string s) { label = std::move(s); }
    const std::string &getLabel() const { return label; }

    /// Turn the display on or off.  Loading continues normally
    void setMasterEnable(bool newEnable) { masterEnable = newEnable; }
    
    /// Set loading mode to Broad (load lowest level first) and Narrow (load current frame first)
    void setLoadMode(LoadMode newMode);
    LoadMode getLoadMode() const { return loadMode; }

    ///  Load all frames or only the one(s) current visible
    ///  Returns true if the mode was changed, resulting in cancel/refresh actions.
    bool setFrameLoadMode(FrameLoadMode, PlatformThreadInfo *, ChangeSet &changes);
    FrameLoadMode getFrameLoadMode() const { return frameLoadMode; }

    /// True if there's loading going on, false if it's settled
    bool getLoadingStatus() const { return loadingStatus; }
    
    // Calculate the load priority for a given tile, respecting the rules
    int calcLoadPriority(const QuadTreeNew::ImportantNode &ident,int frame);

    /// Recalculate the loading default priorities
    void updatePriorityDefaults();

    /// Set/Change the sampling parameters.
    virtual void setSamplingParams(const SamplingParams &inParams) { params = inParams; }
    virtual const SamplingParams &getSamplingParams() const { return params; }
    
    /// Set a separate set of zoom limits
    virtual void setZoomLimits(int minZoom,int maxZoom);
    
    // We potentially have a separate set of zoom limits
    virtual int getMinZoom() const { return minZoom; }
    virtual int getMaxZoom() const { return maxZoom; }

    /// Set if we need the top tiles to load before we'll display a frame
    virtual void setRequireTopTilesLoaded(bool newVal) { requiringTopTilesLoaded = newVal; }

    /// Return the quad display controller this is attached to
    QuadDisplayControllerNew *getController() const { return control; }

    /// Color for polygons created during loading
    void setColor(const RGBAColor &inColor,ChangeSet *changes);
    const RGBAColor &getColor() const { return color; }
    
    /// Render target for the geometry being created
    void setRenderTarget(int focusID,SimpleIdentity renderTargetID);
    SimpleIdentity getRenderTarget(int focusID);
    
    /// Shader ID for geometry being created
    void setShaderID(int focusID,SimpleIdentity shaderID);
    SimpleIdentity getShaderID(int focusID);
    
    /// Return the uniform block (if there is one) that's to be set on all geometry created
    void setUniBlock(const BasicDrawable::UniformBlock &block) { uniBlock = block; }
    BasicDrawable::UniformBlock &getUniBlock() { return uniBlock; }
    const BasicDrawable::UniformBlock &getUniBlock() const { return uniBlock; }

    /// In-memory texture type
    void setTexType(TextureType type) { texType = type; }
    void setTexByteSource(WKSingleByteSource src) { texByteSource = src; }
    
    /// If we're using border pixels, set the individual texture size and border size
    void setTexSize(int texSize,int borderSize);
    
    /// Control draw priority assigned to basic drawable instances
    void setBaseDrawPriority(int newPrior) { baseDrawPriority = newPrior; }
    void setDrawPriorityPerLevel(int newPrior) { drawPriorityPerLevel = newPrior; }

    /// What part of the animation we're displaying.
    /// If the frame loading mode is set to `Current`, you will need to refresh the appropriate frame(s) manually.
    void setCurFrame(PlatformThreadInfo *, int focusID, double curFrame);
    
    /// What part of the animation we're displaying.
    void setCurFrame(PlatformThreadInfo *, int focusID, double curFrame, ChangeSet &);

    double getCurFrame(int focusID) const;

    // Need to know how we're loading the tiles to calculate the render state
    void setFlipY(bool newFlip) { flipY = newFlip; }
    
    // Need to know how we're loading the tiles to calculate the render state
    bool getFlipY() const { return flipY; }
    
    /// Number of frames we're representing
    virtual int getNumFrames() const { return frames.size(); }
    
    // Return the frame info object for a given index
    virtual QuadFrameInfoRef getFrameInfo(int which) const;

    // Determine whether a given frame should be loaded right now
    bool frameShouldLoad(int which) const;

    // Reset all the frames at once
    virtual void setFrames(const std::vector<QuadFrameInfoRef> &newFrames);
    
    /// Current generation of the loader (used for reload lagging)
    int getGeneration() const;
    
    /// Reload the given frame (or everything)
    virtual void reload(PlatformThreadInfo *threadInfo,int frame, ChangeSet &changes);

    /// Reload matching tiles in the given frame (or everything)
    virtual void reload(PlatformThreadInfo *threadInfo,int frame,const Mbr *bound,int boundCount,ChangeSet &changes);
    
    /// Recalculate all the frame/tile priorities
    virtual void updatePriorities(PlatformThreadInfo *threadInfo);

    /// **** QuadTileBuilderDelegate methods ****
    
    /// Called when the builder first starts up.  Keep this around if you need it.
    virtual void setBuilder(QuadTileBuilder *builder,QuadDisplayControllerNew *control) override;
    
    /// Before we tell the delegate to unload tiles, see if they want to keep them around
    /// Returns the tiles we want to preserve after all
    virtual QuadTreeNew::NodeSet builderUnloadCheck(QuadTileBuilder *inBuilder,
                                                    const QuadTreeNew::ImportantNodeSet &loadTiles,
                                                    const QuadTreeNew::NodeSet &unloadTiles,
                                                    int inTargetLevel) override;
    
    /// Load the given group of tiles.  If you don't load them immediately, up to you to cancel any requests
    virtual void builderLoad(PlatformThreadInfo *threadInfo,
                             QuadTileBuilder *inBuilder,
                             const TileBuilderDelegateInfo &updates,
                             ChangeSet &changes) override;
    
    /// Called within builderLoad to let subclasses do other things
    virtual void builderLoadAdditional(PlatformThreadInfo *threadInfo,
                                       QuadTileBuilder *inBuilder,
                                       const TileBuilderDelegateInfo &updates,
                                       ChangeSet &changes);
    
    /// Called right before the layer thread flushes all its current changes
    virtual void builderPreSceneFlush(QuadTileBuilder *inBuilder, ChangeSet &changes) override;
    
    /// Shutdown called on the layer thread if you stuff to clean up
    virtual void builderShutdown(PlatformThreadInfo *threadInfo, QuadTileBuilder *inBuilder, ChangeSet &changes) override;
    
    /// Returns true if we're in the middle of loading things
    virtual bool builderIsLoading() const override { return loadingStatus; }
    
    /// **** Active Model methods ****

    /// Returns true if there's an update to process
    virtual bool hasUpdate() const override;
    
    /// Process the update
    virtual void updateForFrame(RendererFrameInfo *frameInfo) override;

    /** ----- **/
    
    /**
     Instantaneous per-frame stats for the quad image frame loader.
     */
    struct FrameStats
    {
        // Number of tiles this frame is present in
        int totalTiles = 0;
        // Tiles yet to load for this frame
        int tilesToLoad = 0;
    };

    /**
     Instantaneous stats for the whole loader.
     */
    struct Stats
    {
        // Total number of tiles being managed
        int numTiles = 0;
        
        // Per frame stats
        std::vector<FrameStats> frameStats;
    };

    /// Return the stats (thread safe)
    Stats getStats() const;
    
    /// Shut everything down and remove our geometry and resources
    void cleanup(PlatformThreadInfo *threadInfo,ChangeSet &changes);
    
    /// Check if a frame is in the process of loading
    bool isFrameLoading(const QuadTreeIdentifier &ident, SimpleIdentity frameID) const;

    /// Check if a frame is in the process of loading
    bool isFrameLoading(const QuadTreeIdentifier &ident,const QuadFrameInfoRef &frame) const;

    /// Set the loader return ref for cancelling while parsing
    void setLoadReturnRef(const QuadTreeIdentifier &ident,
                          const QuadFrameInfoRef &frame,
                          const QuadLoaderReturnRef &loadReturnRef);
    
    /// Called when the data for a frame comes back
    /// Returns true if that tile is ready for merging
    bool mergeLoadedFrame(const QuadTreeIdentifier &ident,
                          SimpleIdentity tileID,
                          RawDataRef data,
                          std::vector<RawDataRef> &allData);

    /// Called when the data for a frame comes back
    /// Returns true if that tile is ready for merging
    bool mergeLoadedFrame(const QuadTreeIdentifier &ident,
                          const QuadFrameInfoRef &frameInfo,
                          RawDataRef data,
                          std::vector<RawDataRef> &allData);

    /// Builds the render state *and* send it over to the main thread via the scene changes
    virtual void buildRenderState(ChangeSet &changes);

    /// Update the rendering state from the layer thread.  Used in non-frame mode
    virtual void updateRenderState(ChangeSet &changes);
    
    // Run on the layer thread.  Merge the loaded tile into the data.
    virtual void mergeLoadedTile(PlatformThreadInfo *threadInfo,QuadLoaderReturn *loadReturn,ChangeSet &changes);

    // Add a delegate to be called whenever the loading status changes
    using LoadingDelegate = std::function<void(SimpleIdentity,bool)>;
    SimpleIdentity addLoadingDelegate(LoadingDelegate);
    void removeLoadingDelegate(SimpleIdentity);

public:
    ComponentManagerRef compManager;

protected:
    // Return a set of the active frames
    std::set<QuadFrameInfoRef> getActiveFrames() const;

    void setLoadingStatus(bool isLoading);
    void updateLoadingStatus();

    mutable std::mutex statsLock;
    Stats stats;
    
    // Periodically generates the stats
    void makeStats();
    
    // Construct a platform specific tile/frame assets in the subclass
    virtual QIFTileAssetRef makeTileAsset(PlatformThreadInfo *threadInfo,const QuadTreeNew::ImportantNode &ident) = 0;
    
    // Construct a platform specific BatchOps for passing to tile fetcher
    // (we don't know about tile fetchers down here)
    virtual QIFBatchOps *makeBatchOps(PlatformThreadInfo *threadInfo) = 0;
    
    // Process whatever ops we batched up during the load phase
    virtual void processBatchOps(PlatformThreadInfo *threadInfo,QIFBatchOps *) = 0;
        
    virtual void removeTile(PlatformThreadInfo *threadInfo,const QuadTreeNew::Node &ident, QIFBatchOps *batchOps, ChangeSet &changes);
    QIFTileAssetRef addNewTile(PlatformThreadInfo *threadInfo,const QuadTreeNew::ImportantNode &ident,QIFBatchOps *batchOps,ChangeSet &changes);

    Mode mode;
    LoadMode loadMode = LoadMode::Narrow;
    FrameLoadMode frameLoadMode = FrameLoadMode::All;
    
    bool masterEnable = true;
    bool debugMode = false;
    std::string label;
    
    SamplingParams params;
    int minZoom;
    int maxZoom;

    // Set if we require the top tiles to be loaded before we'll display a frame
    bool requiringTopTilesLoaded = true;
    
    TextureType texType = TexTypeUnsignedByte;
    WKSingleByteSource texByteSource = WKSingleRGB;

    int texSize = 0;
    int borderSize = 0;

    // Number of focus points (1 by default)
    int numFocus = 1;

    // One per focus point
    std::vector<SimpleIdentity> shaderIDs = { EmptyIdentity };

    // One per focus point
    std::vector<double> curFrames = { 0.0 };
    
    bool flipY = true;

    int baseDrawPriority = 100;
    int drawPriorityPerLevel = 1;

    bool colorChanged = false;
    RGBAColor color = RGBAColor::white();
    
    // One per focus
    std::vector<SimpleIdentity> renderTargetIDs = { EmptyIdentity };
    BasicDrawable::UniformBlock uniBlock;

    // Tiles in various states of loading or loaded
    QIFTileAssetMap tiles;
    
    // The builder this is a delegate of
    QuadDisplayControllerNew *control = nullptr;
    QuadTileBuilder *builder = nullptr;
    
    // Tile rendering info supplied from the layer thread
    QIFRenderState renderState;
    
    bool changesSinceLastFlush = false;
    
    // We number load requests so we can catch old ones after doing a reload
    int generation = 0;
    
    // Last target level set in quadBuilder:update: callback
    int targetLevel = -1;
    
    // This is set to the targetLevel when all targetLevel tiles are loaded
    int curOvlLevel = -1;
    
    // If set, used to signal when we're done to a RunRequest in process
    std::shared_ptr<bool> lastRunReqFlag;
    
    // True if we're trying to load something, false if we're not
    bool loadingStatus = true;
    
    // Default load priority values.  Used to assign loading priorities
    int topPriority = -1;        // Top nodes, if they're special.  -1 if not
    int nearFramePriority = -1;  // Frames next to the current one, -1 if not
    int restPriority = -1;       // Everything else
    
    // Information about each frame.  Subclasses do more interesting things with this
    std::vector<QuadFrameInfoRef> frames;
    
    std::map<SimpleIdentity,LoadingDelegate> loadingDelegates;
};

}
