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
        refreshing = false;
        childrenEnable = false;
        numParts = 0;
    }
    QuadPagingLoadedTile(const MaplyTileID &ident)
    {
        addCompObjs = nil;
        replaceCompObjs = nil;
        nodeIdent = ident;
        isLoading = false;
        enable = false;
        refreshing = false;
        childrenEnable = false;
        numParts = 0;
    }
    ~QuadPagingLoadedTile() { }

    // Component objects that add to geometry above and below
    NSMutableArray *addCompObjs;
    // Component objects that replace geometry above
    NSMutableArray *replaceCompObjs;
    
    // While refreshing, we store these here temporarily
    NSMutableArray *refAddCompObjs;
    NSMutableArray *refReplaceCompObjs;

    // Details of which node we're representing
    MaplyTileID nodeIdent;

    /// Set if this tile is in the process of loading
    bool isLoading;
    
    /// Set if this tile successfully loaded
    bool didLoad;
    
    /// Keep track of whether the visable objects are enabled
    bool enable;
    
    /// Set if we've asked the delegate to refresh this one
    bool refreshing;
    
    /// If set, our children our enabled, but not us.
    bool childrenEnable;
    
    /// If the source is loading tiles in pieces, they'll set this
    int numParts;
    
    void addToCompObjs(MaplyQuadPagingDataStyle dataStyle,NSArray *newObjs)
    {
        switch (dataStyle)
        {
            case MaplyDataStyleAdd:
                if (refreshing)
                {
                    if (!refAddCompObjs)
                        refAddCompObjs = [NSMutableArray array];
                    [refAddCompObjs addObjectsFromArray:newObjs];
                } else {
                    if (!addCompObjs)
                        addCompObjs = [NSMutableArray array];
                    [addCompObjs addObjectsFromArray:newObjs];
                }
                break;
            case MaplyDataStyleReplace:
                if (refreshing)
                {
                    if (!refReplaceCompObjs)
                        refReplaceCompObjs = [NSMutableArray array];
                    [refReplaceCompObjs addObjectsFromArray:newObjs];
                } else {
                    if (!replaceCompObjs)
                        replaceCompObjs = [NSMutableArray array];
                    [replaceCompObjs addObjectsFromArray:newObjs];
                }
                break;
        }
    }
    
    // Make a list of objects to delete
    void clearContents(NSMutableArray *compObjs)
    {
        if (replaceCompObjs)
            [compObjs addObjectsFromArray:replaceCompObjs];
        if (addCompObjs)
            [compObjs addObjectsFromArray:addCompObjs];
        if (refReplaceCompObjs)
            [compObjs addObjectsFromArray:refReplaceCompObjs];
        if (refAddCompObjs)
            [compObjs addObjectsFromArray:refAddCompObjs];
    }
};
    
/// This is a comparison operator for sorting loaded tile pointers by
/// Quadtree node identifier.
typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const QuadPagingLoadedTile *a,const QuadPagingLoadedTile *b)
    {
        if (a->nodeIdent.level == b->nodeIdent.level)
        {
            if (a->nodeIdent.x == b->nodeIdent.x)
                return a->nodeIdent.y < b->nodeIdent.y;
            return a->nodeIdent.x < b->nodeIdent.x;
        } else
            return a->nodeIdent.level < b->nodeIdent.level;
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
    bool canShortCircuitImportance;
    int maxShortCircuitLevel;
    WhirlyKitViewState *lastViewState;
    WhirlyKitSceneRendererES *_renderer;
}

- (id)initWithCoordSystem:(MaplyCoordinateSystem *)inCoordSys delegate:(NSObject<MaplyPagingDelegate> *)inTileSource
{
    self = [super init];
    
    coordSys = inCoordSys;
    tileSource = inTileSource;
    _numSimultaneousFetches = 8;
    pthread_mutex_init(&tileSetLock, NULL);
    _importance = 512*512;
    _flipY = true;
    _maxTiles = 256;
    canShortCircuitImportance = false;
    maxShortCircuitLevel = -1;
    _useTargetZoomLevel = true;
    _singleLevelLoading = false;
    
    return self;
}

- (void)dealloc
{
    coordSys = nil;
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
    _renderer = renderer;
    
    // Cache min and max zoom.  Tile sources might do a lookup for these
    minZoom = [tileSource minZoom];
    maxZoom = [tileSource maxZoom];
    
    // Set up tile and and quad layer with us as the data source
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:self renderer:renderer];
    // A tile needs to take up this much screen space
    quadLayer.minImportance = _importance;
    quadLayer.maxTiles = _maxTiles;
    
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

- (void)geoBoundsForTileD:(MaplyTileID)tileID ll:(MaplyCoordinateD *)ll ur:(MaplyCoordinateD *)ur
{
    if (!quadLayer || !quadLayer.quadtree || !scene || !scene->getCoordAdapter())
        return;
    
    Point2d mbrLL,mbrUR;
    quadLayer.quadtree->generateMbrForNode(WhirlyKit::Quadtree::Identifier(tileID.x,tileID.y,tileID.level),mbrLL,mbrUR);
    
    CoordSystem *wkCoordSys = quadLayer.coordSys;
    Point2d pts[4];
    pts[0] = wkCoordSys->localToGeographicD(Point3d(mbrLL.x(),mbrLL.y(),0.0));
    pts[1] = wkCoordSys->localToGeographicD(Point3d(mbrUR.x(),mbrLL.y(),0.0));
    pts[2] = wkCoordSys->localToGeographicD(Point3d(mbrUR.x(),mbrUR.y(),0.0));
    pts[3] = wkCoordSys->localToGeographicD(Point3d(mbrLL.x(),mbrUR.y(),0.0));
    Point2d minPt(pts[0].x(),pts[0].y()),  maxPt(pts[0].x(),pts[0].y());
    for (unsigned int ii=1;ii<4;ii++)
    {
        minPt.x() = std::min(minPt.x(),pts[ii].x());
        minPt.y() = std::min(minPt.y(),pts[ii].y());
        maxPt.x() = std::max(maxPt.x(),pts[ii].x());
        maxPt.y() = std::max(maxPt.y(),pts[ii].y());
    }
    
    ll->x = minPt.x();
    ll->y = minPt.y();
    ur->x = maxPt.x();
    ur->y = maxPt.y();
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

- (int)targetZoomLevel
{
    if (!lastViewState || !_renderer || !scene)
        return minZoom;
    
    int zoomLevel = 0;
    WhirlyKit::Point2f center = Point2f(lastViewState.eyePos.x(),lastViewState.eyePos.y());
    // The coordinate adapter might have its own center
    Point3d adaptCenter = scene->getCoordAdapter()->getCenter();
    center.x() += adaptCenter.x();
    center.y() += adaptCenter.y();

    while (zoomLevel <= maxZoom)
    {
        WhirlyKit::Quadtree::Identifier ident;
        ident.x = 0;  ident.y = 0;  ident.level = zoomLevel;
        // Make an MBR right in the middle of where we're looking
        Mbr mbr = quadLayer.quadtree->generateMbrForNode(ident);
        Point2f span = mbr.ur()-mbr.ll();
        mbr.ll() = center - span/2.0;
        mbr.ur() = center + span/2.0;
        float import = ScreenImportance(lastViewState, Point2f(_renderer.framebufferWidth,_renderer.framebufferHeight), lastViewState.eyeVec, 1, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, nil);
        if (import <= quadLayer.minImportance)
        {
            zoomLevel--;
            break;
        }
        zoomLevel++;
    }
    
    return std::min(zoomLevel,maxZoom);
}

/// Called when we get a new view state
/// We need to decide if we can short circuit the screen space calculations
- (void)newViewState:(WhirlyKitViewState *)viewState
{
    lastViewState = viewState;
    
    if (!_useTargetZoomLevel)
    {
        canShortCircuitImportance = false;
        maxShortCircuitLevel = -1;
        return;
    }
    
    CoordSystemDisplayAdapter *coordAdapter = viewState.coordAdapter;
    Point3d center = coordAdapter->getCenter();
    if (center.x() == 0.0 && center.y() == 0.0 && center.z() == 0.0)
    {
        canShortCircuitImportance = true;
        if (!coordAdapter->isFlat())
        {
            canShortCircuitImportance = false;
            return;
        }
        // We happen to store tilt in the view matrix.
        // Note: Fix this.  This won't detect tilt
        //        Eigen::Matrix4d &viewMat = viewState.viewMatrices[0];
        //        if (!viewMat.isIdentity())
        //        {
        //            canShortCircuitImportance = false;
        //            return;
        //        }
        // The tile source coordinate system must be the same as the display's system
        if (!coordSys->coordSystem->isSameAs(coordAdapter->getCoordSystem()))
        {
            canShortCircuitImportance = false;
            return;
        }
        
        // We need to feel our way down to the appropriate level
        maxShortCircuitLevel = [self targetZoomLevel];
        if (_singleLevelLoading)
        {
            std::set<int> targetLevels;
            targetLevels.insert(maxShortCircuitLevel);
            quadLayer.targetLevels = targetLevels;
        }
    } else {
        // Note: Can't short circuit in this case.  Something wrong with the math
        canShortCircuitImportance = false;
    }
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

    double import = 0.0;
    if (canShortCircuitImportance && maxShortCircuitLevel != -1)
    {
        if (TileIsOnScreen(viewState, frameSize, coordSys->coordSystem, scene->getCoordAdapter(), (_singleLevelLoading ? mbr : parentMbr), ident, attrs))
        {
            import = 1.0/(ident.level+10);
            if (ident.level <= maxShortCircuitLevel)
                import += 1.0;
        }
        import *= self.importance;
    } else {
        // This is how much screen real estate we're covering for this tile
        import = ScreenImportance(viewState, frameSize, viewState.eyeVec, 1, [coordSys getCoordSystem], scene->getCoordAdapter(), parentMbr, ident, attrs) / 4;
    }
    
    // Just the importance of this tile.
//    float import = ScreenImportance(viewState, frameSize, viewState->eyeVec, 1, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
    
//    if (import != 0.0)
//        NSLog(@"tile %d: (%d,%d) = %f",ident.level,ident.x,ident.y,import);
    
    return import;
}

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown
{
    super.layerThread = nil;
    quadLayer = nil;
}

#pragma mark - WhirlyKitQuadLoader

- (int)numFrames
{
    return 1;
}

- (int)currentFrame
{
    return -1;
}


- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer
{
    quadLayer = layer;
}

- (bool)isReady
{
    return (numFetches < _numSimultaneousFetches);
}

- (int)localFetches
{
    return 0;
}

- (int)networkFetches
{
    return numFetches;
}

- (void)quadDisplayLayerStartUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)quadDisplayLayerEndUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
}

- (void)askDelegateForLoad:(MaplyTileID)tileID isRefresh:(bool)doRefresh;
{
    //    if (!_flipY)
    //    {
    //        int y = (1<<tileID.level)-tileID.y-1;
    //        tileID.y = y;
    //    }
    
    bool isThere = false;
    bool isLoading = false;
    bool isRefresh = false;
    
    // Look for the existing tile, just in case
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileID);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        QuadPagingLoadedTile *tile = *it;
        isThere = true;
        isRefresh = (*it)->refreshing;
        isLoading = (*it)->isLoading;
        // Note: What if it's loading?
        if (!isRefresh && doRefresh)
        {
            (*it)->refreshing = true;
            tile->isLoading = true;
            tile->didLoad = false;
        }
    }
    pthread_mutex_unlock(&tileSetLock);
    
    // It's already refreshing, so don't bother
    if (isRefresh)
        return;
    
    // Now that's weird, why are we loading the tile twice?
    if (isThere && !doRefresh)
        return;
    numFetches++;
    
    if (!isThere)
    {
        // Okay, let's add it
        QuadPagingLoadedTile *newTile = new QuadPagingLoadedTile(tileID);
        newTile->isLoading = true;
        newTile->didLoad = false;
        newTile->enable = false;
        pthread_mutex_lock(&tileSetLock);
        tileSet.insert(newTile);
        pthread_mutex_unlock(&tileSetLock);
    }
    
    // Now let the delegate know we'd like that tile.
    if (tileID.level >= minZoom)
        [tileSource startFetchForTile:tileID forLayer:self];
    else {
        [self tileDidLoad:tileID];
    }
    
    //    NSLog(@"Loaded Tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
}

// Called on the layer thread
- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer loadTile:(const WhirlyKit::Quadtree::NodeInfo *)tileInfo
{
    MaplyTileID tileID;
    tileID.x = tileInfo->ident.x;
    tileID.y = tileInfo->ident.y;
    tileID.level = tileInfo->ident.level;

    [self askDelegateForLoad:tileID isRefresh:false];
}

// Called on the layer thread
// Clean out the data created for the tile
- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer unloadTile:(const WhirlyKit::Quadtree::NodeInfo *)tileInfo
{
    MaplyTileID tileID;
    tileID.x = tileInfo->ident.x;
    tileID.y = tileInfo->ident.y;
    tileID.level = tileInfo->ident.level;
    
    QuadPagingLoadedTile *tile = NULL;
    NSMutableArray *addCompObjs = [NSMutableArray array],*replaceCompObjs = [NSMutableArray array];
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileID);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        tile = *it;
        if (tile->addCompObjs)
            [addCompObjs addObjectsFromArray:tile->addCompObjs];
        if (tile->refAddCompObjs)
            [addCompObjs addObjectsFromArray:tile->refAddCompObjs];
        if (tile->replaceCompObjs)
            [replaceCompObjs addObjectsFromArray:tile->replaceCompObjs];
        if (tile->refReplaceCompObjs)
            [replaceCompObjs addObjectsFromArray:tile->refReplaceCompObjs];
        tileSet.erase(it);
    }
    pthread_mutex_unlock(&tileSetLock);
    delete tile;

    [_viewC removeObjects:addCompObjs];
    [_viewC removeObjects:replaceCompObjs];
    
    // Check the parent
    if (tileInfo->ident.level >= minZoom && !_singleLevelLoading)
    {
        MaplyTileID parentID;
        parentID.x = tileInfo->ident.x/2;
        parentID.y = tileInfo->ident.y/2;
        parentID.level = tileInfo->ident.level-1;
        [self runTileUpdate];
    }

//    NSLog(@"unLoaded Tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
}

// Called by the delegate.  We'll track the data they added.
- (void)addData:(NSArray *)dataObjects forTile:(MaplyTileID)tileID
{
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileID);
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
    QuadPagingLoadedTile dummyTile(tileID);
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
    [quadLayer loader:self tileDidNotLoad:Quadtree::Identifier(tileID.x,tileID.y,tileID.level) frame:-1];
}

// If it failed, clear out the tile
- (void)tileFailedToLoad:(MaplyTileID)tileID
{
    NSMutableArray *compObjs = [NSMutableArray array];
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileID);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        QuadPagingLoadedTile *theTile = *it;
        tileSet.erase(it);
        theTile->clearContents(compObjs);
        delete theTile;
    }
    numFetches--;
    pthread_mutex_unlock(&tileSetLock);
    
    [_viewC removeObjects:compObjs mode:MaplyThreadCurrent];
    
    // Check the parent
    if (tileID.level > 0 && !_singleLevelLoading)
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
            QuadPagingLoadedTile childTile(childID);
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
                if ([tile->replaceCompObjs count] > 0)
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
                if ([tile->replaceCompObjs count] > 0)
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
            if ([tile->replaceCompObjs count] > 0)
                [toDisable addObjectsFromArray:tile->replaceCompObjs];
        }
    }
}

// Re-evaluate what's visible, starting at the top
- (void)runTileUpdate
{
    NSMutableArray *toEnable = [NSMutableArray array],*toDisable = [NSMutableArray array];
    
    pthread_mutex_lock(&tileSetLock);
    MaplyTileID tileID;
    tileID.x = 0;
    tileID.y = 0;
    tileID.level = 0;
    QuadPagingLoadedTile dummyTile(tileID);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
        [self evaluateTile:*it enable:true toEnable:toEnable toDisable:toDisable];
        
    pthread_mutex_unlock(&tileSetLock);

    if ([toEnable count] > 0)
        [_viewC enableObjects:toEnable mode:MaplyThreadCurrent];
    if ([toDisable count] > 0)
        [_viewC disableObjects:toDisable mode:MaplyThreadCurrent];
}

// Notify the quad paging layer that we loaded, but do it on the layer thread
- (void)loadNotify:(MaplyTileIDObject *)tileIDObj
{
    // Check if we're still active
    if (!coordSys)
        return;
    
    MaplyTileID tileID = tileIDObj.tileID;
    [quadLayer loader:self tileDidLoad:Quadtree::Identifier(tileID.x,tileID.y,tileID.level) frame:-1];
}

- (void)tileDidLoad:(MaplyTileID)tileID
{
    NSMutableArray *remCompObjs = [NSMutableArray array];
    NSMutableArray *addCompObjs = [NSMutableArray array];
    NSMutableArray *replaceCompObjs = [NSMutableArray array];
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileID);
    QuadPagingLoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        QuadPagingLoadedTile *tile = *it;
        tile->isLoading = false;
        tile->didLoad = true;
        if (tile->refreshing)
        {
            if (tile->addCompObjs)
                [remCompObjs addObjectsFromArray:tile->addCompObjs];
            if (tile->replaceCompObjs)
                [remCompObjs addObjectsFromArray:tile->replaceCompObjs];
            [addCompObjs addObjectsFromArray:tile->refAddCompObjs];
            [replaceCompObjs addObjectsFromArray:tile->refReplaceCompObjs];
            tile->addCompObjs = tile->refAddCompObjs;
            tile->replaceCompObjs = tile->refReplaceCompObjs;
            tile->refAddCompObjs = nil;
            tile->refReplaceCompObjs = nil;

            tile->refreshing = false;
        } else {
            [addCompObjs addObjectsFromArray:tile->addCompObjs];
            [replaceCompObjs addObjectsFromArray:tile->replaceCompObjs];
        }
        tile->refreshing = false;
    }
    numFetches--;
    pthread_mutex_unlock(&tileSetLock);
    
    [_viewC startChanges];
    
    // Delete any old objects if we're refreshing
    if ([remCompObjs count] > 0)
        [_viewC removeObjects:remCompObjs mode:MaplyThreadCurrent];

    // These are added immediately and stick around till the tile is deleted
    if ([addCompObjs count] > 0)
        [_viewC enableObjects:addCompObjs mode:MaplyThreadCurrent];
    
    if (_singleLevelLoading)
    {
        if ([replaceCompObjs count] > 0)
            [_viewC enableObjects:replaceCompObjs mode:MaplyThreadCurrent];
    } else
        [self runTileUpdate];
    
    [_viewC endChanges];
    
    [self performSelector:@selector(loadNotify:) onThread:super.layerThread withObject:[MaplyTileIDObject tileWithTileID:tileID] waitUntilDone:NO];
}

- (void)tile:(MaplyTileID)tileID hasNumParts:(int)numParts
{
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileID);
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
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileID);
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
    if (_singleLevelLoading)
        return true;
    
    bool ret = false;
    MaplyTileID tileID;
    tileID.x = tileInfo.ident.x;
    tileID.y = tileInfo.ident.y;
    tileID.level = tileInfo.ident.level;
    
    pthread_mutex_lock(&tileSetLock);
    QuadPagingLoadedTile dummyTile(tileID);
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
        QuadPagingLoadedTile *tile = *it;
        tile->clearContents(compObjs);
        delete *it;
    }
    tileSet.clear();
    pthread_mutex_unlock(&tileSetLock);
    
    [_viewC removeObjects:compObjs mode:MaplyThreadCurrent];
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

    // Brute force our way through
    pthread_mutex_lock(&tileSetLock);
    std::vector<MaplyTileID> tilesToRefresh;
    for (auto it = tileSet.begin();it != tileSet.end(); ++it)
        if ((*it)->nodeIdent.level >= minZoom)
            tilesToRefresh.push_back((*it)->nodeIdent);
    pthread_mutex_unlock(&tileSetLock);
    
    // Ask the delegate to reload each tile
    // Note: Shouldn't hold up the layer thread for this
    for (unsigned int ii=0;ii<tilesToRefresh.size();ii++)
        [self askDelegateForLoad:tilesToRefresh[ii] isRefresh:true];
}


- (NSObject<MaplyPagingDelegate>*)pagingDelegate {
  return tileSource;
}

@end
