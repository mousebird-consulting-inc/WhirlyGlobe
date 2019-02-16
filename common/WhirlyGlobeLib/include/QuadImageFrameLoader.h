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

namespace WhirlyKit
{

// Assets and status associated with a single tile's frame
class QIFFrameAsset
{
public:
    typedef enum {Empty,Loaded,Loading} State;
    
    QIFFrameAsset();
    
    State getState();
    int getPriority();
    SimpleIdentity getTexID();
    
    // Clear out the texture and reset
    void clear(NSMutableArray *toCancel,ChangeSet &changes);
    
    // Put together a fetch request and return it
    MaplyTileFetchRequest *setupFetch(id fetchInfo,id frameInfo,int priority,double importance);
    
    // Update priority for an existing fetch request
    void updateFetching(NSObject<MaplyTileFetcher> *tileFetcher,int newPriority,double newImportance);
    
    // Cancel an outstanding fetch
    void cancelFetch(NSMutableArray *toCancel);
    
    // Keep track of the texture ID
    void loadSuccess(Texture *tex);
    
    void loadFailed();
    
protected:
    State state;
    
    // Returned by the TileFetcher
    MaplyTileFetchRequest *request;
    int priority;
    double importance;
    
    // If set, the texture ID for this asset
    SimpleIdentity texID;
};

typedef std::shared_ptr<QIFFrameAsset> QIFFrameAssetRef;

// Holds all the frame assets and manages various loading state
class QIFTileAsset
{
public:
    QIFTileAsset(const QuadTreeNew::ImportantNode &ident, int numFrames);
    
    typedef enum {Waiting,Active} State;
    
    State getState();
    
    bool getShouldEnable();
    void setShouldEnable(bool newVal);
    
    QuadTreeNew::ImportantNode getIdent();
    
    const std::vector<SimpleIdentity> &getInstanceDrawIDs();
    
    QIFFrameAssetRef getFrame(int frameID);
    
    // True if any of the frames are in the process of loading
    bool anyFramesLoading();
    
    // True if any frames have loaded
    bool anyFramesLoaded();
    
    // Fetch the tile frames.  Just fetch them all for now.
    void startFetching(MaplyQuadImageFrameLoader *loader,NSMutableArray *toStart,NSArray<NSObject<MaplyTileInfoNew> *> *frameInfos);
    
    // True if the given frame is loading
    bool isFrameLoading(int which);
    
    // Importance value changed, so update the fetcher
    void setImportance(NSObject<MaplyTileFetcher> *tileFetcher,double import);
    
    // Clear out the individual frames, loads and all
    void clearFrames(NSMutableArray *toCancel,ChangeSet &changes);
    
    // Clear out geometry and all the frame info
    void clear(NSMutableArray *toCancel, ChangeSet &changes);
    
    // Set up the geometry for this tile
    void setupContents(MaplyQuadImageFrameLoader *loader,LoadedTileNewRef loadedTile,int defaultDrawPriority,SimpleIdentity shaderID,ChangeSet &changes);
    
    // Cancel any outstanding fetches
    void cancelFetches(NSMutableArray *toCancel);
    
    // A single frame loaded successfully
    void frameLoaded(MaplyLoaderReturn *loadReturn,Texture *tex,ChangeSet &changes);
    
    // A single frame failed to load
    void frameFailed(MaplyLoaderReturn *loadReturn,ChangeSet &changes);
    
protected:
    State state;
    QuadTreeNew::ImportantNode ident;
    
    // Set if the sampling layer thinks this should be on
    bool shouldEnable;
    
    std::vector<SimpleIdentity> instanceDrawIDs;
    
    std::vector<QIFFrameAssetRef> frames;
    
    int drawPriority;
};

typedef std::shared_ptr<QIFTileAsset> QIFTileAssetRef;
typedef std::map<QuadTreeNew::Node,QIFTileAssetRef> QIFTileAssetMap;

// Information about a single tile and its current state
class QIFTileState
{
public:
    QIFTileState(int numFrames);
    
    QuadTreeNew::Node node;
    
    // Set if this should be enabled
    bool enable;
    
    // The geometry used to represent the tile
    std::vector<SimpleIdentity> instanceDrawIDs;
    
    // Information about each frame
    class FrameInfo {
    public:
        FrameInfo()
        
        // Node we're using a texture from (could be this one)
        QuadTreeNew::Node texNode;
        SimpleIdentity texID;
    };
    
    // A texture ID per frame
    std::vector<FrameInfo> frames;
};
typedef std::shared_ptr<QIFTileState> QIFTileStateRef;
    
// Used to track loading state and hand it over to the main thread
class QIFRenderState
{
public:
    QIFRenderState();
    QIFRenderState(int numFrames);
    
    std::map<QuadTreeNew::Node,QIFTileStateRef> tiles;
    
    // Number of tiles loaded for each frame
    std::vector<int> tilesLoaded;
    // Number of tiles at the lowest level loaded for each frame
    std::vector<bool> topTilesLoaded;
    
    bool hasUpdate(double curFrame);
    
    double lastCurFrame;
    TimeInterval lastRenderTime;
    TimeInterval lastUpdate;
    
    // Update what the scene is looking at.  Ideally not every frame.
    void updateScene(Scene *scene,double curFrame,TimeInterval now,bool flipY,const RGBAColor &color,ChangeSet &changes);
};

/** Quad Image Frame Loader
 
    This quad loader handles the logic for a multi-frame set of images.
  */
class QuadImageFrameLoader : public QuadTileBuilderDelegate
{
public:
    QuadImageFrameLoader();
    virtual ~QuadImageFrameLoader();
    
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
    
    /// Shutdown called on the layer thread if you stuff to clean up
    virtual void builderShutdown(QuadTileBuilder *builder,ChangeSet &changes);

};

}
