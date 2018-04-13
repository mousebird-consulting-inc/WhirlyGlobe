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
    // Clean out assets
    void clear(MaplyBaseInteractionLayer *interactLayer,ChangeSet &changes) {
        if (texID != EmptyIdentity)
            changes.push_back(new RemTextureReq(texID));
        [interactLayer removeObjects:compObjs changes:changes];
    }
    
    // Tile ID of the texture we're applying to this tile.
    // Might be a lower resolution tile.
    WhirlyKit::QuadTreeNew::Node texNode;

    // The texture ID owned by this node.  Delete it when we're done.
    SimpleIdentity texID;
    
    // Component objects owned by the tile
    NSArray *compObjs;
};

typedef std::map<QuadTreeNew::Node,TileAsset> TileAssetMap;
typedef std::map<QuadTreeNew::Node,id> LoadingMap;
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

    // What we've been asked to load, in order
    WhirlyKit::QuadTreeNew::NodeSet toLoad;
    // What the tile source is currently working on
    LoadingMap currentlyLoading;
    // Tiles we've actually loaded and are active in memory
    WhirlyKit::TileAssetMap loaded;
    
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
//    _debugMode = true;

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

// MARK: Quad Build Delegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)inBuilder layer:(WhirlyKitQuadDisplayLayerNew * __nonnull)inLayer
{
    builder = inBuilder;
    layer = inLayer;
}

// Called on the layer thread
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder loadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(ChangeSet &)changes
{
    for (auto tile: tiles) {
        // Already got this one
        if (loaded.find(tile->ident) != loaded.end()) {
            continue;
        }
        // Already trying to load this one
        if (currentlyLoading.find(tile->ident) != currentlyLoading.end()) {
            continue;
        }
        // Add to the list of loads
        if (toLoad.find(tile->ident) == toLoad.end()) {
            [self findCoverTile:tile->ident changes:changes];
            toLoad.insert(tile->ident);
        }
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
        
        currentlyLoading[*nextLoad] = nil;
        
        // Ask the source to load the tile
        MaplyTileID tileID;
        tileID.level = nextLoad->level;
        tileID.x = nextLoad->x;
        tileID.y = nextLoad->y;
        if (_debugMode)
            NSLog(@"Started loading %d: (%d,%d)",tileID.level,tileID.x,tileID.y);

        toLoad.erase(nextLoad);
        [tileSource startFetchLayer:self tile:tileID frame:-1];
    }
}

// Called on the layer thread
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder unLoadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;

    for (auto tile: tiles) {
        MaplyTileID tileID;
        tileID.level = tile->ident.level;
        tileID.x = tile->ident.x;
        tileID.y = tile->ident.y;

        // Cancel it
        auto currentLoadingIt = currentlyLoading.find(tile->ident);
        if (currentLoadingIt != currentlyLoading.end()) {
            currentlyLoading.erase(currentLoadingIt);
            if (_debugMode)
                NSLog(@"Cancelled loading %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
            id tileData = nil;
            auto loadIt = currentlyLoading.find(tile->ident);
            if (loadIt != currentlyLoading.end())
                tileData = loadIt->second;
                
            [tileSource cancelTile:tileID frame:-1 tileData:tileData];
            
            continue;
        }
        
        // Haven't done anything with it yet
        auto toLoadIt = toLoad.find(tile->ident);
        if (toLoadIt != toLoad.end()) {
            toLoad.erase(toLoadIt);
            
            continue;
        }
        
        // It was loaded, so clean out the contents
        auto loadedIt = loaded.find(tile->ident);
        if (loadedIt != loaded.end()) {
            if (_debugMode)
                NSLog(@"Unloading %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
            loadedIt->second.clear(interactLayer,changes);

            loaded.erase(loadedIt);
        }
    }
    
    [self updateLoading];
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder enableTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;
    
    for (auto tile: tiles) {
        auto it = loaded.find(tile->ident);
        if (it != loaded.end()) {
            [interactLayer enableObjects:it->second.compObjs changes:changes];
        }
    }
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder disableTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes
{
    auto control = viewC.getRenderControl;
    if (!control)
        return;
    auto interactLayer = control->interactLayer;
    
    for (auto tile: tiles) {
        auto it = loaded.find(tile->ident);
        if (it != loaded.end()) {
            [interactLayer disableObjects:it->second.compObjs changes:changes];
        }
    }
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
    if (_debugMode)
        NSLog(@"Loaded %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);

    auto control = viewC.getRenderControl;
    if (!control)
        return false;
    auto interactLayer = control->interactLayer;

    ChangeSet changes;
    
    // No longer loading
    QuadTreeNew::Node node(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    auto currentLoadingIt = currentlyLoading.find(node);
    if (currentLoadingIt != currentlyLoading.end()) {
        currentlyLoading.erase(currentLoadingIt);
    }
    
    TileAsset tileAsset;
    QuadTreeNew::Node ident(loadReturn.tileID.x,loadReturn.tileID.y,loadReturn.tileID.level);
    
    // Creat the image and tie it to the drawables
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
                // Turn on the objects
                if (loadedTile->enabled)
                    [interactLayer enableObjects:loadReturn.compObjs changes:changes];
                
                // Build the image
                Texture *tex = [loadedImage buildTexture:borderPixel destWidth:loadedImage.width destHeight:loadedImage.height];
                if (tex) {
                    success = true;
                    // Create the texture in the renderer
                    tileAsset.texID = tex->getId();
                    changes.push_back(new AddTextureReq(tex));
                    
                    // Assign it to the various drawables
                    for (auto drawID : loadedTile->drawIDs)
                        changes.push_back(new DrawTexChangeRequest(drawID,0,tileAsset.texID));
                }
            }
        }
    }
    
    tileAsset.compObjs = loadReturn.compObjs;
    
    if (success) {
        // This shouldn't happen, but what if there's one already there?
        auto loadedIt = loaded.find(ident);
        if (loadedIt != loaded.end()) {
            loadedIt->second.clear(interactLayer,changes);
        }
        // The asset (texture) is matched directly to the node
        tileAsset.texNode = ident;
        loaded[ident] = tileAsset;
        
        // See if it's useful elsewhere
        [self applyCoverTile:ident asset:tileAsset changes:changes];
    }
        
    [layer.layerThread addChangeRequests:changes];

    [self updateLoading];

    return true;
}

- (void)registerTile:(MaplyTileID)tileID frame:(int)frame data:(id __nullable)tileData
{
    if ([NSThread currentThread] != layer.layerThread)
        return;
    
    QuadTreeNew::Node node(tileID.level,tileID.x,tileID.y);
    if (currentlyLoading.find(node) != currentlyLoading.end())
        currentlyLoading[node] = tileData;
}

// Evaluate and possibly modify the texture for a given tile based on a possible cover tile
- (void)evalCoverTile:(const QuadTreeNew::Node &)coverIdent coverAsset:(const TileAsset &)coverAsset tileIdent:(const QuadTreeNew::Node &)tileIdent changes:(ChangeSet &)changes
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
            changes.push_back(new DrawTexChangeRequest(drawID,0,coverAsset.texID,relLevel,relX,relY));
        }
    }
}

// Look for a texture we've already loaded that could be used for this new tile
- (void)findCoverTile:(const QuadTreeNew::Node &)tileIdent changes:(ChangeSet &)changes
{
    for (auto it : loaded) {
        auto coverIdent = it.first;
        auto coverAsset = it.second;
        if (coverIdent.level < tileIdent.level)
            [self evalCoverTile:coverIdent coverAsset:coverAsset tileIdent:tileIdent changes:changes];
    }
}

// Apply the texture for a tile we just loaded
- (void)applyCoverTile:(const QuadTreeNew::Node &)coverIdent asset:(const TileAsset &)coverAsset changes:(ChangeSet &)changes
{
    // Now see if there are any other places to use this texture
    for (QuadTreeNew::Node tileIdent : toLoad) {
        if (coverIdent.level < tileIdent.level) {
            [self evalCoverTile:coverIdent coverAsset:coverAsset tileIdent:tileIdent changes:changes];
        }
    }
}

@end
