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

@class MaplyQuadImageFrameLoader;

namespace WhirlyKit
{

// Assets and status associated with a single tile's frame
class FrameAsset
{
public:
    typedef enum {Empty,Loaded,Loading} State;
    
    State getState() { return state; }

    void clear(NSObject<MaplyTileFetcher> *tileFetcher,ChangeSet &changes) {
        
    }
    
    void startFetching(NSObject<MaplyTileFetcher> *tileFetcher) {
        
    }
    
    void updateFetching(NSObject<MaplyTileFetcher> *tileFetcher,int priority,double importance) {
        
    }
    
    void loadSuccess() {
        
    }
    
    void loadFailed() {
        
    }
    
protected:
    State state;

    // Returned by the TileFetcher
    id fetchHandle;
    int fetchPriority;
    double fetchImportance;
    
    // If set, the texture ID for this asset
    SimpleIdentity texID;
};
    
typedef std::shared_ptr<FrameAsset> FrameAssetRef;
    
class TileAsset
{
public:
    
    typedef enum {Waiting,Active} State;
    
    State getState() { return state; }
    
    // Importance value changed, so update the fetcher
    void setImportance(NSObject<MaplyTileFetcher> *tileFetcher,double import) {
        
    }
    
    void clear(NSObject<MaplyTileFetcher> *tileFetcher, ChangeSet &changes) {
        
    }
    
    void clearFrames(NSObject<MaplyTileFetcher> *tileFetcher, ChangeSet &changes) {
        
    }
    
    void setupContents(LoadedTileNewRef loadedTile,int defaultDrawPriority,SimpleIdentity shaderID,ChangeSet &changes) {
        
    }
    
    // Fetch the tile frames.  Just fetch them all for now.
    void startFetching(MaplyQuadImageFrameLoader *loader,NSObject<MaplyTileFetcher> *tileFetcher,NSArray<NSObject<MaplyTileInfoNew> *> *frameInfos) {
        state = Active;
        
        NSMutableArray *requests = [NSMutableArray array];
        
        int frame = 0;
        for (MaplyRemoteTileInfoNew *frameInfo in frameInfos) {
            MaplyTileFetchRequest *request = [[MaplyTileFetchRequest alloc] init];
            MaplyTileID tileID;  tileID.level = ident.level;  tileID.x = ident.x;  tileID.y = ident.y;
            id fetchInfo = [frameInfo fetchInfoForTile:tileID];
            request.fetchInfo = fetchInfo;
            request.tileSource = frameInfo;
            request.priority = 0;
            request.importance = ident.importance * loader.importanceScale;
            
            request.success = ^(MaplyTileFetchRequest *request, NSData *data) {
                [loader fetchRequestSuccess:request tileID:tileID frame:frame data:data];
            };
            request.failure = ^(MaplyTileFetchRequest *request, NSError *error) {
                [loader fetchRequestFail:request tileID:tileID frame:frame error:error];
            };
            [requests addObject:request];
            
            frame++;
        }
        
        tile->startFetch(tileFetcher,requests);
    }
    
    void cancelFetches(NSObject<MaplyTileFetcher> *tileFetcher) {
        
    }
    
    // Enable the instanced geometry
    void enableTile(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        shouldEnable = true;
        enable = true;
        for (auto drawID : instanceDrawIDs)
            changes.push_back(new OnOffChangeRequest(drawID,true));
    }
    
    // Disable instanced geometry
    void disableTile(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        shouldEnable = false;
        enable = false;
        for (auto drawID : instanceDrawIDs)
            changes.push_back(new OnOffChangeRequest(drawID,false));
    }
    
protected:
    State state;
    QuadTreeNew::ImportantNode ident;
    
    std::vector<SimpleIdentity> instanceDrawIDs;
    
    std::vector<FrameAssetRef> frames;
    
    int drawPriority;
    
    bool shouldEnable;
    bool enable;
};
    
typedef std::shared_ptr<TileAsset> TileAssetRef;
typedef std::map<QuadTreeNew::Node,TileAssetRef> TileAssetMap;
}

using namespace WhirlyKit;

@interface MaplyQuadImageFrameLoader() <WhirlyKitQuadTileBuilderDelegate>
@end

@implementation MaplyQuadImageFrameLoader
{
    MaplySamplingParams *params;
    NSArray<NSObject<MaplyTileInfoNew> *> *frameInfos;

    // Tiles in various states of loading or loaded
    TileAssetMap tiles;
    
    SimpleIdentity shaderID;
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
    self->texType = GL_UNSIGNED_BYTE;
    
    // Start things out after a delay
    // This lets the caller mess with settings
    dispatch_async(dispatch_get_main_queue(), ^{
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
    // Note: Fill me in
}

- (void)shutdown
{
    [viewC releaseSamplingLayer:samplingLayer forUser:self];
}

- (TileAssetRef)addNewTile:(QuadTreeNew::ImportantNode)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    // Set up a new tile
    auto newTile = TileAssetRef(new TileAsset());
    int defaultDrawPriority = self.baseDrawPriority + self.drawPriorityPerLevel * ident.level;
    tiles[ident] = newTile;
    
    auto loadedTile = [builder getLoadedTile:ident];
    
    // Make the instance drawables we'll use to mirror the geometry
    if (loadedTile)
        newTile->setupContents(loadedTile,defaultDrawPriority,shaderID,changes);
    
    if ([self shouldLoad:ident]) {
        if (self.debugMode)
            NSLog(@"Starting fetch for tile %d: (%d,%d)",ident.level,ident.x,ident.y);
        newTile->startFetching(tileFetcher);
    }
    
    return newTile;
}

- (void)removeTile:(QuadTreeNew::Node)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    auto it = tiles.find(ident);
    // If it's here, let's get rid of it.
    if (it != tiles.end()) {
        if (self.debugMode)
            NSLog(@"Unloading tile %d: (%d,%d)",ident.level,ident.x,ident.y);
        
        it->second->cancelFetches(tileFetcher);
        if (self.debugMode)
            NSLog(@"Cancelled loading %d: (%d,%d)",ident.level,ident.x,ident.y);

        it->second->clear(tileFetcher, changes);
        tiles.erase(it);
    }
}

- (void)enableTile:(QuadTreeNew::Node)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    auto it = tiles.find(ident);
    // Don't know about this one.  Punt
    if (it == tiles.end())
        return;
    
    auto tile = it->second;
    tile->enableTile(interactLayer,changes);
}

- (void)disableTile:(QuadTreeNew::Node)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    auto it = tiles.find(ident);
    // Don't know about this one.  Punt
    if (it == tiles.end())
        return;
    
    auto tile = it->second;
    tile->disableTile(interactLayer,changes);
}

// Decide if this tile ought to be loaded
- (bool)shouldLoad:(QuadTreeNew::ImportantNode &)tile
{
    if (self.importanceCutoff == 0.0 || tile.importance >= self.importanceCutoff) {
        return true;
    }
    
    return false;
}

// MARK: Quad Build Delegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * _Nonnull)inBuilder layer:(WhirlyKitQuadDisplayLayerNew * _Nonnull)inLayer
{
    builder = inBuilder;
    layer = inLayer;
}

- (WhirlyKit::QuadTreeNew::NodeSet)quadBuilder:(WhirlyKitQuadTileBuilder * _Nonnull)builder loadTiles:(const WhirlyKit::QuadTreeNew::ImportantNodeSet &)loadTiles unloadTilesToCheck:(const WhirlyKit::QuadTreeNew::NodeSet &)unloadTiles
{
    QuadTreeNew::NodeSet toKeep;

    // Note: Fill this in
    
    return toKeep;
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder * _Nonnull)builder update:(const WhirlyKit::TileBuilderDelegateInfo &)updates changes:(WhirlyKit::ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;
    
    // Add new tiles
    for (auto it = updates.loadTiles.rbegin(); it != updates.loadTiles.rend(); ++it) {
        auto tile = *it;
        // If it's already there, clear it out
        [self removeTile:tile->ident layer:interactLayer changes:changes];
        
        // Create the new tile and put in the toLoad queue
        auto newTile = [self addNewTile:tile->ident layer:interactLayer changes:changes];
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
        [self removeTile:inTile layer:interactLayer changes:changes];
    }
    
    // Look through the importance updates
    for (auto ident : updates.changeTiles) {
        auto it = tiles.find(ident);
        if (it == tiles.end())
            continue;
        auto tile = it->second;
        
        // We consider it worth loading
        if ([self shouldLoad:ident]) {
            // If it isn't loaded, then start that process
            if (tile->getState() == TileAsset::Waiting) {
                if (self.debugMode)
                    NSLog(@"Tile switched from Wait to Fetch %d: (%d,%d) importance = %f",ident.level,ident.x,ident.y,ident.importance);
                tile->startFetching(tileFetcher);
            }
        } else {
            // We don't consider it worth loading now so drop it if we were
            switch (tile->getState())
            {
                case TileAsset::Waiting:
                    // this is fine
                    break;
                case TileAsset::Active:
                    tile->cancelFetches(tileFetcher);
                    tile->clearFrames(tileFetcher, changes);
                    break;
            }
        }
    }
    
    for (auto tile: updates.enableTiles)
        [self enableTile:tile->ident layer:interactLayer changes:changes];
    
    for (auto tile: updates.disableTiles)
        [self disableTile:tile->ident layer:interactLayer changes:changes];
    
    if (self.debugMode)
        NSLog(@"quadBuilder:updates:changes: changeRequests: %d",(int)changes.size());
}

- (void)quadBuilderPreSceneFlush:(WhirlyKitQuadTileBuilder * _Nonnull)builder
{
}

@end
