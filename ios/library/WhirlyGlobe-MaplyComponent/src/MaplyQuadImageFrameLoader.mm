/*
 *  MaplyQuadImageFrameLoader.mm
 *
 *  Created by Steve Gifford on 9/13/18.
 *  Copyright 2012-2018 mousebird consulting inc
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

#import "MaplyQuadImageFrameLoader.h"
#import "MaplyQuadImageLoader_private.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyShader_private.h"
#import "MaplyRenderTarget_private.h"

@class MaplyQuadImageFrameLoader;

@interface MaplyQuadImageFrameLoader() <WhirlyKitQuadTileBuilderDelegate>
- (void)fetchRequestSuccess:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame data:(NSData *)data;
- (void)fetchRequestFail:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame error:(NSError *)error;
- (bool)hasUpdate;
- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo;
- (int)getNumFrames;
@end

namespace WhirlyKit
{

// Assets and status associated with a single tile's frame
class QIFFrameAsset
{
public:
    typedef enum {Empty,Loaded,Loading} State;
    
    QIFFrameAsset() : state(Empty), request(nil), priority(0), importance(0.0), texID(EmptyIdentity) { }
    
    State getState() { return state; }
    int getPriority() { return priority; }
    SimpleIdentity getTexID() { return texID; }

    // Clear out the texture and reset
    void clear(NSMutableArray *toCancel,ChangeSet &changes) {
        state = Empty;
        if (texID != EmptyIdentity)
            changes.push_back(new RemTextureReq(texID));
        texID = EmptyIdentity;
        if (request) {
            [toCancel addObject:request];
            request = nil;
        }
    }
    
    // Put together a fetch request and return it
    MaplyTileFetchRequest *setupFetch(id fetchInfo,id frameInfo,int priority,double importance) {
        state = Loading;
        
        request = [[MaplyTileFetchRequest alloc] init];
        request.fetchInfo = fetchInfo;
        request.tileSource = frameInfo;
        request.priority = priority;
        request.importance = importance;
        
        return request;
    }
    
    // Update priority for an existing fetch request
    void updateFetching(NSObject<MaplyTileFetcher> *tileFetcher,int newPriority,double newImportance) {
        if (priority == newPriority && importance == newImportance)
            return;
        if (!request)
            return;
        priority = newPriority;
        importance = newImportance;
        
        [tileFetcher updateTileFetch:request priority:priority importance:importance];
    }
    
    // Cancel an outstanding fetch
    void cancelFetch(NSMutableArray *toCancel) {
        if (request)
            [toCancel addObject:request];
        request = nil;
        state = Empty;
    }
    
    // Keep track of the texture ID
    void loadSuccess(Texture *tex) {
        state = Loaded;
        request = nil;
        texID = tex->getId();
    }
    
    void loadFailed() {
        state = Empty;
        request = nil;
    }
    
    void loadSkipped() {
        state = Loaded;
        request = nil;
    }
    
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
    
class QIFTileAsset
{
public:
    QIFTileAsset(const QuadTreeNew::ImportantNode &ident, int numFrames) : state(Waiting), shouldEnable(false), ident(ident), drawPriority(0)
    {
        frames.reserve(numFrames);
        for (unsigned int ii = 0; ii < numFrames; ii++) {
            QIFFrameAssetRef frame(new QIFFrameAsset());
            frames.push_back(frame);
        }
    }
    
    typedef enum {Waiting,Active} State;
    
    State getState() { return state; }

    bool getShouldEnable() { return shouldEnable; }
    void setShouldEnable(bool newVal) { shouldEnable = newVal; }
    
    QuadTreeNew::ImportantNode getIdent() { return ident; }
    
    const std::vector<SimpleIdentity> &getInstanceDrawIDs() { return instanceDrawIDs; }
    
    QIFFrameAssetRef getFrame(int frameID) {
        if (frameID < 0 || frameID >= frames.size())
            return QIFFrameAssetRef(NULL);
        
        return frames[frameID];
    }
    
    // True if any of the frames are in the process of loading
    bool anyFramesLoading() {
        for (auto frame : frames)
            if (frame->getState() == QIFFrameAsset::Loading)
                return true;
        
        return false;
    }
    
    // True if any frames have loaded
    bool anyFramesLoaded() {
        for (auto frame : frames)
            if (frame->getState() == QIFFrameAsset::Loaded)
                return true;
        
        return false;
    }
    
    // Fetch the tile frames.  Just fetch them all for now.
    void startFetching(MaplyQuadImageFrameLoader *loader,NSMutableArray *toStart,NSArray<NSObject<MaplyTileInfoNew> *> *frameInfos) {
        state = Active;
        
        int frame = 0;
        for (MaplyRemoteTileInfoNew *frameInfo in frameInfos) {
            MaplyTileID tileID;  tileID.level = ident.level;  tileID.x = ident.x;  tileID.y = ident.y;
            id fetchInfo = [frameInfo fetchInfoForTile:tileID];
            if (fetchInfo) {
                MaplyTileFetchRequest *request = frames[frame]->setupFetch(fetchInfo,frameInfo,1,ident.importance * loader.importanceScale);
                
                // We need all the min levels to display a a frame, so bump this up a bit
                if (tileID.level == loader->minLevel)
                    request.priority = request.priority - 1;
        
                request.success = ^(MaplyTileFetchRequest *request, NSData *data) {
                    [loader fetchRequestSuccess:request tileID:tileID frame:frame data:data];
                };
                request.failure = ^(MaplyTileFetchRequest *request, NSError *error) {
                    [loader fetchRequestFail:request tileID:tileID frame:frame error:error];
                };
                [toStart addObject:request];
            } else {
                frames[frame]->loadSkipped();
            }
            frame++;
        }
        
    }
    
    // True if the given frame is loading
    bool isFrameLoading(int which) {
        if (which < 0 || which >= frames.size())
            return false;
        
        auto frame = frames[which];
        return frame->getState() == QIFFrameAsset::Loading;
    }

    // Importance value changed, so update the fetcher
    void setImportance(NSObject<MaplyTileFetcher> *tileFetcher,double import) {
        for (auto frame : frames) {
            frame->updateFetching(tileFetcher, frame->getPriority(), import);
        }
        ident.importance = import;
    }
    
    // Clear out the individual frames, loads and all
    void clearFrames(NSMutableArray *toCancel,ChangeSet &changes) {
        for (auto frame : frames)
            frame->clear(toCancel, changes);
    }

    // Clear out geometry and all the frame info
    void clear(NSMutableArray *toCancel, ChangeSet &changes) {
        clearFrames(toCancel, changes);
        
        state = Waiting;
        for (auto drawID : instanceDrawIDs) {
            changes.push_back(new RemDrawableReq(drawID));
        }
        instanceDrawIDs.clear();
        
        shouldEnable = false;
    }
    
    // Set up the geometry for this tile
    void setupContents(MaplyQuadImageFrameLoader *loader,LoadedTileNewRef loadedTile,int defaultDrawPriority,SimpleIdentity shaderID,ChangeSet &changes) {
        drawPriority = defaultDrawPriority;
        for (auto di : loadedTile->drawInfo) {
            int newDrawPriority = defaultDrawPriority;
            switch (di.kind) {
                case WhirlyKit::LoadedTileNew::DrawableGeom:
                    newDrawPriority = defaultDrawPriority;
                    break;
                case WhirlyKit::LoadedTileNew::DrawableSkirt:
                    newDrawPriority = 11;
                    break;
                case WhirlyKit::LoadedTileNew::DrawablePole:
                    newDrawPriority = defaultDrawPriority;
                    break;
            }
            
            // Make a drawable instance to shadow the geometry
            auto drawInst = new BasicDrawableInstance("MaplyQuadImageFrameLoader", di.drawID, BasicDrawableInstance::LocalStyle);
            drawInst->setTexId(0, 0);
            drawInst->setDrawPriority(newDrawPriority);
            drawInst->setEnable(false);
            drawInst->setProgram(shaderID);
            drawInst->setColor([loader.color asRGBAColor]);
            if (loader->renderTarget)
                drawInst->setRenderTarget(loader->renderTarget.renderTargetID);
            changes.push_back(new AddDrawableReq(drawInst));
            instanceDrawIDs.push_back(drawInst->getId());
        }
    }
    
    // Cancel any outstanding fetches
    void cancelFetches(NSMutableArray *toCancel) {
        for (auto frame : frames) {
            frame->cancelFetch(toCancel);
        }
    }
    
    // A single frame loaded successfully
    void frameLoaded(MaplyLoaderReturn *loadReturn,Texture *tex,ChangeSet &changes) {
        if (loadReturn.frame < 0 || loadReturn.frame >= frames.size()) {
            NSLog(@"MaplyQuadImageFrameLoader: Got frame back outside of range.");
            delete tex;
            return;
        }
        
        auto frame = frames[loadReturn.frame];

        frame->loadSuccess(tex);
        
        changes.push_back(new AddTextureReq(tex));
    }
    
    // A single frame failed to load
    void frameFailed(MaplyLoaderReturn *loadReturn,ChangeSet &changes) {
        if (loadReturn.frame < 0 || loadReturn.frame >= frames.size()) {
            NSLog(@"MaplyQuadImageFrameLoader: Got frame back outside of range.");
            return;
        }
        auto frame = frames[loadReturn.frame];
        frame->loadFailed();
    }
    
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
    QIFTileState(int numFrames) : enable(false) { frames.resize(numFrames); }
    
    QuadTreeNew::Node node;
    
    // Set if this should be enabled
    bool enable;
    
    // The geometry used to represent the tile
    std::vector<SimpleIdentity> instanceDrawIDs;
    
    // Information about each frame
    class FrameInfo {
    public:
        FrameInfo() : texNode(0,0,-1), texID(EmptyIdentity) { }
        
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
    QIFRenderState() : lastCurFrame(-1.0), lastUpdate(0.0), lastRenderTime(0.0) { }
    QIFRenderState(int numFrames)
    {
        lastCurFrame = -1.0;
        lastUpdate = 0.0;
        tilesLoaded.resize(numFrames,0);
        topTilesLoaded.resize(numFrames,false);
    }
    
    std::map<QuadTreeNew::Node,QIFTileStateRef> tiles;
    
    // Number of tiles loaded for each frame
    std::vector<int> tilesLoaded;
    // Number of tiles at the lowest level loaded for each frame
    std::vector<bool> topTilesLoaded;
    
    bool hasUpdate(double curFrame) {
        // Current frame moved
        if (curFrame != lastCurFrame)
            return true;
        
        // We got an update from the layer thread
        if (lastUpdate > lastRenderTime)
            return true;
        
        return false;
    }
    
    double lastCurFrame;
    NSTimeInterval lastRenderTime;
    NSTimeInterval lastUpdate;
    
    // Update what the scene is looking at.  Ideally not every frame.
    void updateScene(Scene *scene,double curFrame,NSTimeInterval now,bool flipY,const RGBAColor &color,ChangeSet &changes) {
        if (tiles.empty())
            return;
        unsigned char color4[4];
        color.asUChar4(color4);
        
        lastRenderTime = now;
        lastCurFrame = curFrame;
        
        int activeFrames[2];
        activeFrames[0] = floor(curFrame);
        activeFrames[1] = ceil(curFrame);
        
        // Figure out how many valid frames we've got to look at
        int numFrames = 2;
        if (activeFrames[0] == activeFrames[1]) {
            numFrames = 1;
        }
        // Make sure we've got full coverage on those frames
        if (numFrames > 1 && topTilesLoaded[activeFrames[0]] && topTilesLoaded[activeFrames[1]]) {
            // We're good
        } else if (topTilesLoaded[activeFrames[1]]) {
            // Just one valid frame
            activeFrames[0] = activeFrames[1];
            numFrames = 1;
        } else {
            // Hunt for a good frame
            numFrames = 1;
            bool foundOne = false;
            for (int ii=0;ii<tilesLoaded.size();ii++) {
                int testFrame[2];
                testFrame[0] = activeFrames[0]-ii;
                testFrame[1] = activeFrames[0]+ii+1;
                for (int jj=0;jj<2;jj++) {
                    int theFrame = testFrame[jj];
                    if (theFrame >= 0 && theFrame < tilesLoaded.size()) {
                        if (topTilesLoaded[theFrame]) {
                            activeFrames[0] = theFrame;
                            foundOne = true;
                            break;
                        }
                    }
                }
                if (foundOne)
                    break;
            }
            
            if (!foundOne)
                numFrames = 0;
        }
        
        bool bigEnable = numFrames > 0;
        
//        NSLog(@"numFrames = %d, activeFrames[0] = %d, activeFrames[1] = %d",numFrames,activeFrames[0],activeFrames[1]);
        
        // Work through the tiles, figure out what's to be on and off
        for (auto tileIt : tiles) {
            auto tileID = tileIt.first;
            auto tile = tileIt.second;

            bool enable = bigEnable && tile->enable;
            if (enable) {
                // Assign as many active textures as we've got
                for (unsigned int ii=0;ii<numFrames;ii++) {
                    auto frame = tile->frames[activeFrames[ii]];
                    if (frame.texID != EmptyIdentity) {
                        int relLevel = tileID.level - frame.texNode.level;
                        int relX = tileID.x - frame.texNode.x * (1<<relLevel);
                        int tileIDY = tileID.y;
                        int frameIdentY = frame.texNode.y;
                        if (flipY) {
                            tileIDY = (1<<tileID.level)-tileIDY-1;
                            frameIdentY = (1<<frame.texNode.level)-frameIdentY-1;
                        }
                        int relY = tileIDY - frameIdentY * (1<<relLevel);
                        
                        for (auto drawID : tile->instanceDrawIDs)
                            changes.push_back(new DrawTexChangeRequest(drawID,ii,frame.texID,0,0,relLevel,relX,relY));
//                        NSLog(@"tile %d: (%d,%d), frame = %d getting texNode %d: (%d,%d texID = %d)",tileID.level,tileID.x,tileID.y,ii,frame.texNode.level,frame.texNode.x,frame.texNode.y,frame.texID);
                    } else {
                        enable = false;
                        break;
                    }
                }
                // Clear out the other texture if there is one
                if (numFrames == 1) {
                    for (auto drawID : tile->instanceDrawIDs) {
                        changes.push_back(new DrawTexChangeRequest(drawID,1,EmptyIdentity));
                    }
                }
                
                // Interpolate between two frames or snap to one
                double t = 0.0;
                if (numFrames > 1) {
                    t = curFrame-activeFrames[0];
                }
                
                // We set the interpolation value per drawable
                SingleVertexAttributeSet attrs;
                attrs.insert(SingleVertexAttribute(u_interpNameID,(float)t));
                attrs.insert(SingleVertexAttribute(u_colorNameID,color4));
                
                // Turn it all on
                for (auto drawID : tile->instanceDrawIDs) {
                    changes.push_back(new OnOffChangeRequest(drawID,true));
                    changes.push_back(new DrawUniformsChangeRequest(drawID,attrs));
                }
            }
            
            // Just turn the geometry off if we've got nothing
            if (!enable) {
                for (auto drawID : tile->instanceDrawIDs) {
                    changes.push_back(new OnOffChangeRequest(drawID,false));
                    changes.push_back(new DrawTexChangeRequest(drawID,0,EmptyIdentity));
                    changes.push_back(new DrawTexChangeRequest(drawID,1,EmptyIdentity));
                }
            }
        }
    }
};

}

using namespace WhirlyKit;

@implementation MaplyQuadImageFrameAnimator
{
    MaplyBaseViewController * __weak viewC;
    MaplyQuadImageFrameLoader * __weak loader;
    NSTimeInterval startTime;
    int numFrames;
}

- (instancetype)initWithFrameLoader:(MaplyQuadImageFrameLoader *)inLoader viewC:(MaplyBaseViewController * __nonnull)inViewC
{
    self = [super init];
    loader = inLoader;
    viewC = inViewC;
    startTime = CFAbsoluteTimeGetCurrent();
    _period = 10.0;
    _pauseLength = 0.0;
    numFrames = [loader getNumFrames];
    
    [viewC addActiveObject:self];
    
    return self;
}

- (void)shutdown
{
    [viewC removeActiveObject:self];
    loader = nil;
    viewC = nil;
}

// MARK: ActiveObject overrides

// Have to do the position update in the setCurrentImage so we're not messing with the rendering loop
- (bool)hasUpdate
{
    if (!viewC || !loader)
        return false;
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    NSTimeInterval totalPeriod = _period + _pauseLength;
    double when = fmod(now-startTime,totalPeriod);
    if (when >= _period)
        // Snap it to the end for a while
        [loader setCurrentImage:numFrames-1];
    else {
        double where = when/_period * (numFrames-1);
        [loader setCurrentImage:where];
    }

    return false;
}

- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo
{
}

- (void)teardown
{
    loader = nil;
}

@end

// An active updater called every frame the by the renderer
// We use this to process rendering state from the layer thread
@interface MaplyQuadImageFrameLoaderUpdater : MaplyActiveObject

@property (nonatomic,weak) MaplyQuadImageFrameLoader * __weak loader;

@end

@implementation MaplyQuadImageFrameLoaderUpdater

- (bool)hasUpdate
{
    return [_loader hasUpdate];
}

- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo
{
    return [_loader updateForFrame:frameInfo];
}

- (void)teardown
{
    _loader = nil;
}

@end

@implementation MaplyQuadImageFrameLoader
{
    bool valid;
    
    MaplySamplingParams *params;
    NSArray<NSObject<MaplyTileInfoNew> *> *frameInfos;
    
    // What part of the animation we're displaying
    double curFrame;

    // Tiles in various states of loading or loaded
    QIFTileAssetMap tiles;
    
    // Tile rendering info supplied from the layer thread
    QIFRenderState renderState;
    
    // Active updater used to updater rendering state
    MaplyQuadImageFrameLoaderUpdater *updater;
    
    bool changesSinceLastFlush;
}

- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)inParams tileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *__nonnull)inFrameInfos viewC:(MaplyBaseViewController * __nonnull)inViewC
{
    params = inParams;
    frameInfos = inFrameInfos;
    self->viewC = inViewC;
    
    if (!params.singleLevel) {
        NSLog(@"MaplyQuadImageFrameLoader only supports samplers with singleLevel set to true");
        return nil;
    }
    
    self.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault;
    self.drawPriorityPerLevel = 100;
    
    self = [super init];
    
    self.flipY = true;
    self.debugMode = false;
    self->minLevel = 10000;
    self->maxLevel = -1;
    for (MaplyRemoteTileInfoNew *frameInfo in frameInfos) {
        self->minLevel = std::min(self->minLevel,frameInfo.minZoom);
        self->maxLevel = std::max(self->maxLevel,frameInfo.maxZoom);
    }
    self.importanceScale = 1.0;
    self.importanceCutoff = 0.0;
    self.imageFormat = MaplyImageIntRGBA;
    self.borderTexel = 0;
    self.color = [UIColor whiteColor];
    self->texType = GL_UNSIGNED_BYTE;
    changesSinceLastFlush = true;
    valid = true;
    
    // Start things out after a delay
    // This lets the caller mess with settings
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!self->valid)
            return;
        
        if (!self->viewC || !self->viewC->renderControl || !self->viewC->renderControl->scene)
            return;
        
        if (!self->tileFetcher) {
            self->tileFetcher = [self->viewC addTileFetcher:MaplyQuadImageLoaderFetcherName];
        }
        if (!self->loadInterp) {
            self->loadInterp = [[MaplyImageLoaderInterpreter alloc] init];
        }
        
        self->samplingLayer = [self->viewC findSamplingLayer:inParams forUser:self];
        
        // They changed it, so make sure the cutoff still works
        if (self.importanceScale < 1.0)
            // Yeah, not a thing.
            self.importanceScale = 1.0;
        if (self.importanceScale != 1.0) {
            if (self.importanceCutoff == 0.0) {
                self.importanceCutoff = self->samplingLayer.params.minImportance * self.importanceScale;
            }
        }
        
        // Sort out the texture format
        switch (self.imageFormat) {
            case MaplyImageIntRGBA:
            case MaplyImage4Layer8Bit:
            default:
                self->texType = GL_UNSIGNED_BYTE;
                break;
            case MaplyImageUShort565:
                self->texType = GL_UNSIGNED_SHORT_5_6_5;
                break;
            case MaplyImageUShort4444:
                self->texType = GL_UNSIGNED_SHORT_4_4_4_4;
                break;
            case MaplyImageUShort5551:
                self->texType = GL_UNSIGNED_SHORT_5_5_5_1;
                break;
            case MaplyImageUByteRed:
            case MaplyImageUByteGreen:
            case MaplyImageUByteBlue:
            case MaplyImageUByteAlpha:
            case MaplyImageUByteRGB:
                self->texType = GL_ALPHA;
                break;
        }

        if (self->shaderID == EmptyIdentity) {
            self->shaderID = self->viewC->renderControl->scene->getProgramIDBySceneName(kToolkitDefaultTriangleMultiTex);
        }
    });
    
    return self;
}

- (void)setShader:(MaplyShader *)shader
{
    shaderID = [shader getShaderID];
}

- (void)setCurrentImage:(double)where
{
    curFrame = std::min(std::max(where,0.0),(double)([frameInfos count]-1));
}

- (int)getNumFrames
{
    return [frameInfos count];
}

- (void)shutdown
{
    valid = false;
    if (self->samplingLayer && self->samplingLayer.layerThread)
        [self performSelector:@selector(cleanup) onThread:self->samplingLayer.layerThread withObject:nil waitUntilDone:NO];

    [viewC removeActiveObject:updater];
    [viewC releaseSamplingLayer:samplingLayer forUser:self];
    
    params = nil;
    frameInfos = nil;
    updater = nil;
}

- (QIFTileAssetRef)addNewTile:(QuadTreeNew::ImportantNode)ident layer:(MaplyBaseInteractionLayer *)interactLayer toStart:(NSMutableArray *)toStart changes:(ChangeSet &)changes
{
    // Set up a new tile
    auto newTile = QIFTileAssetRef(new QIFTileAsset(ident,[frameInfos count]));
    int defaultDrawPriority = self.baseDrawPriority + self.drawPriorityPerLevel * ident.level;
    tiles[ident] = newTile;
    
    auto loadedTile = [builder getLoadedTile:ident];
    
    // Make the instance drawables we'll use to mirror the geometry
    if (loadedTile) {
        newTile->setupContents(self,loadedTile,defaultDrawPriority,shaderID,changes);
        newTile->setShouldEnable(loadedTile->enabled);
    }
    
    if ([self shouldLoad:ident]) {
        if (self.debugMode)
            NSLog(@"Starting fetch for tile %d: (%d,%d)",ident.level,ident.x,ident.y);
        newTile->startFetching(self, toStart, frameInfos);
    }
    
    return newTile;
}

- (void)removeTile:(QuadTreeNew::Node)ident layer:(MaplyBaseInteractionLayer *)interactLayer toCancel:(NSMutableArray *)toCancel changes:(ChangeSet &)changes
{
    auto it = tiles.find(ident);
    // If it's here, let's get rid of it.
    if (it != tiles.end()) {
        if (self.debugMode)
            NSLog(@"Unloading tile %d: (%d,%d)",ident.level,ident.x,ident.y);
        
        it->second->clear(toCancel, changes);

        tiles.erase(it);
    }
}

// Decide if this tile ought to be loaded
- (bool)shouldLoad:(QuadTreeNew::ImportantNode &)tile
{
    if (self.importanceCutoff == 0.0 || tile.importance >= self.importanceCutoff) {
        return true;
    }
    
    return false;
}

// Called on a random dispatch queue
- (void)fetchRequestSuccess:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame data:(NSData *)data
{
    if (self.debugMode)
        NSLog(@"MaplyQuadImageLoader: Got fetch back for tile %d: (%d,%d) frame %d",tileID.level,tileID.x,tileID.y,frame);
    
    // Ask the interpreter to parse it
    MaplyLoaderReturn *loadData = [[MaplyLoaderReturn alloc] init];
    loadData.tileID = tileID;
    loadData.frame = frame;
    loadData.tileData = data;
    [self performSelector:@selector(mergeFetchRequest:) onThread:self->samplingLayer.layerThread withObject:loadData waitUntilDone:NO];
}

// Called on the SamplingLayer.LayerThread
- (void)mergeFetchRequest:(MaplyLoaderReturn *)loadReturn
{
    QuadTreeNew::Node ident(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    auto it = tiles.find(ident);
    if (it == tiles.end() || loadReturn.error) {
        return;
    }
    auto tile = it->second;
    
    // Don't actually want this one
    if (!tile->isFrameLoading(loadReturn.frame))
        return;
    
    // Do the parsing on another thread since it can be slow
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self->loadInterp parseData:loadReturn];
        
        [self performSelector:@selector(mergeLoadedTile:) onThread:self->samplingLayer.layerThread withObject:loadReturn waitUntilDone:NO];
    });
}

// Called on the SamplingLayer.LayerThread
- (void)mergeLoadedTile:(MaplyLoaderReturn *)loadReturn
{
    QuadTreeNew::Node ident(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    auto it = tiles.find(ident);
    // Tile disappeared in the mean time, so drop it
    if (it == tiles.end() || loadReturn.error) {
        if (self.debugMode)
            NSLog(@"MaplyQuadImageLoader: Failed to load tile before it was erased %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);
        return;
    }
    auto tile = it->second;
    
    // Build the texture
    WhirlyKitLoadedTile *loadTile = [loadReturn.images count] > 0 ? [loadReturn.images objectAtIndex:0] : nil;
    Texture *tex = NULL;
    LoadedTileNewRef loadedTile = [builder getLoadedTile:ident];
    if ([loadTile.images count] > 0) {
        WhirlyKitLoadedImage *loadedImage = [loadTile.images objectAtIndex:0];
        if ([loadedImage isKindOfClass:[WhirlyKitLoadedImage class]]) {
            if (loadedTile) {
                // Build the image
                tex = [loadedImage buildTexture:self.borderTexel destWidth:loadedImage.width destHeight:loadedImage.height];
                tex->setFormat(texType);
            }
        }
    }

    ChangeSet changes;
    if (tex) {
        tile->frameLoaded(loadReturn, tex, changes);
    } else {
        tile->frameFailed(loadReturn, changes);
    }
    
    [layer.layerThread addChangeRequests:changes];
    
    changesSinceLastFlush = true;
}

// Called on SamplingLayer.layerThread
- (void)fetchRequestFail:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame error:(NSError *)error
{
    NSLog(@"MaplyQuadImageLoader: Failed to fetch tile %d: (%d,%d) frame %d because:\n%@",tileID.level,tileID.x,tileID.y,frame,[error localizedDescription]);
}

// Build up the drawing state for use on the main thread
// All the texture are assigned there
- (void)buildRenderState:(ChangeSet &)changes
{
    int numFrames = [frameInfos count];
    QIFRenderState newRenderState(numFrames);
    for (int frameID=0;frameID<numFrames;frameID++)
        newRenderState.topTilesLoaded[frameID] = true;
    
    // Work through the tiles, figure out their textures as we go
    for (auto tileIt : tiles) {
        auto tileID = tileIt.first;
        auto tile = tileIt.second;
        
        QIFTileStateRef tileState(new QIFTileState(numFrames));
        tileState->node = tileID;
        tileState->instanceDrawIDs = tile->getInstanceDrawIDs();
        tileState->enable = tile->getShouldEnable();
        
        // Work through the frames
        for (int frameID=0;frameID<numFrames;frameID++) {
            auto inFrame = tile->getFrame(frameID);
            auto &outFrame = tileState->frames[frameID];
            
            // Shouldn't happen
            if (!inFrame)
                continue;

            // Look for a tile or parent tile that has a texture ID
            QuadTreeNew::Node texNode = tile->getIdent();
            do {
                auto it = tiles.find(texNode);
                if (it == tiles.end())
                    break;
                auto parentTile = it->second;
                auto parentFrame = parentTile->getFrame(frameID);
                if (parentFrame && parentFrame->getTexID() != EmptyIdentity) {
                    // Got one, so stop
                    outFrame.texID = parentFrame->getTexID();
                    outFrame.texNode = parentTile->getIdent();
                    break;
                }
                
                // Work our way up the hierarchy
                if (texNode.level <= 0)
                    break;
                texNode.level -= 1;
                texNode.x /= 2;
                texNode.y /= 2;
            } while (outFrame.texID == EmptyIdentity);

            // Metrics for overall loading used by the display side
            if (outFrame.texID == EmptyIdentity && inFrame->getState() != QIFFrameAsset::Loaded) {
                if (tile->getIdent().level == minLevel)
                    newRenderState.topTilesLoaded[frameID] = false;
            } else {
                newRenderState.tilesLoaded[frameID]++;
            }
        }
        
        newRenderState.tiles[tileID] = tileState;
    }
    
//    if (true) {
//        std::string outStr = "";
//        for (int fi=0;fi<numFrames;fi++)
//            outStr += newRenderState.topTilesLoaded[fi] ? " 1" : " 0";
//        NSLog(@"MaplyQuadImageLoader topLoaded: %s",outStr.c_str());
//    }
    
    auto mergeReq = new RunBlockReq([self,newRenderState](Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
                                          {
                                              if (self)
                                                  self->renderState = newRenderState;
                                          });
    changes.push_back(mergeReq);
}

// MARK: Quad Build Delegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * _Nonnull)inBuilder layer:(WhirlyKitQuadDisplayLayerNew * _Nonnull)inLayer
{
    builder = inBuilder;
    layer = inLayer;

    dispatch_async(dispatch_get_main_queue(), ^{
        self->updater = [[MaplyQuadImageFrameLoaderUpdater alloc] init];
        self->updater.loader = self;
        [self->viewC addActiveObject:self->updater];
        
        [self setCurrentImage:self->curFrame];
    });
}

- (WhirlyKit::QuadTreeNew::NodeSet)quadBuilder:(WhirlyKitQuadTileBuilder * _Nonnull)builder loadTiles:(const WhirlyKit::QuadTreeNew::ImportantNodeSet &)loadTiles unloadTilesToCheck:(const WhirlyKit::QuadTreeNew::NodeSet &)unloadTiles targetLevel:(int)targetLevel
{
    QuadTreeNew::NodeSet toKeep;

    // List all the tiles that we're going to load or are loading
    QuadTreeNew::NodeSet allLoads;
    for (auto node : loadTiles)
        allLoads.insert(node);
    for (auto node : tiles)
        if (node.second->anyFramesLoading())
            allLoads.insert(node.first);
    
    // For all those loading or will be loading nodes, nail down their parents
    for (auto node : allLoads) {
        auto parent = node;
        while (parent.level > 0) {
            parent.level -= 1; parent.x /= 2;  parent.y /= 2;
            if (unloadTiles.find(parent) != unloadTiles.end())
            {
                auto it = tiles.find(parent);
                // Nail down the parent that's loaded, but don't care otherwise
                if (it != tiles.end() && it->second->anyFramesLoaded()) {
                    toKeep.insert(parent);
                    break;
                }
            }
        }
    }
    
    // Now check all the unloads to see if their parents are loading
    for (auto node : unloadTiles) {
        auto it = tiles.find(node);
        if (it == tiles.end())
            continue;
        // If this tile (to be unloaded) isn't loaded, then we don't care about it
        if (!it->second->anyFramesLoaded())
            continue;
        
        // Check that it's not already being kept around
        if (toKeep.find(node) == toKeep.end()) {
            auto parent = node;
            while (parent.level > 0) {
                parent.level -= 1; parent.x /= 2;  parent.y /= 2;
                if (allLoads.find(parent) != allLoads.end()) {
                    toKeep.insert(node);
                    break;
                }
            }
        }
    }

    return toKeep;
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder * _Nonnull)builder update:(const WhirlyKit::TileBuilderDelegateInfo &)updates changes:(WhirlyKit::ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;
    
    bool somethingChanged = false;
    
    NSMutableArray *toCancel = [NSMutableArray array];
    NSMutableArray *toStart = [NSMutableArray array];
    
    // Add new tiles
    for (auto it = updates.loadTiles.rbegin(); it != updates.loadTiles.rend(); ++it) {
        auto tile = *it;
        // If it's already there, clear it out
        [self removeTile:tile->ident layer:interactLayer toCancel:toCancel changes:changes];
        
        // Create the new tile and put in the toLoad queue
        auto newTile = [self addNewTile:tile->ident layer:interactLayer toStart:toStart changes:changes];
        somethingChanged = true;
    }
    
    // Remove old tiles
    for (auto inTile: updates.unloadTiles) {
        MaplyTileID tileID;
        tileID.level = inTile.level;
        tileID.x = inTile.x;
        tileID.y = inTile.y;
        
        auto it = tiles.find(inTile);
        // Don't know about this one.  Punt
        if (it == tiles.end())
            continue;
        auto tile = it->second;
        
        // Clear out any associated data and remove it from our list
        [self removeTile:inTile layer:interactLayer toCancel:toCancel changes:changes];
        somethingChanged = true;
    }
    
    // Look through the importance updates
    for (auto ident : updates.changeTiles) {
        auto it = tiles.find(ident);
        if (it == tiles.end())
            continue;
        auto tile = it->second;
        auto loadedTile = [builder getLoadedTile:ident];

        tile->setImportance(tileFetcher, ident.importance);

        // Note: We used to have some convoluted importance logic here.  Didn't work out.
        somethingChanged = true;
    }
    
    [tileFetcher cancelTileFetches:toCancel];
    [tileFetcher startTileFetches:toStart];
    
    if (self.debugMode)
        NSLog(@"quadBuilder:updates:changes: changeRequests: %d",(int)changes.size());
    
    changesSinceLastFlush |= somethingChanged;
}

- (void)quadBuilderPreSceneFlush:(WhirlyKitQuadTileBuilder * _Nonnull)builder
{
    if (!changesSinceLastFlush)
        return;
    
    ChangeSet changes;
    [self buildRenderState:changes];
    
    [layer.layerThread addChangeRequests:changes];
    
    changesSinceLastFlush = false;
}

- (void)quadBuilderShutdown:(WhirlyKitQuadTileBuilder * _Nonnull)builder
{
}

- (void)cleanup
{
    ChangeSet changes;
    
    NSMutableArray *toCancel = [NSMutableArray array];
    for (auto tile : tiles) {
        tile.second->clear(toCancel, changes);
    }
    tiles.clear();
    
    [tileFetcher cancelTileFetches:toCancel];
    
    [layer.layerThread addChangeRequests:changes];
}

// MARK: Active Object methods (called by updater)
- (bool)hasUpdate
{
    return renderState.hasUpdate(curFrame);
}

- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo
{
    if (!renderState.hasUpdate(curFrame))
        return;
    
    ChangeSet changes;
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    renderState.updateScene(frameInfo.scene, curFrame, now, self.flipY, [self.color asRGBAColor], changes);
    
    frameInfo.scene->addChangeRequests(changes);
}

@end
