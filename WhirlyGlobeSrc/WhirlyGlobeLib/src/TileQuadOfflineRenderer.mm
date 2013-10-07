/*
 *  TileQuadOfflineRenderer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/7/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "TileQuadOfflineRenderer.h"

using namespace WhirlyKit;

namespace WhirlyKit
{

class OfflineTile
{
public:
    OfflineTile() : isLoading(false), placeholder(false) { };
    OfflineTile(const WhirlyKit::Quadtree::Identifier &ident) : ident(ident), isLoading(false), placeholder(false) { }
    OfflineTile(std::vector<WhirlyKitLoadedImage *>loadImages) : isLoading(false), placeholder(false) { }
    ~OfflineTile()
    {
    }
    
    // Details of which node we're representing
    WhirlyKit::Quadtree::Identifier ident;

    /// Set if this is just a placeholder (no geometry)
    bool placeholder;
    /// Set if this tile is in the process of loading
    bool isLoading;
    
    std::vector<WhirlyKitLoadedImage *> images;
};

typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const OfflineTile *a,const OfflineTile *b)
    {
        return a->ident < b->ident;
    }
} OfflineTileSorter;

/// A set that sorts loaded MB Tiles by Quad tree identifier
typedef std::set<OfflineTile *,OfflineTileSorter> OfflineTileSet;

}

@implementation WhirlyKitQuadTileOfflineLoader
{
    WhirlyKitQuadDisplayLayer *_quadLayer;
    NSString *_name;
    NSObject<WhirlyKitQuadTileImageDataSource> *_imageSource;
    OfflineTileSet tiles;
    int numFetches;
}

- (id)initWithName:(NSString *)name dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource
{
    self = [super init];
    if (!self)
        return nil;

    _name = name;
    _imageSource = imageSource;
    _numImages = 1;
    _sizeX = _sizeY = 1024;
    _mbr = Mbr(GeoCoord::CoordFromDegrees(0.0,0.0),GeoCoord::CoordFromDegrees(1.0, 1.0));
    
    return self;
}

- (void)clear
{
    for (OfflineTileSet::iterator it = tiles.begin();it != tiles.end();++it)
        delete *it;
    tiles.clear();
}

- (void)dealloc
{
    [self clear];
}

#pragma mark - WhirlyKitQuadLoader

- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene
{
    [self clear];
}

- (bool)isReady
{
    return (numFetches <= [_imageSource maxSimultaneousFetches]);
}

- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer
{
    _quadLayer = layer;
}

- (void)quadDisplayLayerStartUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)quadDisplayLayerEndUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer loadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    OfflineTile *newTile = new OfflineTile(tileInfo.ident);
    newTile->isLoading = true;
    
    tiles.insert(newTile);
    
    [_imageSource quadTileLoader:self startFetchForLevel:tileInfo.ident.level col:tileInfo.ident.x row:tileInfo.ident.y attrs:tileInfo.attrs];
    numFetches++;
}

- (OfflineTile *)getTile:(const WhirlyKit::Quadtree::Identifier)ident
{
    OfflineTile dummyTile(ident);
    OfflineTileSet::iterator it = tiles.find(&dummyTile);
    if (it == tiles.end())
        return nil;
    
    return *it;
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer unloadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    OfflineTile dummyTile(tileInfo.ident);
    OfflineTileSet::iterator it = tiles.find(&dummyTile);
    if (it != tiles.end())
    {
        OfflineTile *theTile = *it;
        delete theTile;
        tiles.erase(it);
    }
}

- (bool)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer canLoadChildrenOfTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    OfflineTile *tile = [self getTile:tileInfo.ident];
    if (!tile)
        return false;
    
    return !tile->isLoading;
}

- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadTile forLevel:(int)level col:(int)col row:(int)row
{
    numFetches--;
    Quadtree::Identifier tileIdent(col,row,level);
    OfflineTile *tile = [self getTile:tileIdent];
    if (!tile)
        return;

    tile->isLoading = false;

    // Assemble the iamges
    std::vector<WhirlyKitLoadedImage *> loadImages;
    if ([loadTile isKindOfClass:[WhirlyKitLoadedImage class]])
        loadImages.push_back(loadTile);
    else if ([loadTile isKindOfClass:[WhirlyKitLoadedTile class]])
    {
        WhirlyKitLoadedTile *toLoad = loadTile;
        
        for (WhirlyKitLoadedImage *loadImage in toLoad.images)
            loadImages.push_back(loadImage);
    }
    if (_numImages != loadImages.size())
    {
        if (loadTile)
            NSLog(@"TileQuadLoader: Got %ld images in callback, but was expecting %d.  Punting tile.",loadImages.size(),_numImages);
        [_quadLayer loader:self tileDidNotLoad:tileIdent];
        return;
    }

    NSLog(@"Loaded tile %d: (%d,%d)",level,col,row);
    [_quadLayer loader:self tileDidLoad:tileIdent];
}

@end
