/*
 *  MaplyQuadImageLoader.mm
 *
 *  Created by Steve Gifford on 4/10/18.
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

#import "MaplyQuadImageLoader_private.h"
#import "QuadTileBuilder.h"
#import "MaplyImageTile_private.h"
#import "MaplyRenderController_private.h"
#import "MaplyQuadSampler_private.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyRenderTarget_private.h"

@interface MaplyQuadImageLoader() <WhirlyKitQuadTileBuilderDelegate>

@end

namespace WhirlyKit
{

class TileAsset;
typedef std::shared_ptr<TileAsset> TileAssetRef;

// Keep track of what we've already loaded
class TileAsset
{
public:
    TileAsset() :
        ourTexture(false), texID(EmptyIdentity), drawPriority(0), compObjs(nil), shouldEnable(false), enable(false), state(Waiting)
    { }

    // Tile is doing what?
    typedef enum {Waiting,Loading,Loaded} State;
    State getState() { return state; }
    bool isOurTexture() { return ourTexture; }

    // Completely clear out the tile geometry
    void clear(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        clearToBlank(interactLayer, changes, Waiting);
        for (auto drawID : instanceDrawIDs)
            changes.push_back(new RemDrawableReq(drawID));
        instanceDrawIDs.clear();
    }

    // Clear out the assets unique to this tile
    void clearToBlank(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes,State newState) {
        state = newState;
        if (ourTexture && texID != EmptyIdentity)
            changes.push_back(new RemTextureReq(texID));
        if (compObjs)
            [interactLayer removeObjects:compObjs changes:changes];
        texNode = QuadTreeNew::Node(-1,-1,-1);
        texID = EmptyIdentity;
        ourTexture = false;
        compObjs = nil;
    }
    
    // Return the texture ID
    SimpleIdentity getTexID() { return texID; }
    
    // This is what the master tile thinks
    bool getShouldEnable() { return shouldEnable; }
    
    // Set when we decide children are loading
    bool areChildrenLoading() { return childrenLoading; }
    void setChildrenLoading(bool newVal) { childrenLoading = newVal; }

    // Assign a borrowed texture from another node
    void setBorrowedTexture(QuadTreeNew::Node borrowNode,SimpleIdentity inTexID)
    {
        ourTexture = false;
        texNode = borrowNode;
        texID = inTexID;
    }
    
    // Check if this tile is using another cover texture
    bool isUsingCoverTex() {
        return !ourTexture && texID != EmptyIdentity;
    }
    
    // Return the cover texture node
    QuadTreeNew::Node getCoverTexNode() {
        return texNode;
    }
    
    // Check if this node is using a borrowed texture from the given one
    bool isUsingTexFromNode(QuadTreeNew::Node borrowNode) {
        if (ourTexture)
            return false;
        return texNode == borrowNode;
    }
 
    // Kick off the request and keep track of the handle for later
    void startFetch(NSObject<MaplyTileFetcher> *tileFetcher,NSArray<MaplyTileFetchRequest *> *requests) {
        state = Loading;
        for (MaplyTileFetchRequest *request in requests)
            fetchHandles.push_back(request);
        [tileFetcher startTileFetches:requests];
    }

    // Stop trying to make Fetch happen
    void cancelFetch(NSObject<MaplyTileFetcher> *tileFetcher) {
        NSMutableArray *toCancel = [NSMutableArray array];
        for (unsigned int ii=0;ii<fetchHandles.size();ii++)
            if (fetchHandles[ii]) {
                [toCancel addObject:fetchHandles[ii]];
                fetchHandles[ii] = nil;
            }
        [tileFetcher cancelTileFetches:toCancel];
        state = Waiting;
    }
    
    // Called after a completed load
    bool setHasLoaded(MaplyTileFetchRequest *request,NSData *loadedData) {
        bool anyLoading = false;
        int which = -1;
        for (unsigned int ii=0;ii<fetchHandles.size();ii++) {
            if (fetchHandles[ii] == request) {
                which = ii;
                fetchHandles[ii] = nil;
            }
            if (fetchHandles[ii])
                anyLoading = true;
        }
        if (!anyLoading) {
            state = Loaded;
        }
        
        // Keep track of the data in case we're loading multiple sets
        if (which >= 0) {
            if (which >= fetchedData.size())
                fetchedData.resize(which+1,nil);
            fetchedData[which] = loadedData;
        }
        
        return !anyLoading;
    }
    
    // Return the list of data fetched and clear it locally
    std::vector<NSData *> getAndClearData() {
        auto toRet = fetchedData;
        fetchedData.clear();
        return toRet;
    }
    
    // After a successful load, set up the texture and any other contents
    void setupContents(LoadedTileNewRef loadedTile,Texture *tex,NSArray *inCompObjs,MaplyBaseInteractionLayer *layer,ChangeSet &changes) {
        shouldEnable = loadedTile->enabled;
        compObjs = inCompObjs;

        if (tex) {
            // Create the texture in the renderer
            ourTexture = true;
            texID = tex->getId();
            changes.push_back(new AddTextureReq(tex));
            
            // Assign it to the various drawables
            for (int which = 0; which < loadedTile->drawInfo.size(); which++) {
                int drawInstID = instanceDrawIDs[which];
                
                // Draw priority for thie asset when it's loaded
                int newDrawPriority = drawPriority;
                switch (loadedTile->drawInfo[which].kind) {
                    case WhirlyKit::LoadedTileNew::DrawableGeom:
                        newDrawPriority = drawPriority;
                        break;
                    case WhirlyKit::LoadedTileNew::DrawableSkirt:
                        newDrawPriority = 11;
                        break;
                    case WhirlyKit::LoadedTileNew::DrawablePole:
                        newDrawPriority = drawPriority;
                        break;
                }
                changes.push_back(new DrawTexChangeRequest(drawInstID,0,texID));
                changes.push_back(new DrawPriorityChangeRequest(drawInstID,newDrawPriority));

                if (shouldEnable)
                    changes.push_back(new OnOffChangeRequest(drawInstID,true));
            }

            if (shouldEnable) {
                enable = true;
                [layer enableObjects:compObjs changes:changes];
            }
        }
    }
    
    // Set up the instance to the base tile's geometry
    void setupGeom(MaplyQuadImageLoader *loader,LoadedTileNewRef loadedTile,int defaultDrawPriority,ChangeSet &changes) {
        // Assign it to the various drawables
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
            auto drawInst = new BasicDrawableInstance("MaplyQuadImageLoader", di.drawID, BasicDrawableInstance::LocalStyle);
            drawInst->setTexId(0, 0);
            drawInst->setDrawPriority(newDrawPriority);
            drawInst->setEnable(false);
            drawInst->setColor([loader.color asRGBAColor]);
            if (loader->renderTarget)
                drawInst->setRenderTarget(loader->renderTarget.renderTargetID);
            changes.push_back(new AddDrawableReq(drawInst));
            instanceDrawIDs.push_back(drawInst->getId());
        }
    }
    
    // Is this one a better cover tile than what we have?
    bool isBetterCoverTile(const QuadTreeNew::Node &coverIdent,const QuadTreeNew::Node &thisIdent) {
        if (coverIdent.level >= thisIdent.level)
            return false;
        if (ourTexture)
            return false;
        
        // Has to be above us
        int relLevel = thisIdent.level - coverIdent.level;
        int relX = thisIdent.x - coverIdent.x * (1<<relLevel), relY = thisIdent.y - coverIdent.y * (1<<relLevel);
        if (!(relX >= 0 && relY >= 0 && relX < 1<<relLevel && relY < 1<<relLevel))
            return false;
        
        if (texID == EmptyIdentity) {
            return true;
        } else {
            // Not the same one we have
            if (texNode == coverIdent)
                return false;
            // Has to be better than what we've got
            return texNode.level < coverIdent.level;
        }
        return false;
    }
    
    // Apply texture from a cover tile to this one
    void applyCoverTile(const QuadTreeNew::Node &coverIdent,TileAssetRef coverAsset,LoadedTileNewRef loadedTile,bool flipY,ChangeSet &changes) {
        int relLevel = loadedTile->ident.level - coverIdent.level;
        int loadedTileY = loadedTile->ident.y;
        int coverIdentY = coverIdent.y;
        if (flipY) {
            loadedTileY = (1<<loadedTile->ident.level)-loadedTileY-1;
            coverIdentY = (1<<coverIdent.level)-coverIdentY-1;
        }
        int relX = loadedTile->ident.x - coverIdent.x * (1<<relLevel), relY = loadedTileY - coverIdentY * (1<<relLevel);

        texNode = coverIdent;
        ourTexture = false;
        texID = coverAsset->texID;
        for (int which = 0; which < loadedTile->drawInfo.size(); which++)
        {
            int drawInstID = instanceDrawIDs[which];
            // Need to use the draw priority of the cover asset so we can interleave geometry
            int newDrawPriority = coverAsset->drawPriority;
            switch (loadedTile->drawInfo[which].kind) {
                case WhirlyKit::LoadedTileNew::DrawableGeom:
                    newDrawPriority = coverAsset->drawPriority;
                    break;
                case WhirlyKit::LoadedTileNew::DrawableSkirt:
                    newDrawPriority = 11;
                    break;
                case WhirlyKit::LoadedTileNew::DrawablePole:
                    newDrawPriority = coverAsset->drawPriority;
                    break;
            }
            changes.push_back(new DrawTexChangeRequest(drawInstID,0,texID,0,0,relLevel,relX,relY));
            changes.push_back(new DrawPriorityChangeRequest(drawInstID,newDrawPriority));
            if (loadedTile->enabled)
                changes.push_back(new OnOffChangeRequest(drawInstID,true));
        }
    }

    // Enable the instanced geometry and comp objects
    void enableTile(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        shouldEnable = true;
        enable = true;
        for (auto drawID : instanceDrawIDs)
            changes.push_back(new OnOffChangeRequest(drawID,true));
        [interactLayer enableObjects:compObjs changes:changes];
    }
    
    // Disable instanced geometry and comp objects
    void disableTile(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        shouldEnable = false;
        enable = false;
        for (auto drawID : instanceDrawIDs)
            changes.push_back(new OnOffChangeRequest(drawID,false));
        [interactLayer disableObjects:compObjs changes:changes];
    }
    
protected:
    State state;
    bool childrenLoading;
    
    // Tile ID of the texture we're applying to this tile.
    // Might be a lower resolution tile.
    QuadTreeNew::Node texNode;
    
    // IDs of the instances we created to shadow the geometry
    std::vector<SimpleIdentity> instanceDrawIDs;

    // The texture ID owned by this node.  Delete it when we're done.
    bool ourTexture;
    SimpleIdentity texID;
    
    // Draw Priority assigned to this tile by default
    int drawPriority;
    
    // Set if the Tile Builder thinks this should be enabled
    bool shouldEnable;
    bool enable;
    
    // Component objects owned by the tile
    NSArray *compObjs;
    
    // Handle returned by the fetcher while it's fetching
    std::vector<id> fetchHandles;
    
    // Data returned by the fetcher, in case we have more than one tile info
    std::vector<NSData *> fetchedData;
};

typedef std::map<QuadTreeNew::Node,TileAssetRef> TileAssetMap;
}

using namespace WhirlyKit;

@implementation MaplyLoaderReturn

- (id)init
{
    self = [super init];
    _frame = -1;
    
    return self;
}

@end

@implementation MaplyImageLoaderInterpreter

- (void)parseData:(MaplyLoaderReturn * __nonnull)loadReturn
{
    // Create the image and tie it to the drawables
    MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:loadReturn.tileData];
    // Note: Deal with border pixels
    int borderPixel = 0;
    WhirlyKitLoadedTile *loadTile = [tileData wkTile:borderPixel convertToRaw:true];
    loadReturn.image = loadTile;
}

@end

@implementation MaplyQuadImageLoaderBase

- (void)setTileFetcher:(NSObject<MaplyTileFetcher> * __nonnull)inTileFetcher
{
    if (tileFetcher) {
        NSLog(@"Caller tried to set tile fetcher after startup in MaplyQuadImageLoader.  Ignoring.");
        return;
    }
    
    tileFetcher = inTileFetcher;
}

- (void)setRenderTarget:(MaplyRenderTarget *__nonnull)inRenderTarget
{
    renderTarget = inRenderTarget;
}

- (void)setInterpreter:(NSObject<MaplyLoaderInterpreter> * __nonnull)interp
{
    if (loadInterp) {
        NSLog(@"Caller tried to set loader interpreter after startup in MaplyQuadImageLoader.  Ignoring.");
        return;
    }
    
    loadInterp = interp;
}

- (MaplyBoundingBox)geoBoundsForTile:(MaplyTileID)tileID
{
    if (!layer || !layer.quadtree)
        return kMaplyNullBoundingBox;
    
    MaplyBoundingBox bounds;
    MaplyBoundingBoxD boundsD = [self geoBoundsForTileD:tileID];
    bounds.ll = MaplyCoordinateMake(boundsD.ll.x,boundsD.ll.y);
    bounds.ur = MaplyCoordinateMake(boundsD.ur.x,boundsD.ur.y);
    
    return bounds;
}

- (MaplyBoundingBoxD)geoBoundsForTileD:(MaplyTileID)tileID
{
    WhirlyKitQuadDisplayLayerNew *thisQuadLayer = layer;
    if (!layer || !layer.quadtree)
        return kMaplyNullBoundingBoxD;
    
    MaplyBoundingBoxD bounds;
    MbrD mbrD = thisQuadLayer.quadtree->generateMbrForNode(WhirlyKit::QuadTreeNew::Node(tileID.x,tileID.y,tileID.level));
    
    CoordSystem *wkCoordSys = thisQuadLayer.coordSys;
    Point2d pts[4];
    pts[0] = wkCoordSys->localToGeographicD(Point3d(mbrD.ll().x(),mbrD.ll().y(),0.0));
    pts[1] = wkCoordSys->localToGeographicD(Point3d(mbrD.ur().x(),mbrD.ll().y(),0.0));
    pts[2] = wkCoordSys->localToGeographicD(Point3d(mbrD.ur().x(),mbrD.ur().y(),0.0));
    pts[3] = wkCoordSys->localToGeographicD(Point3d(mbrD.ll().x(),mbrD.ur().y(),0.0));
    Point2d minPt(pts[0].x(),pts[0].y()),  maxPt(pts[0].x(),pts[0].y());
    for (unsigned int ii=1;ii<4;ii++)
    {
        minPt.x() = std::min(minPt.x(),pts[ii].x());
        minPt.y() = std::min(minPt.y(),pts[ii].y());
        maxPt.x() = std::max(maxPt.x(),pts[ii].x());
        maxPt.y() = std::max(maxPt.y(),pts[ii].y());
    }
    bounds.ll = MaplyCoordinateDMake(minPt.x(), minPt.y());
    bounds.ur = MaplyCoordinateDMake(maxPt.x(), maxPt.y());
    
    return bounds;
}

- (MaplyBoundingBox)boundsForTile:(MaplyTileID)tileID
{
    MaplyBoundingBox bounds;
    MaplyBoundingBoxD boundsD;
    
    boundsD = [self boundsForTileD:tileID];
    bounds.ll = MaplyCoordinateMake(boundsD.ll.x, boundsD.ll.y);
    bounds.ur = MaplyCoordinateMake(boundsD.ur.x, boundsD.ur.y);
    
    return bounds;
}

- (MaplyBoundingBoxD)boundsForTileD:(MaplyTileID)tileID
{
    WhirlyKitQuadDisplayLayerNew *thisQuadLayer = layer;
    if (!layer || !layer.quadtree)
        return kMaplyNullBoundingBoxD;
    
    MaplyBoundingBoxD bounds;
    
    MbrD mbrD = thisQuadLayer.quadtree->generateMbrForNode(WhirlyKit::QuadTreeNew::Node(tileID.x,tileID.y,tileID.level));
    bounds.ll = MaplyCoordinateDMake(mbrD.ll().x(), mbrD.ll().y());
    bounds.ur = MaplyCoordinateDMake(mbrD.ur().x(), mbrD.ur().y());
    
    return bounds;
}

@end

NSString * const MaplyQuadImageLoaderFetcherName = @"QuadImageLoader";

@implementation MaplyQuadImageLoader
{
    MaplySamplingParams *params;
    NSArray<MaplyRemoteTileInfoNew *> *tileInfos;
    
    // Tiles in various states of loading or loaded
    TileAssetMap tiles;
}

- (instancetype)initWithParams:(MaplySamplingParams *)params tileInfo:(MaplyRemoteTileInfoNew *)tileInfo viewC:(MaplyBaseViewController *)viewC
{
    return [self initWithParams:params tileInfos:@[tileInfo] viewC:viewC];
}

- (nullable instancetype)initWithParams:(MaplySamplingParams *)inParams tileInfos:(NSArray<MaplyRemoteTileInfoNew *> *__nonnull)inTileInfos viewC:(MaplyBaseViewController * __nonnull)inViewC
{
    params = inParams;
    tileInfos = inTileInfos;
    self->viewC = inViewC;

    self.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault;
    self.drawPriorityPerLevel = 100;

    self = [super init];

    self.flipY = true;
    self.debugMode = false;
    self->minLevel = 10000;
    self->maxLevel = -1;
    for (MaplyRemoteTileInfoNew *tileInfo in tileInfos) {
        self->minLevel = std::min(self->minLevel,tileInfo.minZoom);
        self->maxLevel = std::max(self->maxLevel,tileInfo.maxZoom);
    }
    self.importanceScale = 1.0;
    self.importanceCutoff = 0.0;
    self.imageFormat = MaplyImageIntRGBA;
    self.borderTexel = 0;
    self.color = [UIColor whiteColor];
    self->texType = GL_UNSIGNED_BYTE;

    // Start things out after a delay
    // This lets the caller mess with settings
    dispatch_async(dispatch_get_main_queue(), ^{
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
    });

    return self;
}

- (void)removeTile:(QuadTreeNew::Node)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    auto it = tiles.find(ident);
    // If it's here, let's get rid of it.
    if (it != tiles.end()) {
        if (self.debugMode)
            NSLog(@"Unloading tile %d: (%d,%d)",ident.level,ident.x,ident.y);

        if (it->second->getState() == TileAsset::Loading) {
            if (self.debugMode)
                NSLog(@"Cancelled loading %d: (%d,%d)",ident.level,ident.x,ident.y);
            
            it->second->cancelFetch(tileFetcher);
        }
        it->second->clear(interactLayer, changes);
        tiles.erase(it);
    }
}

// Go get this tile
- (void)fetchThisTile:(TileAssetRef)tile ident:(QuadTreeNew::ImportantNode)ident
{
    if (self.debugMode)
        NSLog(@"Starting fetch for tile %d: (%d,%d)",ident.level,ident.x,ident.y);
    
    NSMutableArray *requests = [NSMutableArray array];
    
    for (MaplyRemoteTileInfoNew *tileInfo in tileInfos) {
        if (ident.level >= tileInfo.minZoom && ident.level <= tileInfo.maxZoom) {
            // Put together a request for the fetcher
            MaplyTileFetchRequest *request = [[MaplyTileFetchRequest alloc] init];
            MaplyTileID tileID;  tileID.level = ident.level;  tileID.x = ident.x;  tileID.y = ident.y;
            id fetchInfo = [tileInfo fetchInfoForTile:tileID];
            request.fetchInfo = fetchInfo;
            request.tileSource = tileInfo;
            request.priority = 0;
            request.importance = ident.importance * self.importanceScale;
            
            request.success = ^(MaplyTileFetchRequest *request, NSData *data) {
                [self fetchRequestSuccess:request tileID:tileID frame:-1 data:data];
            };
            request.failure = ^(MaplyTileFetchRequest *request, NSError *error) {
                [self fetchRequestFail:request tileID:tileID frame:-1 error:error];
            };
            [requests addObject:request];
        }
    }

    tile->startFetch(tileFetcher,requests);
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
        newTile->setupGeom(self,loadedTile,defaultDrawPriority,changes);

    if ([self shouldLoad:ident])
        [self fetchThisTile:newTile ident:ident];

    [self findCoverTile:ident changes:changes];
    
    return newTile;
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

// Called on a random dispatch queue
- (void)fetchRequestSuccess:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame data:(NSData *)data
{
    if (self.debugMode)
        NSLog(@"MaplyQuadImageLoader: Got fetch back for tile %d: (%d,%d)",tileID.level,tileID.x,tileID.y);

    // Ask the interpreter to parse it
    MaplyLoaderReturn *loadData = [[MaplyLoaderReturn alloc] init];
    loadData.tileID = tileID;
    loadData.frame = frame;
    loadData.tileData = data;
    [self performSelector:@selector(mergeFetchRequest:) onThread:self->samplingLayer.layerThread withObject:@[loadData,request] waitUntilDone:NO];
}

// Called on the SamplingLayer.LayerThread
- (void)mergeFetchRequest:(NSArray *)retData
{
    MaplyLoaderReturn *loadReturn = [retData objectAtIndex:0];
    MaplyTileFetchRequest *request = [retData objectAtIndex:1];

    QuadTreeNew::Node ident(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    auto it = tiles.find(ident);
    // Failed, so clean up the objects that may have been created
    if (it == tiles.end() || it->second->getState() != TileAsset::Loading || loadReturn.error) {
        if (loadReturn.compObjs) {
            [viewC removeObjects:loadReturn.compObjs mode:MaplyThreadCurrent];
        }
        return;
    }
    auto tile = it->second;

    // Loaded all the requests, so parse the data
    if (tile->setHasLoaded(request,loadReturn.tileData)) {
        // Construct a LoaderReturn that contains all the data
        auto allData = tile->getAndClearData();
        if (allData.empty())
            return;
        
        MaplyLoaderReturn *multiLoadData = [[MaplyLoaderReturn alloc] init];
        multiLoadData.tileID = loadReturn.tileID;
        multiLoadData.frame = loadReturn.frame;
        multiLoadData.multiTileData = [NSMutableArray array];
        for (unsigned int ii=0;ii<allData.size();ii++)
            [(NSMutableArray *)multiLoadData.multiTileData addObject:allData[ii]];
        multiLoadData.tileData = [multiLoadData.multiTileData objectAtIndex:0];

        // Hand over to another queue to do the parsing, since that can be slow
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            [self->loadInterp parseData:multiLoadData];
            
            [self performSelector:@selector(mergeLoadedTile:) onThread:self->samplingLayer.layerThread withObject:multiLoadData waitUntilDone:NO];
        });
    }

}

// Called on SamplingLayer.layerThread
- (void)mergeLoadedTile:(MaplyLoaderReturn *)loadReturn
{
    if (self.debugMode)
        NSLog(@"MaplyQuadImageLoader: Merging fetch for %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);

    if (loadReturn.error)
        NSLog(@"MaplyQuadImageLoader: Error in parsing tile data:\n%@",[loadReturn.error localizedDescription]);
    
    QuadTreeNew::Node ident(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    auto it = tiles.find(ident);
    // Failed, so clean up the objects that may have been created
    if (it == tiles.end() || it->second->getState() != TileAsset::Loaded || loadReturn.error)
    {
        if (loadReturn.compObjs) {
            [viewC removeObjects:loadReturn.compObjs mode:MaplyThreadCurrent];
        }
        if (self.debugMode)
            NSLog(@"MaplyQuadImageLoader: Failed to load tile before it was erased %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);
        return;
    }
    auto tile = it->second;
    
    // We know the tile succeeded and is something we're looking for
    // Now put its data in place

    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;
    
    ChangeSet changes;
    
    WhirlyKitLoadedTile *loadTile = loadReturn.image;
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
    
    tile->setupContents(loadedTile,tex,loadReturn.compObjs,interactLayer,changes);

    if (self.debugMode)
        NSLog(@"Loaded %d: (%d,%d) texID = %d",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y,(int)(tex ? tex->getId() : 0));

    if (tex) {
        // See if it's useful elsewhere
        [self applyCoverTile:ident asset:tile changes:changes];
    } else {
        NSLog(@"Failed to create texture for tile %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);
        // Something failed, so just clear it back to blank
        tile->clearToBlank(interactLayer, changes, TileAsset::Waiting);
    }
    
    // Evaluate parents wrt to their children and enable/disable
    [self evalParentsLayer:interactLayer changes:changes];
    
    [layer.layerThread addChangeRequests:changes];
}

// Called on SamplingLayer.layerThread
- (void)fetchRequestFail:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame error:(NSError *)error
{
    NSLog(@"MaplyQuadImageLoader: Failed to fetch tile %d: (%d,%d) because:\n%@",tileID.level,tileID.x,tileID.y,[error localizedDescription]);
    
    
}

// Decide if this tile ought to be loaded
- (bool)shouldLoad:(QuadTreeNew::ImportantNode &)tile
{
    if (self.importanceCutoff == 0.0 || tile.importance >= self.importanceCutoff) {
        return true;
    }
    
    return false;
}

// Clear out assets for a tile, but keep the geometry
- (void)clearTileToBlank:(TileAssetRef &)tile ident:(QuadTreeNew::ImportantNode &)ident layer:(MaplyBaseInteractionLayer *)layer changes:(WhirlyKit::ChangeSet &)changes
{
    if (self.debugMode)
        NSLog(@"Clear tile to blank %d: (%d,%d) texId = %d",ident.level,ident.x,ident.y,(int)tile->getTexID());

    tile->clearToBlank(layer, changes, TileAsset::Waiting);

    // Find a new cover texture for tile
    [self findCoverTile:ident changes:changes];
    
    // If any tiles were using this as a cover, have them find a new one
    for (auto posTile : tiles) {
        if (posTile.second->isUsingTexFromNode(ident)) {
            posTile.second->clearToBlank(layer, changes, posTile.second->getState());
            [self findCoverTile:posTile.first changes:changes];
        }
    }
}

// MARK: Quad Build Delegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)inBuilder layer:(WhirlyKitQuadDisplayLayerNew * __nonnull)inLayer
{
    builder = inBuilder;
    layer = inLayer;
}

- (QuadTreeNew::NodeSet)quadBuilder:(WhirlyKitQuadTileBuilder * _Nonnull)builder loadTiles:(const QuadTreeNew::ImportantNodeSet &)loadTiles unloadTilesToCheck:(const QuadTreeNew::NodeSet &)unloadTiles
{
    QuadTreeNew::NodeSet toKeep;
    
    // A list of all the tiles that we're going to load or are loading
    QuadTreeNew::NodeSet allLoads;
    for (auto node : loadTiles)
        allLoads.insert(node);
    for (auto node : tiles)
        if (node.second->getState() == TileAsset::Loading)
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
                if (it != tiles.end() && it->second->getState() == TileAsset::Loaded) {
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
        // If this tile (to be unloaded) isn't full loaded, then we don't care about it
        if (it->second->getState() != TileAsset::Loaded)
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

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder update:(const WhirlyKit::TileBuilderDelegateInfo &)updates changes:(WhirlyKit::ChangeSet &)changes
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
                [self fetchThisTile:tile ident:ident];
            }
        } else {
            // We don't consider it worth loading now so drop it if we were
            switch (tile->getState())
            {
                case TileAsset::Waiting:
                    // this is fine
                    break;
                case TileAsset::Loaded:
                    if (self.debugMode)
                        NSLog(@"Tile switched from Loaded to Wait %d: (%d,%d) importance = %f",ident.level,ident.x,ident.y,ident.importance);
                    [self clearTileToBlank:tile ident:ident layer:interactLayer changes:changes];
                    break;
                case TileAsset::Loading:
                    if (self.debugMode)
                        NSLog(@"Canceled fetch for tile %d: (%d,%d)",ident.level,ident.x,ident.y);
                    tile->cancelFetch(tileFetcher);
                    break;
            }
        }
    }
 
    for (auto tile: updates.enableTiles)
        [self enableTile:tile->ident layer:interactLayer changes:changes];
    
    for (auto tile: updates.disableTiles)
        [self disableTile:tile->ident layer:interactLayer changes:changes];

    if (params.singleLevel) {
        [self updateCovers:interactLayer changes:changes];
    } else {
        [self evalParentsLayer:interactLayer changes:changes];
    }
    
    if (self.debugMode)
        NSLog(@"quadBuilder:updates:changes: changeRequests: %d",(int)changes.size());
}

- (void)quadBuilderPreSceneFlush:(WhirlyKitQuadTileBuilder *)builder
{
}

// Look for covers that have disappeared and have to be replaced
- (void)updateCovers:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    for (auto it : tiles) {
        auto ident = it.first;
        auto tile = it.second;
        
        if (tile->isUsingCoverTex()) {
            auto coverNode = tile->getCoverTexNode();
            auto coverIt = tiles.find(coverNode);
            if (coverIt == tiles.end() || coverIt->second->isUsingCoverTex()) {
                tile->clearToBlank(interactLayer, changes, tile->getState());
                [self findCoverTile:ident changes:changes];
            }
        }
    }
}

// Evaluate parents wrt to their children and enable/disable
- (void)evalParentsLayer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    // Mark everything with children loading or not
    for (auto it : tiles) {
        it.second->setChildrenLoading(false);
    }
    for (auto it = tiles.rbegin(); it != tiles.rend(); it++) {
        auto ident = it->first;
        auto tile = it->second;
        if ((tile->getState() == TileAsset::Loading)
            && ident.level > minLevel) {
            // Mark the parent as having loading children
            QuadTreeNew::Node parentIdent(ident.x/2,ident.y/2,ident.level-1);
            auto parentIt = tiles.find(parentIdent);
            if (parentIt != tiles.end())
                parentIt->second->setChildrenLoading(true);
        }
    }
    
    // If there are children loading and this is off, turn it back on
    for (auto it : tiles) {
        auto ident = it.first;
        auto tile = it.second;
        if (tile->getState() == TileAsset::Loaded) {
            if (tile->areChildrenLoading() || tile->getShouldEnable()) {
                tile->enableTile(interactLayer, changes);
            } else {
                tile->disableTile(interactLayer, changes);
            }
        }
    }
}

// Look for a texture we've already loaded that could be used for this new tile
- (void)findCoverTile:(const QuadTreeNew::Node &)tileIdent changes:(ChangeSet &)changes
{
    TileAssetRef bestTile;
    QuadTreeNew::Node bestIdent;
    auto it = tiles.find(tileIdent);
    if (it == tiles.end())
        return;
    auto tile = it->second;
    LoadedTileNewRef loadedTile = [builder getLoadedTile:tileIdent];
    if (!loadedTile)
        return;

    // Note: Pick the cover tile and then assign it rather than reassigning each time
    for (auto it : tiles) {
        auto coverIdent = it.first;
        auto coverAsset = it.second;
        if (coverAsset->getState() == TileAsset::Loaded && coverAsset->isOurTexture() && tile->isBetterCoverTile(coverIdent,tileIdent)) {
            if (!bestTile || coverIdent.level > bestIdent.level) {
                bestIdent = coverIdent;
                bestTile = coverAsset;
            }
        }
    }
    
    if (bestTile) {
        if (self.debugMode)
            NSLog(@"Found old cover tile %d : (%d,%d) to tile %d : (%d,%d)",bestIdent.level,bestIdent.x,bestIdent.y,tileIdent.level,tileIdent.x,tileIdent.y);
        tile->applyCoverTile(bestIdent,bestTile,loadedTile,self.flipY,changes);
    }
}

// Apply the texture for a tile we just loaded
- (void)applyCoverTile:(const QuadTreeNew::Node &)coverIdent asset:(TileAssetRef)coverAsset changes:(ChangeSet &)changes
{
    TileAssetRef bestTile;
    
    // Now see if there are any other places to use this texture
    for (auto it : tiles) {
        if (it.second->getState() != TileAsset::Loaded) {
            auto tileIdent = it.first;
            auto tile = it.second;
            if (tile->isBetterCoverTile(coverIdent,tileIdent)) {
                LoadedTileNewRef loadedTile = [builder getLoadedTile:tileIdent];
                if (loadedTile) {
                    if (self.debugMode)
                        NSLog(@"Applying new cover tile %d : (%d,%d) to tile %d : (%d,%d)",coverIdent.level,coverIdent.x,coverIdent.y,tileIdent.level,tileIdent.x,tileIdent.y);
                    tile->applyCoverTile(coverIdent,coverAsset,loadedTile,self.flipY,changes);
                }
            }
        }
    }
}

- (void)shutdown
{
    [viewC releaseSamplingLayer:samplingLayer forUser:self];
}

@end
