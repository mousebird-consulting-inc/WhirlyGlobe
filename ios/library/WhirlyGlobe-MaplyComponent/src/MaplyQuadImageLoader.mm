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
#import "MaplyScreenLabel.h"

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
    TileAsset() : drawPriority(0), compObjs(nil), ovlCompObjs(nil), enable(false), state(Waiting)
    { }

    // Tile is doing what?
    typedef enum {Waiting,Loading,Loaded} State;
    State getState() { return state; }

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
        for (auto texID: texIDs)
            changes.push_back(new RemTextureReq(texID));
        if (compObjs)
            [interactLayer removeObjects:compObjs changes:changes];
        if (ovlCompObjs)
            [interactLayer removeObjects:ovlCompObjs changes:changes];
        texIDs.clear();
        compObjs = nil;
        ovlCompObjs = nil;
    }
    
    // Return the texture ID
    const std::vector<SimpleIdentity> &getTexIDs() { return texIDs; }
    
    int getDrawPriority() { return drawPriority; }
    
    bool getEnable() { return enable; }
    
    // Set when we decide children are loading
    bool areChildrenLoading() { return childrenLoading; }
    void setChildrenLoading(bool newVal) { childrenLoading = newVal; }
 
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
    bool dataWasLoaded(MaplyTileFetchRequest *request,NSData *loadedData) {
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
        
        // Keep track of the data in case we're loading multiple sets
        if (which >= 0) {
            if (which >= fetchedData.size())
                fetchedData.resize(which+1,nil);
            fetchedData[which] = loadedData;
        }
        
        return !anyLoading;
    }

    // Acknowledge the load
    void setLoaded() {
        state = Loaded;
    }
    
    // Return the list of data fetched and clear it locally
    std::vector<NSData *> getAndClearData() {
        std::vector<NSData *> toRet;
        for (auto data : fetchedData)
            if (data)
                toRet.push_back(data);
        fetchedData.clear();
        return toRet;
    }
    
    // After a successful load, set up the texture and any other contents
    void setupContents(LoadedTileNewRef loadedTile,std::vector<Texture *> texs,NSArray *inCompObjs,NSArray *inOvlCompObjs,
                       MaplyBaseInteractionLayer *layer,ChangeSet &changes) {
        // Sometimes we can end up loading/unloading/reloading a tile
        if ([compObjs count] > 0)
            [layer removeObjects:compObjs changes:changes];
        if ([ovlCompObjs count] > 0)
            [layer removeObjects:ovlCompObjs changes:changes];
        
        enable = loadedTile->enabled;
        compObjs = inCompObjs;
        ovlCompObjs = inOvlCompObjs;

        // Create the texture in the renderer
        for (Texture *tex : texs) {
            SimpleIdentity texID = tex->getId();
            changes.push_back(new AddTextureReq(tex));
            texIDs.push_back(texID);
        }

        // Assign it to the various drawables
        for (int which = 0; which < loadedTile->drawInfo.size(); which++) {
            int drawInstID = instanceDrawIDs[which];

            // Draw priority for this asset when it's loaded
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
            int whichTex = 0;
            for (auto texID : texIDs) {
                changes.push_back(new DrawTexChangeRequest(drawInstID,whichTex,texID));
                whichTex++;
            }
            changes.push_back(new DrawPriorityChangeRequest(drawInstID,newDrawPriority));
        }

        enable = true;
    }
    
    // Set up the instance to the base tile's geometry
    void setupGeom(MaplyQuadImageLoader *loader,LoadedTileNewRef loadedTile,int defaultDrawPriority,ChangeSet &changes) {
        enable = true;
        
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
            drawInst->setRequestZBuffer(loader.zBufferRead);
            drawInst->setWriteZBuffer(loader.zBufferWrite);
            if (loader->shaderID != EmptyIdentity)
                drawInst->setProgram(loader->shaderID);
            if (loader->renderTarget)
                drawInst->setRenderTarget(loader->renderTarget.renderTargetID);
            changes.push_back(new AddDrawableReq(drawInst));
            instanceDrawIDs.push_back(drawInst->getId());
        }
    }

    // Build the change requests to turn this one off or on
    // Separate from our enable.  Might be missing our texture
    void generateEnableChange(bool thisEnable,MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        if (thisEnable) {
            for (auto drawID : instanceDrawIDs)
                changes.push_back(new OnOffChangeRequest(drawID,true));
            [interactLayer enableObjects:compObjs changes:changes];
        } else {
            for (auto drawID : instanceDrawIDs)
                changes.push_back(new OnOffChangeRequest(drawID,false));
            [interactLayer disableObjects:compObjs changes:changes];
        }
    }
    
    // Build the change requests to enable/disable overlay data
    void generateOverlayEnableChange(bool thisEnable,MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        if ([ovlCompObjs count] == 0)
            return;
        
        if (thisEnable) {
            [interactLayer enableObjects:ovlCompObjs mode:MaplyThreadCurrent];
        } else {
            [interactLayer disableObjects:ovlCompObjs mode:MaplyThreadCurrent];
        }
    }
    
    // Assign the given texture to our geometry
    void generateTexIDChange(const std::vector<SimpleIdentity> &theseTexIDs,int thisDrawPriority,
                             const QuadTreeNew::Node &thisNode,WhirlyKit::LoadedTileNewRef thisLoadedTile,const QuadTreeNew::Node &texNode,
                             bool flipY,MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        int relLevel = thisNode.level - texNode.level;
        int relX = thisNode.x - texNode.x * (1<<relLevel);
        int tileIDY = thisNode.y;
        int texIdentY = texNode.y;
        if (flipY) {
            tileIDY = (1<<thisNode.level)-tileIDY-1;
            texIdentY = (1<<texNode.level)-texIdentY-1;
        }
        int relY = tileIDY - texIdentY * (1<<relLevel);
        for (auto drawID : instanceDrawIDs) {
            int whichTex = 0;
            for (auto thisTexID : theseTexIDs) {
                changes.push_back(new DrawTexChangeRequest(drawID,whichTex,thisTexID,0,0,relLevel,relX,relY));
                whichTex++;
            }
        }
        // Draw priority values are trickier
        int ii = 0;
        for (auto di : thisLoadedTile->drawInfo) {
            int newDrawPriority = thisDrawPriority;
            switch (di.kind) {
                case WhirlyKit::LoadedTileNew::DrawableGeom:
                    break;
                case WhirlyKit::LoadedTileNew::DrawableSkirt:
                    newDrawPriority = 11;
                    break;
                case WhirlyKit::LoadedTileNew::DrawablePole:
                    break;
            }
            SimpleIdentity drawID = instanceDrawIDs[ii];
            changes.push_back(new DrawPriorityChangeRequest(drawID,newDrawPriority));
            ii++;
        }
    }

    // Enable the instanced geometry and comp objects
    void enableTile(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        enable = true;
    }

    // Disable instanced geometry and comp objects
    void disableTile(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        enable = false;
    }
    
protected:
    State state;
    bool childrenLoading;
    
    // IDs of the instances we created to shadow the geometry
    std::vector<SimpleIdentity> instanceDrawIDs;

    // The texture ID owned by this node.  Delete it when we're done.
    std::vector<SimpleIdentity> texIDs;
    
    // Draw Priority assigned to this tile by default
    int drawPriority;
    
    // Set if the Tile Builder thinks this should be enabled
    bool enable;
    
    // Component objects owned by the tile
    NSArray *compObjs,*ovlCompObjs;
    
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
    if (loadTile)
        loadReturn.images = @[loadTile];
    else {
        loadReturn.error = [[NSError alloc] initWithDomain:@"MaplyQuadImageLaoder" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Tile data was not valid image"}];
    }
}

@end

@implementation MaplyDebugImageLoaderInterpreter
{
    MaplyBaseViewController * __weak viewC;
    MaplyQuadImageLoaderBase * __weak loader;
    UIFont *font;
}

- (id)initWithLoader:(MaplyQuadImageLoaderBase *)inLoader viewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    loader = inLoader;
    viewC = inViewC;
    font = [UIFont systemFontOfSize:12.0];
    
    return self;
}

- (void)parseData:(MaplyLoaderReturn * __nonnull)loadReturn
{
    [super parseData:loadReturn];
    
    MaplyBoundingBox bbox = [loader geoBoundsForTile:loadReturn.tileID];
    MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
    MaplyCoordinate center;
    center.x = (bbox.ll.x+bbox.ur.x)/2.0;  center.y = (bbox.ll.y+bbox.ur.y)/2.0;
    label.loc = center;
    label.text = [NSString stringWithFormat:@"%d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y];
    label.layoutImportance = MAXFLOAT;
    
    MaplyComponentObject *labelObj = [viewC addScreenLabels:@[label] desc:
                                     @{kMaplyFont: font,
                                       kMaplyTextColor: UIColor.blackColor,
                                       kMaplyTextOutlineColor: UIColor.whiteColor,
                                       kMaplyTextOutlineSize: @(2.0)
                                       }
                                                       mode:MaplyThreadCurrent];
    
    MaplyCoordinate coords[5];
    coords[0] = bbox.ll;  coords[1] = MaplyCoordinateMake(bbox.ur.x, bbox.ll.y);
    coords[2] = bbox.ur;  coords[3] = MaplyCoordinateMake(bbox.ll.x, bbox.ur.y);
    coords[4] = coords[0];
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:5 attributes:nil];
    [vecObj subdivideToGlobe:0.001];
    MaplyComponentObject *outlineObj = [viewC addVectors:@[vecObj] desc:nil mode:MaplyThreadCurrent];
    
    loadReturn.compObjs = @[labelObj,outlineObj];
}

@end

@implementation MaplyQuadImageLoaderBase

- (instancetype)init
{
    self = [super init];
    
    _zBufferRead = false;
    _zBufferWrite = true;
    
    return self;
}

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

- (void)setShader:(MaplyShader *)shader
{
    shaderID = [shader getShaderID];
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
    bool valid;
    MaplySamplingParams *params;
    NSArray<MaplyRemoteTileInfoNew *> *tileInfos;
    SimpleIdentity shaderID;
    
    // Current overlay level
    int curOverlayLevel;
    int targetLevel;
    bool hasOverlayObjects;
    
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
    valid = true;

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
    curOverlayLevel = -1;
    targetLevel = -1;
    hasOverlayObjects = false;
    shaderID = EmptyIdentity;

    // Start things out after a delay
    // This lets the caller mess with settings
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!self->valid)
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
    
    // Unique number for grouping these requests together
    // This way they're not processed out of order
    int thisGroup = (int)random();
    
    for (MaplyRemoteTileInfoNew *tileInfo in tileInfos) {
        if (ident.level >= tileInfo.minZoom && ident.level <= tileInfo.maxZoom) {
            // Put together a request for the fetcher
            MaplyTileFetchRequest *request = [[MaplyTileFetchRequest alloc] init];
            MaplyTileID tileID;  tileID.level = ident.level;  tileID.x = ident.x;  tileID.y = ident.y;
            id fetchInfo = [tileInfo fetchInfoForTile:tileID];
            // If there's no fetch info, then there's no data to fetch
            // Note: What happens if this is true for all data sources?
            if (fetchInfo) {
                request.fetchInfo = fetchInfo;
                request.tileSource = tileInfo;
                request.priority = 0;
                request.group = thisGroup;
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
    // Ask the interpreter to parse it
    MaplyLoaderReturn *loadData = [[MaplyLoaderReturn alloc] init];
    loadData.tileID = tileID;
    loadData.frame = frame;
    loadData.tileData = data;
    [self performSelector:@selector(mergeFetchRequest:) onThread:self->samplingLayer.layerThread withObject:@[loadData,request] waitUntilDone:NO];
}

// Deal with returned tile data (or not)
- (void)mergeFetchedData:(MaplyLoaderReturn *)loadReturn forTile:(TileAssetRef)tile tileID:(const QuadTreeNew::Node &)tileID frame:(int)frame request:(MaplyTileFetchRequest *)request
{
    // May get nil data, which means we just clean out the request
    NSData *tileData = loadReturn ? loadReturn.tileData : nil;
    
    // Loaded all the requests, so parse the data
    if (tile->dataWasLoaded(request,tileData)) {
        // Construct a LoaderReturn that contains all the data
        auto allData = tile->getAndClearData();
        if (allData.empty()) {
            tile->setLoaded();
            return;
        }
        
        MaplyLoaderReturn *multiLoadData = [[MaplyLoaderReturn alloc] init];
        MaplyTileID thisTileID;  thisTileID.level = tileID.level;  thisTileID.x = tileID.x;  thisTileID.y = tileID.y;
        multiLoadData.tileID = thisTileID;
        multiLoadData.frame = frame;
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
            [viewC removeObjects:loadReturn.ovlCompObjs mode:MaplyThreadCurrent];
        }
        return;
    }
    auto tileID = it->first;
    auto tile = it->second;

    if (self.debugMode)
        NSLog(@"MaplyQuadImageLoader: Merging fetch for tile %d: (%d,%d)",tileID.level,tileID.x,tileID.y);

    [self mergeFetchedData:loadReturn forTile:tile tileID:ident frame:loadReturn.frame request:request];
}

// Called on SamplingLayer.layerThread
- (void)mergeLoadedTile:(MaplyLoaderReturn *)loadReturn
{
    if (!valid)
        return;
    
    if (self.debugMode)
        NSLog(@"MaplyQuadImageLoader: Merging fetch for %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);

    if (loadReturn.error)
        NSLog(@"MaplyQuadImageLoader: Error in parsing tile data:\n%@",[loadReturn.error localizedDescription]);
    
    QuadTreeNew::Node ident(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    auto it = tiles.find(ident);
    // Failed, so clean up the objects that may have been created
    if (it == tiles.end() || loadReturn.error)
    {
        if (loadReturn.compObjs || loadReturn.ovlCompObjs) {
            [viewC removeObjects:loadReturn.compObjs mode:MaplyThreadCurrent];
            [viewC removeObjects:loadReturn.ovlCompObjs mode:MaplyThreadCurrent];
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

    // Might get more than one texture back
    std::vector<Texture *> texs;
    LoadedTileNewRef loadedTile = [builder getLoadedTile:ident];
    for (WhirlyKitLoadedTile *loadTile in loadReturn.images) {
        Texture *tex = NULL;
        if ([loadTile.images count] > 0) {
            WhirlyKitLoadedImage *loadedImage = [loadTile.images objectAtIndex:0];
            if ([loadedImage isKindOfClass:[WhirlyKitLoadedImage class]]) {
                if (loadedTile) {
                    // Build the image
                    tex = [loadedImage buildTexture:self.borderTexel destWidth:loadedImage.width destHeight:loadedImage.height];
                    tex->setFormat(texType);
                    texs.push_back(tex);

                    if (self.debugMode)
                        NSLog(@"Loaded %d: (%d,%d) texID = %d",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y,(int)(tex ? tex->getId() : 0));
                }
            }
        }
    }
    
    if (!texs.empty()) {
        if ([loadReturn.ovlCompObjs count] > 0)
            hasOverlayObjects = true;
        
        tile->setupContents(loadedTile,texs,loadReturn.compObjs,loadReturn.ovlCompObjs,interactLayer,changes);
    } else {
        NSLog(@"Failed to create texture for tile %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);
        // Something failed, so just clear it back to blank
        tile->clearToBlank(interactLayer, changes, TileAsset::Waiting);
    }
    
    // Tile is finally loaded after we've merged the data in
    tile->setLoaded();
    
    [layer.layerThread addChangeRequests:changes];
}

// Called on SamplingLayer.layerThread
- (void)fetchRequestFail:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame error:(NSError *)error
{
    NSLog(@"MaplyQuadImageLoader: Failed to fetch tile %d: (%d,%d) because:\n%@",tileID.level,tileID.x,tileID.y,[error localizedDescription]);
    
    QuadTreeNew::Node ident(tileID.x,tileID.y,tileID.level);
    auto it = tiles.find(ident);
    // Failed, so clean up the objects that may have been created
    if (it == tiles.end() || it->second->getState() != TileAsset::Loading) {
        return;
    }
    auto thisTileID = it->first;
    auto tile = it->second;
    
    [self mergeFetchedData:nil forTile:tile tileID:thisTileID frame:frame request:request];
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
        NSLog(@"Clear tile to blank %d: (%d,%d)",ident.level,ident.x,ident.y);

    tile->clearToBlank(layer, changes, TileAsset::Waiting);
}

// MARK: Quad Build Delegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)inBuilder layer:(WhirlyKitQuadDisplayLayerNew * __nonnull)inLayer
{
    builder = inBuilder;
    layer = inLayer;
}

- (QuadTreeNew::NodeSet)quadBuilder:(WhirlyKitQuadTileBuilder * _Nonnull)builder
                          loadTiles:(const QuadTreeNew::ImportantNodeSet &)loadTiles
                 unloadTilesToCheck:(const QuadTreeNew::NodeSet &)unloadTiles
                        targetLevel:(int)targetLevel
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

        // Lastly, hold anything that might be used for an overlay
        // Note: Only do this if we're using overlays
        if (curOverlayLevel != targetLevel) {
            if (hasOverlayObjects && node.level == curOverlayLevel) {
                if (toKeep.find(node) == toKeep.end())
                    toKeep.insert(node);
            }
        }
    }
    
    
    if (self.debugMode && !toKeep.empty()) {
        NSLog(@"Keeping: ");
        for (auto tile : toKeep) {
            NSLog(@" %d: (%d, %d)", tile.level, tile.x, tile.y);
        }
    }
    
    return toKeep;
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder
             update:(const WhirlyKit::TileBuilderDelegateInfo &)updates
            changes:(WhirlyKit::ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;
    
    targetLevel = updates.targetLevel;
    
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
    
    if (self.debugMode)
        NSLog(@"quadBuilder:updates:changes: changeRequests: %d",(int)changes.size());
}

// Turns things on/off and find cover textures
- (void)quadBuilderPreSceneFlush:(WhirlyKitQuadTileBuilder *)builder
{
    ChangeSet changes;
    auto control = viewC.getRenderControl;
    auto interactLayer = control->interactLayer;
    
    if (hasOverlayObjects) {
        if (curOverlayLevel == -1) {
            curOverlayLevel = targetLevel;
            if (self.debugMode)
                NSLog(@"Picking new overlay level %d, targetLevel = %d",curOverlayLevel,targetLevel);
        } else {
            bool allLoaded = true;
            for (auto it : tiles) {
                auto tileID = it.first;
                auto tile = it.second;
                if (tileID.level == targetLevel && tile->getState() == TileAsset::Loading) {
                    allLoaded = false;
                    break;
                }
            }
            
            if (allLoaded) {
                curOverlayLevel = targetLevel;
                if (self.debugMode)
                    NSLog(@"Picking new overlay level %d, targetLevel = %d",curOverlayLevel,targetLevel);
            }
        }
    } else {
        curOverlayLevel = targetLevel;
    }

    // Figure out the current state for each tile
    // Note: We should store the state and just send changes
    for (auto it : tiles) {
        auto tileID = it.first;
        auto tile = it.second;
        bool enable = tile->getEnable();
        
        std::vector<SimpleIdentity> texIDs = tile->getTexIDs();
        int drawPriority = tile->getDrawPriority();
        QuadTreeNew::Node texNode = it.first;
        if (enable) {
            // Borrow the texture from a parent
            while (texIDs.empty() && texNode.level > 0) {
                texNode = QuadTreeNew::Node(texNode.x/2,texNode.y/2,texNode.level-1);
                auto pit = tiles.find(texNode);
                if (pit != tiles.end()) {
                    texIDs = pit->second->getTexIDs();
                    drawPriority = pit->second->getDrawPriority();
                }
            }
            
            if (texIDs.empty()) {
                enable = false;
            }
        }

        bool ovlEnable = false;
        if (enable) {
            if (self.debugMode) {
                if (texNode != tileID)
                    NSLog(@"Applying parent %d: (%d,%d) to tile %d: (%d,%d)",texNode.level,texNode.x,texNode.y,tileID.level,tileID.x,tileID.y);
            }
            auto loadedTile = [builder getLoadedTile:it.first];
            if (loadedTile)
                tile->generateTexIDChange(texIDs,drawPriority,it.first,loadedTile,texNode,self.flipY,interactLayer,changes);
            tile->generateEnableChange(true,interactLayer,changes);

            // Turn on the overlays if we're at the highest loaded level
            if (tileID.level == curOverlayLevel)
                ovlEnable = true;
        } else {
            tile->generateEnableChange(false,interactLayer,changes);
        }
                
        // Turn any overlay data on/off
        tile->generateOverlayEnableChange(ovlEnable,interactLayer,changes);
    }
    
    [layer.layerThread addChangeRequests:changes];
}

- (void)quadBuilderShutdown:(WhirlyKitQuadTileBuilder * _Nonnull)builder
{
}

- (void)shutdown
{
    tileFetcher = nil;
    loadInterp = nil;
    [viewC releaseSamplingLayer:samplingLayer forUser:self];
}

@end
