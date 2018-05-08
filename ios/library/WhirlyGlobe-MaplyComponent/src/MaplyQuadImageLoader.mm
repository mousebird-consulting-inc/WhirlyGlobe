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

namespace WhirlyKit
{

// Keep track of what we've already loaded
class TileAsset
{
public:
    TileAsset() : texID(EmptyIdentity), drawPriority(0), compObjs(nil), shouldEnable(false), enable(false), loadingHandle(nil), state(ToLoad) { }
    
    // Clean out assets
    void clear(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        if (texID != EmptyIdentity)
            changes.push_back(new RemTextureReq(texID));
        [interactLayer removeObjects:compObjs changes:changes];
        texID = EmptyIdentity;
        compObjs = nil;
    }
    
    typedef enum {ToLoad,Loading,Loaded} State;
    State state;
    // Set when we decide children are loading
    bool childrenLoading;
    
    // Tile ID of the texture we're applying to this tile.
    // Might be a lower resolution tile.
    WhirlyKit::QuadTreeNew::Node texNode;

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
    
    // Handle returned by the loader while it's loading (probably a task)
    id loadingHandle;
};

typedef std::shared_ptr<TileAsset> TileAssetRef;
typedef std::map<QuadTreeNew::Node,TileAssetRef> TileAssetMap;
}

using namespace WhirlyKit;

@implementation MaplyQuadImageLoaderReturn

- (id)init
{
    self = [super init];
    _frame = -1;
    
    return self;
}

@end

@implementation MaplyQuadImageLoader
{
    NSObject<MaplyTileSource> *tileSource;
    WhirlyKitQuadTileBuilder * __weak builder;
    WhirlyKitQuadDisplayLayerNew * __weak layer;
    int minLevel,maxLevel;

    // Tiles in various states of loading or loaded
    TileAssetMap tiles;
    // Tiles in the process of loading
    TileAssetMap currentlyLoading;
    // Tiles to load next
    TileAssetMap toLoad;
    
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
}

- (instancetype)initWithTileSource:(NSObject<MaplyTileSource> *)inTileSource viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)inViewC
{
    tileSource = inTileSource;
    viewC = inViewC;
    if (![tileSource respondsToSelector:@selector(startFetchLayer:tile:frame:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement startFetchLayer:tile:frame:");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(cancelTile:frame:tileData:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement cancelTile:frame:tileData:");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(clear:tileData:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement clear:tileData:");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(validTile:bbox:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement validTile:bbox:");
        return nil;
    }
    
    _numSimultaneousFetches = 16;
    _flipY = true;
    _debugMode = true;
    minLevel = tileSource.minZoom;
    maxLevel = tileSource.maxZoom;

    self = [super init];
    return self;
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
    // It's already here, which is weird.  Let's get rid of it.
    if (it != tiles.end()) {
        it->second->clear(interactLayer, changes);
        if (it->second->state == TileAsset::Loading)
            currentlyLoading.erase(it->first);
        if (it->second->state == TileAsset::ToLoad)
            toLoad.erase(it->first);
        tiles.erase(it);
    }
}

- (TileAssetRef)addNewTile:(QuadTreeNew::Node)ident layer:(MaplyBaseInteractionLayer *)interactLayer changes:(ChangeSet &)changes
{
    // Set up a new tile
    auto newTile = TileAssetRef(new TileAsset());
    newTile->state = TileAsset::ToLoad;
    newTile->shouldEnable = false;
    newTile->enable = false;
    tiles[ident] = newTile;
    toLoad[ident] = newTile;
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
    [interactLayer disableObjects:tile->compObjs changes:changes];
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

    for (auto tile: loadTiles) {
        // If it's already there, clear it out
        [self removeTile:tile->ident layer:interactLayer changes:changes];
        
        // Create the new tile and put in the toLoad queue
        auto newTile = [self addNewTile:tile->ident layer:interactLayer changes:changes];
    }
    
    [self updateLoading];
}

// Called on the layer thread
- (void)updateLoading
{
    // Ask for a few more to load
    while (currentlyLoading.size() < _numSimultaneousFetches) {
        auto nextLoad = toLoad.begin();
        if (nextLoad == toLoad.end())
            break;
        
        auto tile = nextLoad->second;
        auto ident = nextLoad->first;
        currentlyLoading[ident] = tile;
        
        // Ask the source to load the tile
        MaplyTileID tileID;
        tileID.level = ident.level;
        tileID.x = ident.x;
        tileID.y = ident.y;
        if (_debugMode)
            NSLog(@"Started loading %d: (%d,%d)",tileID.level,tileID.x,tileID.y);

        tile->state = TileAsset::Loading;
        toLoad.erase(nextLoad);
        
        [tileSource startFetchLayer:self tile:tileID frame:-1];
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
        if ((tile->state == TileAsset::Loading || tile->state == TileAsset::ToLoad)
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
        if (tile->state == TileAsset::Loading) {
            if (_debugMode)
                NSLog(@"Cancelled loading %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
            id tileData = tile->loadingHandle;
            
            [tileSource cancelTile:tileID frame:-1 tileData:tileData];
        }
        
        // Clear out any associated data and remove it from our list
        [self removeTile:inTile->ident layer:interactLayer changes:changes];
    }
    
    [self updateLoading];
    
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

// Called from anywhere
- (bool)loadedReturn:(MaplyQuadImageLoaderReturn *)loadReturn
{
    // Note: Check the data coming in and return false if it's bad

    [self performSelector:@selector(loadedReturnRun:) onThread:layer.layerThread withObject:loadReturn waitUntilDone:NO];
    
    return true;
}

// Called on the layer thread
- (bool)loadedReturnRun:(MaplyQuadImageLoaderReturn *)loadReturn
{
    auto control = viewC.getRenderControl;
    if (!control)
        return false;
    auto interactLayer = control->interactLayer;

    ChangeSet changes;
    
    QuadTreeNew::Node ident(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    auto it = tiles.find(ident);
    // Never seen it
    if (it == tiles.end()) {
//        NSLog(@"LoadedReturnRun: Got a tile we aren't actually loading %d: (%d,%d)",ident.level,ident.x,ident.y);
        return false;
    }
    auto tile = it->second;
    tile->state = TileAsset::Loaded;

    if (_debugMode)
        NSLog(@"Loaded %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);

    // Shouldn't be in toLoad, but doesn't hurt
    toLoad.erase(ident);
    currentlyLoading.erase(ident);
    
    // Create the image and tie it to the drawables
    MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:loadReturn.image];
    // Note: Deal with border pixels
    int borderPixel = 0;
    bool success = false;
    WhirlyKitLoadedTile *loadTile = [tileData wkTile:borderPixel convertToRaw:true];
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
                    tile->drawPriority = loadedTile->drawPriority;
                    changes.push_back(new AddTextureReq(tex));
                    
                    // Assign it to the various drawables
                    for (auto drawID : loadedTile->drawIDs) {
                        changes.push_back(new DrawTexChangeRequest(drawID,0,tile->texID));
                        changes.push_back(new DrawPriorityChangeRequest(drawID,tile->drawPriority));
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

    [self updateLoading];

    return true;
}

- (void)registerTile:(MaplyTileID)tileID frame:(int)frame data:(id __nullable)tileData
{
    if ([NSThread currentThread] != layer.layerThread)
        return;
    
    QuadTreeNew::Node node(tileID.level,tileID.x,tileID.y);
    auto it = currentlyLoading.find(node);
    if (it != currentlyLoading.end())
        it->second->loadingHandle = tileData;
}

// Evaluate and possibly modify the texture for a given tile based on a possible cover tile
- (void)evalCoverTile:(const QuadTreeNew::Node &)coverIdent coverAsset:(TileAssetRef)coverAsset tileIdent:(const QuadTreeNew::Node &)tileIdent changes:(ChangeSet &)changes
{
    int relLevel = tileIdent.level - coverIdent.level;
    int relX = tileIdent.x - coverIdent.x * (1<<relLevel), relY = tileIdent.y - coverIdent.y * (1<<relLevel);
    if (relX >= 0 && relY >= 0 && relX < 1<<relLevel && relY < 1<<relLevel) {
//        NSLog(@"Cover tile: %d: (%d,%d), child tile: %d: (%d,%d), rel: %d: (%d,%d)",coverIdent.level,coverIdent.x,coverIdent.y,tileIdent.level,tileIdent.x,tileIdent.y,relLevel,relX,relY);
        LoadedTileNewRef loadedTile = [builder getLoadedTile:tileIdent];
        for (auto drawID: loadedTile->drawIDs)
        {
            if (_flipY)
                relY = (1<<relLevel)-relY-1;
            changes.push_back(new DrawTexChangeRequest(drawID,0,coverAsset->texID,relLevel,relX,relY));
            changes.push_back(new DrawPriorityChangeRequest(drawID,coverAsset->drawPriority));
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
    for (auto it : toLoad) {
        auto tileIdent = it.first;
        if ( coverIdent.level < tileIdent.level) {
            [self evalCoverTile:coverIdent coverAsset:coverAsset tileIdent:tileIdent changes:changes];
        }
    }
}

@end
