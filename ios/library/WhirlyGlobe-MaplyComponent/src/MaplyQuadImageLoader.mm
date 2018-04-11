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

namespace WhirlyKit
{

// Keep track of what we've already loaded
class TileAsset
{
public:
    // Clean out assets
    void clear(ChangeSet &changes) {
        if (texID != EmptyIdentity)
            changes.push_back(new RemTextureReq(texID));
    }
    
    SimpleIdentity texID;
};
    
typedef std::map<QuadTreeNew::Node,TileAsset> TileAssetMap;
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
    WhirlyKit::QuadTreeNew::NodeSet currentlyLoading;
    // Tiles we've actually loaded and are active in memory
    WhirlyKit::TileAssetMap loaded;
}

- (instancetype)initWithTileSource:(NSObject<MaplyTileSource> *)inTileSource
{
    tileSource = inTileSource;
    if (![tileSource respondsToSelector:@selector(startFetchLayer:tile:frame:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement startFetchLayer:tile:frame:");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(cancelTile:frame:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement cancelTile:frame:");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(clear)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement clear");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(validTile:bbox:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement validTile:bbox:");
        return nil;
    }
    
    _numSimultaneousFetches = 16;
    _debugMode = true;

    self = [super init];
    return self;
}

// MARK: Quad Build Delegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)inBuilder layer:(WhirlyKitQuadDisplayLayerNew * __nonnull)inLayer
{
    builder = inBuilder;
    layer = inLayer;
}

// Called on the layer thread
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder loadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles
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
        
        currentlyLoading.insert(*nextLoad);
        
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
- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder unLoadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles
{
    ChangeSet changes;
    
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
            [tileSource cancelTile:tileID frame:-1];
            
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
            loadedIt->second.clear(changes);

            loaded.erase(loadedIt);
        }
    }
    
    [self updateLoading];
}

// Called from anywhere
- (bool)loadedReturn:(MaplyQuadImageLoaderReturn *)loadReturn
{
    // Note: Check the data coming in and return false if it's bad
    
    if (layer.layerThread != [NSThread currentThread]) {
        [self performSelector:@selector(loadedReturn:) onThread:layer.layerThread withObject:loadReturn waitUntilDone:NO];
        return true;
    }

    if (_debugMode)
        NSLog(@"Loaded %d: (%d,%d)",loadReturn.tileID.level,loadReturn.tileID.x,loadReturn.tileID.y);

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
    WhirlyKitLoadedTile *loadTile = [tileData wkTile:borderPixel convertToRaw:true];
    if ([loadTile.images count] > 0) {
        WhirlyKitLoadedImage *loadedImage = [loadTile.images objectAtIndex:0];
        if ([loadedImage isKindOfClass:[WhirlyKitLoadedImage class]]) {
            Texture *tex = [loadedImage buildTexture:borderPixel destWidth:loadedImage.width destHeight:loadedImage.height];
            if (tex) {
                // Create the texture in the renderer
                tileAsset.texID = tex->getId();
                changes.push_back(new AddTextureReq(tex));
                
                // Assign it to the various drawables
                LoadedTileNewRef loadedTile = [builder getLoadedTile:ident];
                if (loadedTile) {
                    for (auto drawID : loadedTile->drawIDs)
                        changes.push_back(new DrawTexChangeRequest(drawID,0,tileAsset.texID));
                }
            }
        }
    }
    
    // This shouldn't happen, but what if there's one already there?
    auto loadedIt = loaded.find(ident);
    if (loadedIt != loaded.end()) {
        loadedIt->second.clear(changes);
    }
    loaded[ident] = tileAsset;

    [self updateLoading];
    
    [layer.layerThread addChangeRequests:changes];

    return true;
}

@end
