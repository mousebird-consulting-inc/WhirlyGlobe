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
#import "MaplyViewController_private.h"

using namespace WhirlyKit;

namespace WhirlyKit
{
    
// Used to track tiles in the process of loading
class QuadPagingLoadedTile
{
public:
    QuadPagingLoadedTile()
    {
        addCompObjs = nil;
        replaceCompObjs = nil;
        isLoading = false;
        enable = false;
        childrenEnable = false;
        numParts = 0;
    }
    QuadPagingLoadedTile(const WhirlyKit::Quadtree::Identifier &ident)
    {
        addCompObjs = nil;
        replaceCompObjs = nil;
        nodeIdent = ident;
        isLoading = false;
        enable = false;
        childrenEnable = false;
        numParts = 0;
    }
    ~QuadPagingLoadedTile() { }

    // Component objects that add to geometry above and below
    NSMutableArray *addCompObjs;
    // Component objects that replace geometry above
    NSMutableArray *replaceCompObjs;

    // Details of which node we're representing
    WhirlyKit::Quadtree::Identifier nodeIdent;

    /// Set if this tile is in the process of loading
    bool isLoading;
    
    /// Set if this tile successfully loaded
    bool didLoad;
    
    /// Keep track of whether the visable objects are enabled
    bool enable;
    
    /// If set, our children our enabled, but not us.
    bool childrenEnable;
    
    /// If the source is loading tiles in pieces, they'll set this
    int numParts;
    
    void addToCompObjs(MaplyQuadPagingDataStyle dataStyle,NSArray *newObjs)
    {
        switch (dataStyle)
        {
            case MaplyDataStyleAdd:
                if (!addCompObjs)
                    addCompObjs = [NSMutableArray array];
                [addCompObjs addObjectsFromArray:newObjs];
                break;
            case MaplyDataStyleReplace:
                if (!replaceCompObjs)
                    replaceCompObjs = [NSMutableArray array];
                [replaceCompObjs addObjectsFromArray:newObjs];
                break;
        }
    }
};
    
/// This is a comparison operator for sorting loaded tile pointers by
/// Quadtree node identifier.
typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const QuadPagingLoadedTile *a,const QuadPagingLoadedTile *b)
    {
        return a->nodeIdent < b->nodeIdent;
    }
} QuadPagingLoadedTileSorter;

    
typedef std::set<QuadPagingLoadedTile *,QuadPagingLoadedTileSorter> QuadPagingLoadedTileSet;

}

// Simple wrapper for tile IDs so we can pass them as objects
@interface MaplyTileIDObject : NSObject

@property (nonatomic,assign) MaplyTileID tileID;
+ (MaplyTileIDObject *) tileWithTileID:(MaplyTileID)tileID;

@end

@implementation MaplyTileIDObject

+ (MaplyTileIDObject *) tileWithTileID:(MaplyTileID)tileID
{
    MaplyTileIDObject *tileObj = [[MaplyTileIDObject alloc] init];
    tileObj.tileID = tileID;
    
    return tileObj;
}

@end

@implementation MaplyQuadPagingLayer
{
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
    _importance = 512*512;
    
    return self;
}

- (void)dealloc
{
    pthread_mutex_lock(&tileSetLock);
    for (QuadPagingLoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
        delete *it;
    tileSet.clear();
    pthread_mutex_unlock(&tileSetLock);
    
    pthread_mutex_destroy(&tileSetLock);
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)inViewC
{
    super.layerThread = inLayerThread;
    scene = inScene;
    _viewC = inViewC;
    
    // Cache min and max zoom.  Tile sources might do a lookup for these
    minZoom = [tileSource minZoom];
    maxZoom = [tileSource maxZoom];
    
    // Set up tile and and quad layer with us as the data source
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:self renderer:renderer];
    // A tile needs to take up this much screen space
    quadLayer.minImportance = _importance;
    quadLayer.maxTiles = 256;
    
    [super.layerThread addLayer:quadLayer];
    
    return true;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [inLayerThread removeLayer:quadLayer];
    
    // Note: Need to wait for anything that's been dispatched to finish
    pthread_mutex_lock(&tileSetLock);
    for (QuadPagingLoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
        delete *it;
    tileSet.clear();
    pthread_mutex_unlock(&tileSetLock);
}

- (void)geoBoundsforTile:(MaplyTileID)tileID ll:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur
{
    if (!quadLayer || !quadLayer.quadtree || !scene || !scene->getCoordAdapter())
        return;
    
    Mbr mbr = quadLayer.quadtree->generateMbrForNode(WhirlyKit::Quadtree::Identifier(tileID.x,tileID.y,tileID.level));
    
    GeoMbr geoMbr;
    CoordSystem *wkCoordSys = quadLayer.coordSys;
    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ll().x(),mbr.ll().y(),0.0)));
    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ur().x(),mbr.ll().y(),0.0)));
    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ur().x(),mbr.ur().y(),0.0)));
    geoMbr.addGeoCoord(wkCoordSys->localToGeographic(Point3f(mbr.ll().x(),mbr.ur().y(),0.0)));
    
    ll->x = geoMbr.ll().x();
    ll->y = geoMbr.ll().y();
    ur->x = geoMbr.ur().x();
    ur->y = geoMbr.ur().y();
}

- (void)boundsforTile:(MaplyTileID)tileID ll:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur
{
    Mbr mbr = quadLayer.quadtree->generateMbrForNode(WhirlyKit::Quadtree::Identifier(tileID.x,tileID.y,tileID.level));
    
    ll->x = mbr.ll().x();
    ll->y = mbr.ll().y();
    ur->x = mbr.ur().x();
    ur->y = mbr.ur().y();
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
    return 0;
}

/// Return the maximum quad tree zoom level.  Must be at least minZoom
- (int)maxZoom
{
    return maxZoom;
}

/// Return an importance value for the given tile
- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *) viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
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
    double import = ScreenImportance(viewState, frameSize, viewState.eyeVec, 1, [coordSys getCoordSystem], scene->getCoordAdapter(), parentMbr, ident, attrs) / 4;
    
    // Just the importance of this tile.
//    float import = ScreenImportance(viewState, frameSize, viewState->eyeVec, 1, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
    
//    NSLog(@"tile %d: (%d,%d) = %f",ident.level,ident.x,ident.y,import);
    
    return import;
}

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown
{
    super.layerThread = nil;
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
    MaplyTileID tileID;
    tileID.x = tileInfo.ident.x;
    tileID.y = tileInfo.ident.y;
    tileID.level = tileInfo.ident.level;
    
    bool isThere = false;
    
    // Look for the existing tile, just in case
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileInfo.ident);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
        isThere = true;
    pthread_mutex_unlock(&tileSetLock);

    // Now that's weird, why are we loading the tile twice?
    if (isThere)
        return;
    numFetches++;
    
    // Okay, let's add it
    QuadPagingLoadedTile *newTile = new QuadPagingLoadedTile(tileInfo.ident);
    newTile->isLoading = true;
    newTile->didLoad = false;
    newTile->enable = false;
    pthread_mutex_lock(&tileSetLock);
    tileSet.insert(newTile);
    pthread_mutex_unlock(&tileSetLock);
    
    // Now let the delegate know we'd like that tile.
    if (tileID.level >= minZoom)
        [tileSource startFetchForTile:tileID forLayer:self];
    else {
        [self tileDidLoad:tileID];
    }
    
//    NSLog(@"Tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
}

// Called on the layer thread
// Clean out the data created for the tile
- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer unloadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    QuadPagingLoadedTile *tile = NULL;
    NSArray *addCompObjs = nil,*replaceCompObjs = nil;
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileInfo.ident);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        tile = *it;
        addCompObjs = (*it)->addCompObjs;
        replaceCompObjs = (*it)->replaceCompObjs;
        tileSet.erase(it);
    }
    pthread_mutex_unlock(&tileSetLock);
    delete tile;

    [_viewC removeObjects:addCompObjs];
    [_viewC removeObjects:replaceCompObjs];
    
    // Check the parent
    if (tileInfo.ident.level >= minZoom)
    {
        MaplyTileID parentID;
        parentID.x = tileInfo.ident.x/2;
        parentID.y = tileInfo.ident.y/2;
        parentID.level = tileInfo.ident.level-1;
        [self runTileUpdate];
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
        tile->addToCompObjs(MaplyDataStyleAdd,dataObjects);
    
    pthread_mutex_unlock(&tileSetLock);
}

- (void)addData:(NSArray *)dataObjects forTile:(MaplyTileID)tileID style:(MaplyQuadPagingDataStyle)dataStyle
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
        tile->addToCompObjs(dataStyle, dataObjects);
    
    pthread_mutex_unlock(&tileSetLock);
}

// Notify the quad paging layer that we loaded, but do it on the layer thread
- (void)loadFailNotify:(MaplyTileIDObject *)tileIDObj
{
    MaplyTileID tileID = tileIDObj.tileID;
    [quadLayer loader:self tileDidNotLoad:Quadtree::Identifier(tileID.x,tileID.y,tileID.level)];
}

// If it failed, clear out the tile
- (void)tileFailedToLoad:(MaplyTileID)tileID
{
    Quadtree::Identifier tileIdent(tileID.x,tileID.y,tileID.level);

    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileIdent);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        tileSet.erase(it);
        delete *it;
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
        [self runTileUpdate];
    }
    
    [self performSelector:@selector(loadFailNotify:) onThread:super.layerThread withObject:[MaplyTileIDObject tileWithTileID:tileID] waitUntilDone:NO];
}

// Recursively evaluate what tiles should and shouldn't be on
// This assumes the caller has the tile lock
- (void)evaluateTile:(QuadPagingLoadedTile *)tile enable:(bool)enable toEnable:(NSMutableArray *)toEnable toDisable:(NSMutableArray *)toDisable
{
    // Look for children
    std::vector<QuadPagingLoadedTile *> children;
    for (unsigned int ix=0;ix<2;ix++)
        for (unsigned int iy=0;iy<2;iy++)
        {
            MaplyTileID childID;
            childID.x = 2*tile->nodeIdent.x + ix;
            childID.y = 2*tile->nodeIdent.y + iy;
            childID.level = tile->nodeIdent.level + 1;
            QuadPagingLoadedTile childTile(Quadtree::Identifier(childID.x,childID.y,childID.level));
            QuadPagingLoadedTileSet::iterator it = tileSet.find(&childTile);
            if (it != tileSet.end() && (*it)->didLoad)
                children.push_back((*it));
        }

    if (enable)
    {
        // Enable all the children and disable ourselves
        if (children.size() == 4)
        {
            for (unsigned int ii=0;ii<4;ii++)
                [self evaluateTile:children[ii] enable:enable toEnable:toEnable toDisable:toDisable];
            if (tile->enable)
            {
                tile->enable = false;
                [toDisable addObjectsFromArray:tile->replaceCompObjs];
            }
        } else {
            // Disable the children
            for (unsigned int ii=0;ii<children.size();ii++)
                [self evaluateTile:children[ii] enable:false toEnable:toEnable toDisable:toDisable];
            // Enable ourselves
            if (!tile->isLoading && !tile->enable)
            {
                tile->enable = true;
                [toEnable addObjectsFromArray:tile->replaceCompObjs];
            }
        }
    } else {
        // Disable any children
        for (unsigned int ii=0;ii<children.size();ii++)
            [self evaluateTile:children[ii] enable:false toEnable:toEnable toDisable:toDisable];
        // And ourselves
        if (tile->enable)
        {
            tile->enable = false;
            [toDisable addObjectsFromArray:tile->replaceCompObjs];
        }
    }
}

// Re-evaluate what's visible, starting at the top
- (void)runTileUpdate
{
    NSMutableArray *toEnable = [NSMutableArray array],*toDisable = [NSMutableArray array];
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(Quadtree::Identifier(0,0,0));
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
        [self evaluateTile:*it enable:true toEnable:toEnable toDisable:toDisable];
        
    pthread_mutex_unlock(&tileSetLock);

    if ([toEnable count] > 0)
        [_viewC enableObjects:toEnable mode:MaplyThreadAny];
    if ([toDisable count] > 0)
        [_viewC disableObjects:toDisable mode:MaplyThreadAny];
}

// Notify the quad paging layer that we loaded, but do it on the layer thread
- (void)loadNotify:(MaplyTileIDObject *)tileIDObj
{
    MaplyTileID tileID = tileIDObj.tileID;
    [quadLayer loader:self tileDidLoad:Quadtree::Identifier(tileID.x,tileID.y,tileID.level)];
}

- (void)tileDidLoad:(MaplyTileID)tileID
{
    NSArray *addCompObjs = nil;
    
    Quadtree::Identifier tileIdent(tileID.x,tileID.y,tileID.level);
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileIdent);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        QuadPagingLoadedTile *tile = *it;
        tile->isLoading = false;
        tile->didLoad = true;
        addCompObjs = tile->addCompObjs;
    }
    numFetches--;
    pthread_mutex_unlock(&tileSetLock);

    // These are added immediately and stick around till the tile is deleted
    if ([addCompObjs count] > 0)
        [_viewC enableObjects:addCompObjs mode:MaplyThreadAny];
    
    [self runTileUpdate];
    
    [self performSelector:@selector(loadNotify:) onThread:super.layerThread withObject:[MaplyTileIDObject tileWithTileID:tileID] waitUntilDone:NO];
}

- (void)tile:(MaplyTileID)tileID hasNumParts:(int)numParts
{
    Quadtree::Identifier tileIdent(tileID.x,tileID.y,tileID.level);
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileIdent);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        QuadPagingLoadedTile *tile = *it;
        tile->numParts = numParts;
    }
    pthread_mutex_unlock(&tileSetLock);
}

- (void)tileDidLoad:(MaplyTileID)tileID part:(int)whichPart
{
    bool finishedLoad = false;
    
    Quadtree::Identifier tileIdent(tileID.x,tileID.y,tileID.level);
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileIdent);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        QuadPagingLoadedTile *tile = *it;
        tile->numParts--;
        finishedLoad = tile->numParts <= 0;
    }
    pthread_mutex_unlock(&tileSetLock);
    
    if (finishedLoad)
        [self tileDidLoad:tileID];
}

// As long as we're not loading the tile, we can load the children
- (bool)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer canLoadChildrenOfTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    bool ret = false;
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileInfo.ident);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        ret = !(*it)->isLoading;
    }
    pthread_mutex_unlock(&tileSetLock);    
    
    return ret;
}

- (void)clearContents
{
    NSMutableArray *compObjs = [NSMutableArray array];
    pthread_mutex_lock(&tileSetLock);
    for (QuadPagingLoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
    {
        if ((*it)->replaceCompObjs)
            [compObjs addObjectsFromArray:(*it)->replaceCompObjs];
        if ((*it)->addCompObjs)
            [compObjs addObjectsFromArray:(*it)->addCompObjs];
        delete *it;
    }
    tileSet.clear();
    pthread_mutex_unlock(&tileSetLock);
    
    [_viewC removeObjects:compObjs];
}

- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene
{
    [self clearContents];
}

- (void)reload
{
    if ([NSThread currentThread] != super.layerThread)
    {
        [self performSelector:@selector(reload) onThread:super.layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    [quadLayer refresh];
}

@end
