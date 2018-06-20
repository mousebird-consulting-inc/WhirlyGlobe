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

namespace WhirlyKit
{

// Keep track of what we've already loaded
class TileAsset
{
public:
    TileAsset() : texID(EmptyIdentity), drawPriority(0), compObjs(nil), shouldEnable(false), enable(false), fetchHandle(nil), state(Loading) { }
    
    // Clean out assets
    void clear(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        if (texID != EmptyIdentity)
            changes.push_back(new RemTextureReq(texID));
        for (auto drawID : instanceDrawIDs)
            changes.push_back(new RemDrawableReq(drawID));
        if (compObjs)
            [interactLayer removeObjects:compObjs changes:changes];
        texID = EmptyIdentity;
        compObjs = nil;
    }
    
    typedef enum {Loading,Loaded} State;
    State state;
    // Set when we decide children are loading
    bool childrenLoading;
    
    // Tile ID of the texture we're applying to this tile.
    // Might be a lower resolution tile.
    WhirlyKit::QuadTreeNew::Node texNode;
    
    // IDs of the instances we created to shadow the geometry
    std::vector<SimpleIdentity> instanceDrawIDs;

    // The texture ID owned by this node.  Delete it when we're done.
    SimpleIdentity texID;
    
    // Draw Priority assigned to this tile by default
    int drawPriority;
    
    // Set if the Tile Builder thinks this should be enabled
    bool shouldEnable;
    
    // Mirror of enable from the Tile Builder
    bool enable;
    
    // Component objects owned by the tile
    NSArray *compObjs;
    
    // Handle returned by the fetcher while it's fetching
    id fetchHandle;
};

typedef std::shared_ptr<TileAsset> TileAssetRef;
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

@implementation MaplyQuadImageLoader
{
    MaplySamplingParams *params;
    MaplyRemoteTileInfo *tileInfo;
    WhirlyKitQuadTileBuilder * __weak builder;
    WhirlyKitQuadDisplayLayerNew * __weak layer;
    int minLevel,maxLevel;
    
    MaplyTileFetcher * __weak tileFetcher;
    NSObject<MaplyLoaderInterpreter> *loadInterp;
    
    // Tiles in various states of loading or loaded
    TileAssetMap tiles;
    
    MaplyBaseViewController * __weak viewC;
    MaplyQuadSamplingLayer *samplingLayer;
}

- (nullable instancetype)initWithParams:(MaplySamplingParams *)inParams tileInfo:(MaplyRemoteTileInfo *__nonnull)inTileInfo viewC:(MaplyBaseViewController * __nonnull)inViewC
{
    params = inParams;
    tileInfo = inTileInfo;
    viewC = inViewC;

    _baseDrawPriority = kMaplyImageLayerDrawPriorityDefault;
    _drawPriorityPerLevel = 100;

//    if (![tileSource respondsToSelector:@selector(startFetchLayer:tile:frame:)]) {
//        NSLog(@"MaplyQuadImageLoader requires tile source implement startFetchLayer:tile:frame:");
//        return nil;
//    }
//    if (![tileSource respondsToSelector:@selector(cancelTile:frame:tileData:)]) {
//        NSLog(@"MaplyQuadImageLoader requires tile source implement cancelTile:frame:tileData:");
//        return nil;
//    }
//    if (![tileSource respondsToSelector:@selector(clear:tileData:)]) {
//        NSLog(@"MaplyQuadImageLoader requires tile source implement clear:tileData:");
//        return nil;
//    }
//    if (![tileSource respondsToSelector:@selector(validTile:bbox:)]) {
//        NSLog(@"MaplyQuadImageLoader requires tile source implement validTile:bbox:");
//        return nil;
//    }

    self = [super init];

    _flipY = true;
    _debugMode = false;
    minLevel = tileInfo.minZoom;
    maxLevel = tileInfo.maxZoom;

    // Start things out after a delay
    // This lets the caller mess with settings
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!self->tileFetcher) {
            self->tileFetcher = [self->viewC getSharedTileFetcher];
        }
        if (!self->loadInterp) {
            self->loadInterp = [[MaplyImageLoaderInterpreter alloc] init];
        }
        
        self->samplingLayer = [self->viewC findSamplingLayer:inParams forUser:self];
    });

    return self;
}

- (void)setTileFetcher:(MaplyTileFetcher * __nonnull)inTileFetcher
{
    if (tileFetcher) {
        NSLog(@"Caller tried to set tile fetcher after startup in MaplyQuadImageLoader.  Ignoring.");
        return;
    }
    
    tileFetcher = inTileFetcher;
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

- (void)removeTile:(QuadTreeNew::Node)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    auto it = tiles.find(ident);
    // If it's here, let's get rid of it.
    if (it != tiles.end()) {
        it->second->clear(interactLayer, changes);
        if (it->second->state == TileAsset::Loading) {
            if (_debugMode)
                NSLog(@"Cancelled loading %d: (%d,%d)",ident.level,ident.x,ident.y);

            [tileFetcher cancelTileFetch:it->second->fetchHandle];
        }
        tiles.erase(it);
    }
}

- (TileAssetRef)addNewTile:(QuadTreeNew::ImportantNode)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    // Put together a request for the fetcher
    MaplyTileFetchRequest *request = [[MaplyTileFetchRequest alloc] init];
    MaplyTileID tileID;  tileID.level = ident.level;  tileID.x = ident.x;  tileID.y = ident.y;
    request.tileInfo = tileInfo;
    request.tileID = tileID;
    request.frame = -1;
    request.importance = ident.importance;
    
    request.success = ^(MaplyTileFetchRequest *request, NSData *data) {
        [self performSelector:@selector(fetchRequestSuccess:) onThread:self->samplingLayer.layerThread withObject:@[request,data] waitUntilDone:NO];
    };
    request.failure = ^(MaplyTileFetchRequest *request, NSError *error) {
        [self performSelector:@selector(fetchRequestFail:) onThread:self->samplingLayer.layerThread withObject:@[request,error] waitUntilDone:NO];
    };

    // Set up a new tile
    auto newTile = TileAssetRef(new TileAsset());
    newTile->state = TileAsset::Loading;
    newTile->shouldEnable = false;
    newTile->enable = false;
    newTile->drawPriority = _baseDrawPriority + _drawPriorityPerLevel * ident.level;
    tiles[ident] = newTile;
    
    auto loadedTile = [builder getLoadedTile:ident];
    
    // Make the instance drawables we'll use to mirror the geometry
    if (loadedTile) {
        // Assign it to the various drawables
        for (auto di : loadedTile->drawInfo) {
            int newDrawPriority = newTile->drawPriority;
            switch (di.kind) {
                case WhirlyKit::LoadedTileNew::DrawableGeom:
                    newDrawPriority = newTile->drawPriority;
                    break;
                case WhirlyKit::LoadedTileNew::DrawableSkirt:
                    newDrawPriority = 11;
                    break;
                case WhirlyKit::LoadedTileNew::DrawablePole:
                    newDrawPriority = newTile->drawPriority;
                    break;
            }
            // Make a drawable instance to shadow the geometry
            auto drawInst = new BasicDrawableInstance("MaplyQuadImageLoader", di.drawID, BasicDrawableInstance::ReuseStyle);
            drawInst->setTexId(0, 0);
            drawInst->setDrawPriority(newDrawPriority);
            drawInst->setEnable(false);
            changes.push_back(new AddDrawableReq(drawInst));
            newTile->instanceDrawIDs.push_back(drawInst->getId());
        }
    }

    newTile->fetchHandle = [tileFetcher startTileFetch:request];
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
    tile->shouldEnable = true;
    tile->enable = true;
    for (auto drawID : tile->instanceDrawIDs)
        changes.push_back(new OnOffChangeRequest(drawID,true));
    [interactLayer enableObjects:tile->compObjs changes:changes];
}

- (void)disableTile:(QuadTreeNew::Node)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    auto it = tiles.find(ident);
    // Don't know about this one.  Punt
    if (it == tiles.end())
        return;

    auto tile = it->second;
    tile->shouldEnable = false;
    tile->enable = false;
    for (auto drawID : tile->instanceDrawIDs)
        changes.push_back(new OnOffChangeRequest(drawID,false));
    [interactLayer disableObjects:tile->compObjs changes:changes];
}

// Called on SamplingLayer.layerThread
- (void)fetchRequestSuccess:(NSArray *)args
{
    if (args.count != 2)
        return;
    
    MaplyTileFetchRequest *request = [args objectAtIndex:0];
    NSData *data = [args objectAtIndex:1];
    
    QuadTreeNew::Node ident(request.tileID.x,request.tileID.y,request.tileID.level);
    auto it = tiles.find(ident);
    if (it == tiles.end()) {
        // Didn't want it, so drop it on the floor
        return;
    }
    auto tile = it->second;

    // Ask the interpreter to parse it, but on its own damn queue
    MaplyLoaderReturn *loadData = [[MaplyLoaderReturn alloc] init];
    loadData.tileID = request.tileID;
    loadData.frame = request.frame;
    loadData.tileData = data;
    if (tile->state == TileAsset::Loading) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                        ^{
                            [self->loadInterp parseData:loadData];
                            if (loadData.error) {
                                NSLog(@"MaplyQuadImageLoader: Error in parsing tile data:\n%@",[loadData.error localizedDescription]);
                                // Note: Set the tile state to failed or something?
                            } else {
                                [self performSelector:@selector(mergeLoadedTile:) onThread:self->samplingLayer.layerThread withObject:loadData waitUntilDone:NO];
                            }
                        });
    } else {
        // That's weird.  Punt for now
        NSLog(@"MaplyQuadImageLoader loaded a tile that was already loaded.");
    }
}

// Called on SamplingLayer.layerThread
- (void)mergeLoadedTile:(MaplyLoaderReturn *)loadReturn
{
    QuadTreeNew::Node ident(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    auto it = tiles.find(ident);
    // Failed, so clean up the objects that may have been created
    if (it == tiles.end() || it->second->state != TileAsset::Loading)
    {
        if (loadReturn.compObjs) {
            [viewC removeObjects:loadReturn.compObjs mode:MaplyThreadCurrent];
        }
        return;
    }
    auto tile = it->second;
    
    tile->state = TileAsset::Loaded;
    
    // We know the tile succeeded and is something we're looking for
    // Now put its data in place

    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;
    
    ChangeSet changes;
    if (_debugMode)
        NSLog(@"Loaded %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);
    
    WhirlyKitLoadedTile *loadTile = loadReturn.image;
    // Note: Get this from somewhere
    int borderPixel = 0;
    bool success = false;
    if ([loadTile.images count] > 0) {
        WhirlyKitLoadedImage *loadedImage = [loadTile.images objectAtIndex:0];
        if ([loadedImage isKindOfClass:[WhirlyKitLoadedImage class]]) {
            LoadedTileNewRef loadedTile = [builder getLoadedTile:ident];
            if (loadedTile) {
                // Build the image
                Texture *tex = [loadedImage buildTexture:borderPixel destWidth:loadedImage.width destHeight:loadedImage.height];
                if (tex) {
                    success = true;
                    // Create the texture in the renderer
                    tile->texID = tex->getId();
                    changes.push_back(new AddTextureReq(tex));
                    
                    // Assign it to the various drawables
                    for (auto instID : tile->instanceDrawIDs) {
                        changes.push_back(new DrawTexChangeRequest(instID,0,tile->texID));
                        
                        if (loadedTile->enabled)
                            changes.push_back(new OnOffChangeRequest(instID,true));
                    }
                }
                tile->shouldEnable = loadedTile->enabled;
            }
        }
    }
    
    tile->compObjs = loadReturn.compObjs;
    
    if (success) {
        // The asset (texture) is matched directly to the node
        tile->texNode = ident;
        if (tile->shouldEnable) {
            tile->enable = true;
            [interactLayer enableObjects:loadReturn.compObjs changes:changes];
        }
        
        // See if it's useful elsewhere
        [self applyCoverTile:ident asset:tile changes:changes];
    } else {
        [self removeTile:ident layer:interactLayer changes:changes];
    }
    
    // Evaluate parents wrt to their children and enable/disable
    [self evalParentsLayer:interactLayer changes:changes];
    
    [layer.layerThread addChangeRequests:changes];
}

// Called on SamplingLayer.layerThread
- (void)fetchRequestFail:(NSArray *)args
{
    if (args.count != 2)
        return;
    
//    MaplyTileFetchRequest *request = [args objectAtIndex:0];
    NSError *error = [args objectAtIndex:1];
    
    NSLog(@"MaplyQuadImageLoader: Failed to fetch tile because:\n%@",[error localizedDescription]);
}

// MARK: Quad Build Delegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)inBuilder layer:(WhirlyKitQuadDisplayLayerNew * __nonnull)inLayer
{
    builder = inBuilder;
    layer = inLayer;
}

// Called on the layer thread
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder loadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)loadTiles changes:(ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;

    for (auto it = loadTiles.rbegin(); it != loadTiles.rend(); ++it) {
        auto tile = *it;
        // If it's already there, clear it out
        [self removeTile:tile->ident layer:interactLayer changes:changes];
        
        // Create the new tile and put in the toLoad queue
        auto newTile = [self addNewTile:tile->ident layer:interactLayer changes:changes];
    }
}

// Evaluate parents wrt to their children and enable/disable
- (void)evalParentsLayer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    // Mark everything with children loading or not
    for (auto it : tiles) {
        it.second->childrenLoading = false;
    }
    for (auto it = tiles.rbegin(); it != tiles.rend(); it++) {
        auto ident = it->first;
        auto tile = it->second;
        if ((tile->state == TileAsset::Loading)
            && ident.level > minLevel) {
            // Mark the parent as having loading children
            QuadTreeNew::Node parentIdent(ident.x/2,ident.y/2,ident.level-1);
            auto parentIt = tiles.find(parentIdent);
            if (parentIt != tiles.end())
                parentIt->second->childrenLoading = true;
        }
    }
    
    // If there are children loading and this is off, turn it back on
    for (auto it : tiles) {
        auto ident = it.first;
        auto tile = it.second;
        if (tile->state == TileAsset::Loaded) {
            if (tile->childrenLoading || tile->shouldEnable) {
                if (!tile->enable) {
                    tile->enable = true;
                    [interactLayer enableObjects:tile->compObjs changes:changes];
                }
            } else {
                if (tile->enable) {
                    tile->enable = false;
                    [interactLayer disableObjects:tile->compObjs changes:changes];
                }
            }
        }
    }
}

// Called on the layer thread
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder unLoadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)inTiles changes:(ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;

    for (auto inTile: inTiles) {
        MaplyTileID tileID;
        tileID.level = inTile->ident.level;
        tileID.x = inTile->ident.x;
        tileID.y = inTile->ident.y;

        auto it = tiles.find(inTile->ident);
        // Don't know about this one.  Punt
        if (it == tiles.end())
            continue;
        auto tile = it->second;

        // Clear out any associated data and remove it from our list
        [self removeTile:inTile->ident layer:interactLayer changes:changes];
    }
    
    // Evaluate parents wrt to their children
    [self evalParentsLayer:interactLayer changes:changes];
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder enableTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)inTiles changes:(WhirlyKit::ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;
    
    for (auto tile: inTiles)
        [self enableTile:tile->ident layer:interactLayer changes:changes];
    
    // Evaluate parents wrt to their children and enable/disable
    [self evalParentsLayer:interactLayer changes:changes];
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder disableTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)inTiles changes:(WhirlyKit::ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;

    for (auto tile: inTiles)
        [self disableTile:tile->ident layer:interactLayer changes:changes];
    
    // Evaluate parents wrt to their children and enable/disable
    [self evalParentsLayer:interactLayer changes:changes];
}

// Evaluate and possibly modify the texture for a given tile based on a possible cover tile
- (void)evalCoverTile:(const QuadTreeNew::Node &)coverIdent coverAsset:(TileAssetRef)coverAsset tileIdent:(const QuadTreeNew::Node &)tileIdent changes:(ChangeSet &)changes
{
    int relLevel = tileIdent.level - coverIdent.level;
    int relX = tileIdent.x - coverIdent.x * (1<<relLevel), relY = tileIdent.y - coverIdent.y * (1<<relLevel);
    if (relX >= 0 && relY >= 0 && relX < 1<<relLevel && relY < 1<<relLevel) {
//        NSLog(@"Cover tile: %d: (%d,%d), child tile: %d: (%d,%d), rel: %d: (%d,%d)",coverIdent.level,coverIdent.x,coverIdent.y,tileIdent.level,tileIdent.x,tileIdent.y,relLevel,relX,relY);
        LoadedTileNewRef loadedTile = [builder getLoadedTile:tileIdent];
        auto it = tiles.find(tileIdent);
        TileAssetRef thisTile;
        if (it != tiles.end())
            thisTile = it->second;
        if (loadedTile && thisTile) {
            thisTile->texNode = coverIdent;
            for (int which = 0; which < loadedTile->drawInfo.size(); which++)
            {
                auto di = loadedTile->drawInfo[which];
                int drawInstID = thisTile->instanceDrawIDs[which];
                int newDrawPriority = coverAsset->drawPriority;
                switch (di.kind) {
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
                if (_flipY)
                    relY = (1<<relLevel)-relY-1;
                changes.push_back(new DrawTexChangeRequest(drawInstID,0,coverAsset->texID,relLevel,relX,relY));
                changes.push_back(new DrawPriorityChangeRequest(drawInstID,newDrawPriority));
                if (loadedTile->enabled)
                    changes.push_back(new OnOffChangeRequest(drawInstID,true));
            }
        }
    }
}

// Look for a texture we've already loaded that could be used for this new tile
- (void)findCoverTile:(const QuadTreeNew::Node &)tileIdent changes:(ChangeSet &)changes
{
    for (auto it : tiles) {
        auto coverIdent = it.first;
        auto coverAsset = it.second;
        if (coverAsset->state == TileAsset::Loaded && coverIdent.level < tileIdent.level)
            [self evalCoverTile:coverIdent coverAsset:coverAsset tileIdent:tileIdent changes:changes];
    }
}

// Apply the texture for a tile we just loaded
- (void)applyCoverTile:(const QuadTreeNew::Node &)coverIdent asset:(TileAssetRef)coverAsset changes:(ChangeSet &)changes
{
    // Now see if there are any other places to use this texture
    for (auto it : tiles) {
        if (it.second->state == TileAsset::Loading) {
            auto tileIdent = it.first;
            if ( coverIdent.level < tileIdent.level) {
                [self evalCoverTile:coverIdent coverAsset:coverAsset tileIdent:tileIdent changes:changes];
            }
        }
    }
}

- (void)shutdown
{
    [viewC releaseSamplingLayer:samplingLayer forUser:self];
}

@end
