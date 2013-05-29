/*
 *  MaplyQuadPagingLayer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/20/13.
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

#import "MaplyQuadPagingLayer_private.h"
#import "MaplyCoordinateSystem_private.h"

using namespace WhirlyKit;

namespace WhirlyKit
{
    
// Used to track tiles in the process of loading
class QuadPagingLoadedTile
{
public:
    QuadPagingLoadedTile()
    {
        compObjs = nil;
        isLoading = false;
    }
    QuadPagingLoadedTile(const WhirlyKit::Quadtree::Identifier &ident)
    {
        compObjs = nil;
        nodeInfo.ident = ident;
        isLoading = false;
    }
    ~QuadPagingLoadedTile() { }

    // Objects created by the delegate for this tile
    NSMutableArray *compObjs;

    // Details of which node we're representing
    WhirlyKit::Quadtree::NodeInfo nodeInfo;

    /// Set if this tile is in the process of loading
    bool isLoading;
    
    /// Set if this tile successfully loaded
    bool didLoad;
    
    void addCompObjs(NSArray *newObjs)
    {
        if (!compObjs)
            compObjs = [NSMutableArray array];
        [compObjs addObjectsFromArray:newObjs];
    }
};
    
/// This is a comparison operator for sorting loaded tile pointers by
/// Quadtree node identifier.
typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const QuadPagingLoadedTile *a,const QuadPagingLoadedTile *b)
    {
        return a->nodeInfo.ident < b->nodeInfo.ident;
    }
} QuadPagingLoadedTileSorter;

    
typedef std::set<QuadPagingLoadedTile *,QuadPagingLoadedTileSorter> QuadPagingLoadedTileSet;

}

@implementation MaplyQuadPagingLayer
{
    WhirlyKitLayerThread * __weak layerThread;
    WhirlyKitQuadDisplayLayer *quadLayer;
    Scene *scene;
    MaplyCoordinateSystem *coordSys;
    NSObject<MaplyPagingDelegate> *tileSource;
    int minZoom,maxZoom;
    // Ongoing fetches
    int numFetches;
    // Tiles we've loaded or are loading
    pthread_mutex_t tileSetLock;
    QuadPagingLoadedTileSet tileSet;
}

- (id)initWithCoordSystem:(MaplyCoordinateSystem *)inCoordSys delegate:(NSObject<MaplyPagingDelegate> *)inTileSource
{
    self = [super init];
    
    coordSys = inCoordSys;
    tileSource = inTileSource;
    _numSimultaneousFetches = 8;
    pthread_mutex_init(&tileSetLock, NULL);
    
    return self;
}

- (void)dealloc
{
    pthread_mutex_lock(&tileSetLock);
    for (QuadPagingLoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
        delete *it;
    pthread_mutex_unlock(&tileSetLock);
    
    pthread_mutex_destroy(&tileSetLock);
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)inViewC
{
    layerThread = inLayerThread;
    scene = inScene;
    _viewC = inViewC;
    
    // Cache min and max zoom.  Tile sources might do a lookup for these
    minZoom = [tileSource minZoom];
    maxZoom = [tileSource maxZoom];
    
    // Set up tile and and quad layer with us as the data source
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:self renderer:renderer];
    // A tile needs to take up this much screen space
    quadLayer.minImportance = 256*256;
    
    [layerThread addLayer:quadLayer];
    
    return true;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [inLayerThread removeLayer:quadLayer];
    
    // Note: Need to wait for anything that's been dispatched to finish
}

- (void)geoBoundsforTile:(MaplyTileID)tileID ll:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur
{
    Mbr mbr = quadLayer.quadtree->generateMbrForNode(WhirlyKit::Quadtree::Identifier(tileID.x,tileID.y,tileID.level));
    
    GeoMbr geoMbr;
    CoordSystem *wkCoordSys = scene->getCoordAdapter()->getCoordSystem();
    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ll().x(),mbr.ll().y(),0.0)));
    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ur().x(),mbr.ll().y(),0.0)));
    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ur().x(),mbr.ur().y(),0.0)));
    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ll().x(),mbr.ur().y(),0.0)));
    
    ll->x = geoMbr.ll().x();
    ll->y = geoMbr.ll().y();
    ur->x = geoMbr.ur().x();
    ur->y = geoMbr.ur().y();
}

#pragma mark - WhirlyKitQuadDataStructure protocol

/// Return the coordinate system we're working in
- (WhirlyKit::CoordSystem *)coordSystem
{
    return [coordSys getCoordSystem];
}

/// Bounding box used to calculate quad tree nodes.  In local coordinate system.
- (WhirlyKit::Mbr)totalExtents
{
    MaplyCoordinate ll,ur;
    [coordSys getBoundsLL:&ll ur:&ur];
    
    Mbr mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y));
    return mbr;
}

/// Bounding box of data you actually want to display.  In local coordinate system.
/// Unless you're being clever, make this the same as totalExtents.
- (WhirlyKit::Mbr)validExtents
{
    return [self totalExtents];
}

/// Return the minimum quad tree zoom level (usually 0)
- (int)minZoom
{
    return minZoom;
}

/// Return the maximum quad tree zoom level.  Must be at least minZoom
- (int)maxZoom
{
    return maxZoom;
}

/// Return an importance value for the given tile
- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *) viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    if (ident.level <= 1)
        return MAXFLOAT;
    
    // For a child tile, we're taking the size of our parent so all the children load at once
    WhirlyKit::Quadtree::Identifier parentIdent;
    parentIdent.x = ident.x / 2;
    parentIdent.y = ident.y / 2;
    parentIdent.level = ident.level - 1;
    
    Mbr parentMbr = quadLayer.quadtree->generateMbrForNode(parentIdent);

    // This is how much screen real estate we're covering for this tile
    float import = ScreenImportance(viewState, frameSize, viewState->eyeVec, 1, [coordSys getCoordSystem], scene->getCoordAdapter(), parentMbr, ident, attrs) / 4;
    
//    NSLog(@"tile (%d,%d,%d) = %f",ident.x,ident.y,ident.level,import);
    
    return import;
}

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown
{
    layerThread = nil;
    quadLayer = nil;
}

#pragma mark - WhirlyKitQuadLoader

- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer
{
    quadLayer = layer;
}

- (bool)isReady
{
    return (numFetches < _numSimultaneousFetches);
}

- (void)quadDisplayLayerStartUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)quadDisplayLayerEndUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

// Called on the layer thread
- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer loadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    // Look for the existing tile, just in case
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileInfo.ident);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    pthread_mutex_unlock(&tileSetLock);

    // Now that's weird, why are we loading the tile twice?
    if (it != tileSet.end())
        return;
    numFetches++;
    
    // Okay, let's add it
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile *newTile = new QuadPagingLoadedTile(tileInfo.ident);
    newTile->isLoading = true;
    newTile->didLoad = false;
    tileSet.insert(newTile);
    pthread_mutex_unlock(&tileSetLock);
    
    // Now let the delegate know we'd like that tile.
    MaplyTileID tileID;
    tileID.x = tileInfo.ident.x;
    tileID.y = tileInfo.ident.y;
    tileID.level = tileInfo.ident.level;
    [tileSource startFetchForTile:tileID forLayer:self];
}

// Called on the layer thread
// Clean out the data created for the tile
- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer unloadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    NSArray *compObjs = nil;
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileInfo.ident);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        compObjs = (*it)->compObjs;
        tileSet.erase(it);
    }
    pthread_mutex_unlock(&tileSetLock);

    [_viewC removeObjects:compObjs];
    
    // Check the parent
    if (tileInfo.ident.level > 0)
    {
        MaplyTileID parentID;
        parentID.x = tileInfo.ident.x/2;
        parentID.y = tileInfo.ident.y/2;
        parentID.level = tileInfo.ident.level-1;
        [self updateTile:parentID];
    }

}

// Called by the delegate.  We'll track the data they added.
- (void)addData:(NSArray *)dataObjects forTile:(MaplyTileID)tileID
{
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(Quadtree::Identifier(tileID.x,tileID.y,tileID.level));
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    // Didn't find it, so immediately delete the thing
    if (it == tileSet.end())
    {
        [_viewC removeObjects:dataObjects];
        pthread_mutex_unlock(&tileSetLock);
        return;
    }
    QuadPagingLoadedTile *tile = *it;
    
    if (dataObjects)
        tile->addCompObjs(dataObjects);
    
    pthread_mutex_unlock(&tileSetLock);
}

// If it failed, clear out the tile
- (void)tileFailedToLoad:(MaplyTileID)tileID
{
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(Quadtree::Identifier(tileID.x,tileID.y,tileID.level));
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
        tileSet.erase(it);
    numFetches--;
    pthread_mutex_unlock(&tileSetLock);
    
    // Check the parent
    if (tileID.level > 0)
    {
        MaplyTileID parentID;
        parentID.x = tileID.x/2;
        parentID.y = tileID.y/2;
        parentID.level = tileID.level-1;
        [self updateTile:parentID];
    }
}

// See if all or some of the children are loaded
- (void)updateTile:(MaplyTileID)tileID
{
    int numChildrenLoaded = 0;
    NSArray *toRemove = nil;
    bool refetch = false;
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(Quadtree::Identifier(tileID.x,tileID.y,tileID.level));
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        QuadPagingLoadedTile *tile = *it;
        
        // Look for children
        for (unsigned int ix=0;ix<2;ix++)
            for (unsigned int iy=0;iy<2;iy++)
            {
                MaplyTileID childID;
                childID.x = 2*tileID.x + ix;
                childID.y = 2*tileID.y + iy;
                childID.level = tileID.level + 1;
                QuadPagingLoadedTile childTile(Quadtree::Identifier(childID.x,childID.y,childID.level));
                QuadPagingLoadedTileSet::iterator it = tileSet.find(&childTile);
                if (it != tileSet.end())
                    numChildrenLoaded++;
            }

        // If any of the children are loaded, they're probably all loaded
        if (numChildrenLoaded > 0)
        {
            toRemove = tile->compObjs;
            tile->compObjs = nil;
            tile->didLoad = false;
        } else {
            if (!tile->isLoading && !tile->didLoad)
            {
                // Otherwise, we might have back off from four, reload
                tile->isLoading = true;
                tile->didLoad = false;
                
                refetch = true;
            }
        }
    }

    pthread_mutex_unlock(&tileSetLock);

    // Now let the delegate know we'd like that tile.
    if (refetch)
        [tileSource startFetchForTile:tileID forLayer:self];
    
    if (toRemove)
        [_viewC removeObjects:toRemove];
}

- (void)tileDidLoad:(MaplyTileID)tileID
{
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(Quadtree::Identifier(tileID.x,tileID.y,tileID.level));
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        QuadPagingLoadedTile *tile = *it;
        tile->isLoading = false;
        tile->didLoad = true;
    }
    numFetches--;
    pthread_mutex_unlock(&tileSetLock);
    
    // Check the parent
    if (tileID.level > 0)
    {
        MaplyTileID parentID;
        parentID.x = tileID.x/2;
        parentID.y = tileID.y/2;
        parentID.level = tileID.level-1;
        [self updateTile:parentID];
    }
}

// As long as we're not loading the tile, we can load the children
- (bool)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer canLoadChildrenOfTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    bool ret = false;
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileInfo.ident);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    pthread_mutex_unlock(&tileSetLock);
    
    if (it != tileSet.end())
    {
        ret = !(*it)->isLoading;
    }
    
    return ret;
}

- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene
{
    NSMutableArray *compObjs = [NSMutableArray array];
    pthread_mutex_lock(&tileSetLock);
    for (QuadPagingLoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
        if ((*it)->compObjs)
            [compObjs addObjectsFromArray:(*it)->compObjs];
    pthread_mutex_unlock(&tileSetLock);
    
    [_viewC removeObjects:compObjs];
}

@end
