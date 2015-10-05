/*
 *  MaplyQuadImageOfflineLayer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/7/13.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MaplyQuadImageOfflineLayer.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyViewControllerLayer_private.h"
#import "QuadDisplayLayer.h"
#import "MaplyActiveObject.h"
#import "MaplyActiveObject_private.h"
#import "WhirlyGlobe.h"
#import "MaplyImageTile_private.h"
#import "MaplyTexture_private.h"

using namespace WhirlyKit;

@implementation MaplyOfflineImage
@end

@interface MaplyQuadImageOfflineLayer() <WhirlyKitQuadDataStructure,WhirlyKitQuadTileImageDataSource,WhirlyKitQuadTileOfflineDelegate>
@end

@implementation MaplyQuadImageOfflineLayer
{
    MaplyBaseViewController * __weak _viewC;
    MaplyCoordinateSystem *coordSys;
    WhirlyKitQuadTileOfflineLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    WhirlyKitViewState *lastViewState;
    WhirlyKitSceneRendererES *renderer;
    Scene *scene;
    bool sourceSupportsMulti;
    bool sourceWantsAsync;
    int minZoom,maxZoom;
    int tileSize;
    bool canDoValidTiles;
    bool canShortCircuitImportance;
    int maxShortCircuitLevel;
    std::vector<int> framePriorities;
}

- (id)initWithCoordSystem:(MaplyCoordinateSystem *)inCoordSys tileSource:(NSObject<MaplyTileSource> *)inTileSource
{
    self = [super init];
    
    coordSys = inCoordSys;
    _tileSource = inTileSource;
    _maxTiles = 256;
    _numSimultaneousFetches = 8;
    _flipY = true;
    _imageDepth = 1;
    _asyncFetching = true;
    _period = 0.0;
    _bbox.ll = MaplyCoordinateMakeWithDegrees(-180, -90);
    _bbox.ur = MaplyCoordinateMakeWithDegrees(+180, +90);
    _on = true;
    _textureSize = CGSizeMake(1024, 1024);
    _autoRes = true;
    _importanceScale = 1.0;
    _singleLevelLoading = false;
    canShortCircuitImportance = false;
    maxShortCircuitLevel = -1;

    // See if we're letting the source do the async calls r what
    sourceWantsAsync = [_tileSource respondsToSelector:@selector(startFetchLayer:tile:)];

    // Check if the source can handle multiple images
    sourceSupportsMulti = [_tileSource respondsToSelector:@selector(imageForTile:numImages:)];

    // Can answer questions about tiles
    canDoValidTiles = [_tileSource respondsToSelector:@selector(validTile:bbox:)];
    
    return self;
}

- (void)setTileSource:(NSObject<MaplyTileSource> *)tileSource
{
    if ([NSThread currentThread] != super.layerThread)
    {
        [self performSelector:@selector(setTileSource:) onThread:super.layerThread withObject:tileSource waitUntilDone:NO];
        return;
    }
    
    _tileSource = tileSource;
    
    [self setupTileLoader];
    [quadLayer refresh];
} 

- (NSArray *)loadedFrames
{
    std::vector<WhirlyKit::FrameLoadStatus> frameStatus;
    [quadLayer getFrameLoadStatus:frameStatus];
    
    NSMutableArray *stats = [NSMutableArray array];
    for (unsigned int ii=0;ii<frameStatus.size();ii++)
    {
        FrameLoadStatus &inStatus = frameStatus[ii];
        MaplyFrameStatus *status = [[MaplyFrameStatus alloc] init];
        status.numTilesLoaded = inStatus.numTilesLoaded;
        status.fullyLoaded = inStatus.complete;
        status.currentFrame = inStatus.currentFrame;
        
        [stats addObject:status];
    }
    
    return stats;
}

- (void)setFrameLoadingPriority:(NSArray *)priorities
{
    if ([NSThread currentThread] != super.layerThread)
    {
        [self performSelector:@selector(setFrameLoadingPriority:) onThread:super.layerThread withObject:priorities waitUntilDone:NO];
        return;
    }

    if ([priorities count] != _imageDepth)
        return;
    framePriorities.resize([priorities count]);
    for (unsigned int ii=0;ii<[priorities count];ii++)
        framePriorities[ii] = [priorities[ii] intValue];
    
    if (quadLayer)
        [quadLayer setFrameLoadingPriorities:framePriorities];
}

- (void)setImportanceScale:(float)importanceScale
{
    _importanceScale = importanceScale;
    [quadLayer poke];
}

- (void)setBbox:(MaplyBoundingBox)bbox
{
    _bbox = bbox;
    Mbr mbr;
    mbr.ll().x() = bbox.ll.x;  mbr.ll().y() = bbox.ll.y;
    mbr.ur().x() = bbox.ur.x;  mbr.ur().y() = bbox.ur.y;
    tileLoader.mbr = mbr;
}

- (void)setOn:(bool)on
{
    _on = on;
    tileLoader.on = _on;
    quadLayer.enable = _on;
}

- (void)setPeriod:(float)period
{
    _period = period;
    tileLoader.period = period;
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

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [inLayerThread removeLayer:quadLayer];
}

#pragma mark - WhirlyKitQuadDataStructure,WhirlyKitQuadTileImageDataSource

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)inRenderer viewC:(MaplyBaseViewController *)viewC
{
    _viewC = viewC;
    super.layerThread = inLayerThread;
    scene = inScene;
    renderer = inRenderer;
    
    minZoom = [_tileSource minZoom];
    maxZoom = [_tileSource maxZoom];
    tileSize = [_tileSource tileSize];

    // Set up tile and and quad layer with us as the data source
    tileLoader = [[WhirlyKitQuadTileOfflineLoader alloc] initWithName:@"Offline" dataSource:self];
    [self setupTileLoader];
    
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:tileLoader renderer:renderer];
    quadLayer.maxTiles = _maxTiles;
    if (!framePriorities.empty())
        [quadLayer setFrameLoadingPriorities:framePriorities];
    
    [super.layerThread addLayer:quadLayer];
    
    return true;
}

- (void)setupTileLoader
{
    tileLoader.outputDelegate = self;
    tileLoader.period = _period;
    Mbr mbr = Mbr(Point2f(_bbox.ll.x,_bbox.ll.y),Point2f(_bbox.ur.x,_bbox.ur.y));
    tileLoader.mbr = mbr;
    tileLoader.outputDelegate = self;
    tileLoader.numImages = _imageDepth;
    tileLoader.sizeX = _textureSize.width;
    tileLoader.sizeY = _textureSize.height;
    tileLoader.autoRes = _autoRes;
    tileLoader.previewLevels = _previewLevels;
}

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
    if (!lastViewState || !renderer || !scene)
        return minZoom;
    
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    int zoomLevel = 0;
    Point3d center3d = CoordSystemConvert3d(coordAdapter->getCoordSystem(),coordSys->coordSystem,coordAdapter->displayToLocal(lastViewState.eyeVecModel));
    Point2f centerLocal(center3d.x(),center3d.y());

    // Bounding box in local coordinate system
    Point3d ll,ur;
    ll = coordSys->coordSystem->geographicToLocal3d(GeoCoord(-M_PI,-M_PI/2.0));
    ur = coordSys->coordSystem->geographicToLocal3d(GeoCoord(M_PI,M_PI/2));
    
    // The coordinate adapter might have its own center
    Point3d adaptCenter = scene->getCoordAdapter()->getCenter();
    centerLocal += Point2f(adaptCenter.x(),adaptCenter.y());
    while (zoomLevel <= maxZoom)
    {
        WhirlyKit::Quadtree::Identifier ident;
        Point2d thisTileSize((ur.x()-ll.x())/(1<<zoomLevel),(ur.y()-ll.y())/(1<<zoomLevel));
        ident.x = (centerLocal.x()-ll.x())/thisTileSize.x();
        ident.y = (centerLocal.y()-ll.y())/thisTileSize.y();
        ident.level = zoomLevel;
        
        // Make an MBR right in the middle of where we're looking
        Mbr mbr = quadLayer.quadtree->generateMbrForNode(ident);
        Point2f span = mbr.ur()-mbr.ll();
        mbr.ll() = centerLocal - span/2.0;
        mbr.ur() = centerLocal + span/2.0;
        float import = ScreenImportance(lastViewState, Point2f(renderer.framebufferWidth,renderer.framebufferHeight), lastViewState.eyeVec, tileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, nil);
        import *= _importanceScale;
        if (import <= quadLayer.minImportance)
        {
            zoomLevel--;
            break;
        }
        zoomLevel++;
    }
    
    return std::min(zoomLevel,maxZoom);
}

/// Called by the quad display layer when the view changes
- (void)newViewState:(WhirlyKitViewState *)viewState
{
    lastViewState = viewState;
    
    if (!_singleLevelLoading)
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
        
        // Normally we would insist the coordinate system be flat, but
        // we make an exception for offline rendering
        
        // We need to feel our way down to the appropriate level
        maxShortCircuitLevel = [self targetZoomLevel];
        if (_singleLevelLoading)
        {
            std::set<int> targetLevels;
            targetLevels.insert(maxShortCircuitLevel);
            for (NSNumber *level in _multiLevelLoads)
            {
                if ([level isKindOfClass:[NSNumber class]])
                {
                    int whichLevel = [level integerValue];
                    if (whichLevel < 0)
                        whichLevel = maxShortCircuitLevel+whichLevel;
                    if (whichLevel >= 0 && whichLevel < maxShortCircuitLevel)
                        targetLevels.insert(whichLevel);
                }
            }
            quadLayer.targetLevels = targetLevels;
        }
    } else {
        // Note: Can't short circuit in this case.  Something wrong with the math
        canShortCircuitImportance = false;
    }
    
//    NSLog(@"Offline renderer:  target level = %d",maxShortCircuitLevel);
}

/// Return an importance value for the given tile
- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *) viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    if (ident.level == 0)
        return MAXFLOAT;
    
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();

    MaplyTileID tileID;
    tileID.level = ident.level;
    tileID.x = ident.x;
    tileID.y = ident.y;

    if (canDoValidTiles)
    {
        MaplyBoundingBox bbox;
        bbox.ll.x = mbr.ll().x();  bbox.ll.y = mbr.ll().y();
        bbox.ur.x = mbr.ur().x();  bbox.ur.y = mbr.ur().y();
        if (![_tileSource validTile:tileID bbox:&bbox])
            return 0.0;
    }
 
    double import = 0.0;
    if (canShortCircuitImportance && maxShortCircuitLevel != -1)
    {
        if (coordAdapter->isFlat())
        {
            if (TileIsOnScreen(viewState, frameSize, coordSys->coordSystem, coordAdapter, mbr, ident, attrs))
            {
                import = 1.0/(ident.level+10);
                if (ident.level <= maxShortCircuitLevel)
                    import += 1.0;
            }
        } else {
            // We need the backfacing checks that ScreenImportance does
            import = ScreenImportance(viewState, frameSize, viewState.eyeVec, tileSize, [coordSys getCoordSystem], coordAdapter, mbr, ident, attrs);
            if (import > 0.0)
            {
                import = 1.0/(ident.level+10);
                if (ident.level <= maxShortCircuitLevel)
                    import += 1.0;
            }
        }
    } else {
        import = ScreenImportance(viewState, frameSize, viewState.eyeVec, tileSize, [coordSys getCoordSystem], coordAdapter, mbr, ident, attrs);
        import *= _importanceScale;
    }
    
//    NSLog(@"Tiles = %d: (%d,%d), import = %f",ident.level,ident.x,ident.y,import);
    
    return import;
}

- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row frame:(int)frame attrs:(NSMutableDictionary *)attrs
{
    MaplyTileID tileID;
    tileID.x = col;  tileID.y = row;  tileID.level = level;
    
    // If this is a lower level than we provide, just do a placeholder
    if (tileID.level < minZoom)
    {
        NSArray *args = @[[WhirlyKitLoadedImage PlaceholderImage],@(tileID.x),@(tileID.y),@(tileID.level),@(frame),_tileSource];
        if (super.layerThread)
        {
            if ([NSThread currentThread] == super.layerThread)
                [self performSelector:@selector(mergeTile:) withObject:args];
            else
                [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
        }
        return;
    }
    
    // If we're not doing OSM style addressing, we need to flip the Y back to TMS
    if (!_flipY)
    {
        int y = (1<<level)-tileID.y-1;
        tileID.y = y;
    }
    
    // The tile source wants to do all the async management
    if (sourceWantsAsync)
    {
        // Tile sources often do work in the startFetch so let's spin that off
        if (_asyncFetching)
        {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                           ^{
                               [_tileSource startFetchLayer:self tile:tileID frame:frame];
                           }
                           );
        } else {
            [_tileSource startFetchLayer:self tile:tileID frame:frame];
        }
        return;
    }

    // This is the fetching block.  We'll invoke it a couple of different ways below.
    void (^workBlock)() =
    ^{
        // Get the data for the tile and sort out what the delegate returned to us
        id tileReturn = [_tileSource imageForTile:tileID frame:frame];
        
        if ([tileReturn isKindOfClass:[NSError class]])
        {
            NSLog(@"OfflineLayer: Failed to load tile %d: (%d,%d) because:\n%@",tileID.level,tileID.x,tileID.y,[((NSError *)tileReturn) description]);
            tileReturn = nil;
        }
        
        MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:tileReturn];
        WhirlyKitLoadedTile *loadTile = [tileData wkTile:0 convertToRaw:false];
        
        NSArray *args = @[(loadTile ? loadTile : [NSNull null]),@(col),@(row),@(level),@(frame),_tileSource];
        if (super.layerThread)
        {
            if ([NSThread currentThread] == super.layerThread)
                [self performSelector:@selector(mergeTile:) withObject:args];
            else
                [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
        }
    };
    
    // For async mode, off we go
    if (_asyncFetching)
    {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                       workBlock);
    } else {
        // In sync mode, we just do the work
        workBlock();
    }
}

- (void)loadedImages:(id)tileReturn forTile:(MaplyTileID)tileID
{
    [self loadedImages:tileReturn forTile:tileID frame:-1];
}


- (void)loadedImages:(id)tileReturn forTile:(MaplyTileID)tileID frame:(int)frame
{
    // Adjust the y back to what the system is expecting
    int y = tileID.y;
    if (!_flipY)
    {
        y = (1<<tileID.level)-tileID.y-1;
    }

    MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:tileReturn];
    WhirlyKitLoadedTile *loadTile = [tileData wkTile:0 convertToRaw:false];
    
    NSArray *args = @[(loadTile ? loadTile : [NSNull null]),@(tileID.x),@(y),@(tileID.level),@(frame),_tileSource];
    if (super.layerThread)
    {
        if ([NSThread currentThread] == super.layerThread)
            [self performSelector:@selector(mergeTile:) withObject:args];
        else
            [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
    }
}

- (void)loadError:(NSError *)error forTile:(MaplyTileID)tileID frame:(int)frame
{
    [self loadedImages:nil forTile:tileID frame:frame];
}

// Merge the tile result back on the layer thread
- (void)mergeTile:(NSArray *)args
{
    if (!super.layerThread)
        return;
    
    WhirlyKitLoadedTile *loadTile = args[0];
    if ([loadTile isKindOfClass:[NSNull class]])
        loadTile = nil;
    int col = [args[1] intValue];
    int row = [args[2] intValue];
    int level = [args[3] intValue];
    int frame = [args[4] intValue];
    id oldTileSource = args[5];
    
    // This might happen if we change tile sources while we're waiting for a network call
    if (_tileSource != oldTileSource)
        return;
    
    [tileLoader dataSource: self loadedImage:loadTile forLevel: level col: col row: row frame:frame];
}

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown
{
    super.layerThread = nil;
}

// Note: Should allow the users to set this
- (int)maxSimultaneousFetches
{
    return _numSimultaneousFetches;
}

#pragma mark - WhirlyKitQuadTileOfflineDelegate

// Here's where we get the generated image back.
// We're not on the main thread, Dorothy.
- (void)loader:(WhirlyKitQuadTileOfflineLoader *)loader image:(WhirlyKitQuadTileOfflineImage *)inImage
{
    if (_delegate && inImage && inImage.texture != EmptyIdentity)
    {
        MaplyBoundingBox bbox;
        Mbr mbr = inImage.mbr;
        bbox.ll.x = mbr.ll().x();  bbox.ll.y = mbr.ll().y();
        bbox.ur.x = mbr.ur().x();  bbox.ur.y = mbr.ur().y();
        MaplyOfflineImage *offlineImage = [[MaplyOfflineImage alloc] init];
        offlineImage.bbox = bbox;
        
        // Convert the textures into MaplyTextures
        // Note: Does the lack of interact layer break things?
        MaplyTexture *maplyTex = [[MaplyTexture alloc] init];
        maplyTex.texID = inImage.texture;
        maplyTex.interactLayer = NULL;
        
        offlineImage.tex = maplyTex;
        offlineImage.centerSize = inImage.centerSize;
        offlineImage.texSize = inImage.texSize;
        offlineImage.frame = inImage.frame;
        for (unsigned int ii=0;ii<4;ii++)
            offlineImage->cornerSizes[ii] = inImage->cornerSizes[ii];
        
        [_delegate offlineLayer:self image:offlineImage];
    }
}


@end
