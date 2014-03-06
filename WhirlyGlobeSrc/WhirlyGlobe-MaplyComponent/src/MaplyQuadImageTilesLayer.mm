/*
 *  MaplyQuadImageTilesLayer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
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

#import "MaplyQuadImageTilesLayer_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "QuadDisplayLayer.h"
#import "MaplyActiveObject.h"
#import "MaplyActiveObject_private.h"
#import "MaplyBaseViewController_private.h"
#import "WhirlyGlobe.h"
#import "MaplyImageTile_private.h"

using namespace WhirlyKit;

@interface MaplyQuadImageTilesLayer()
- (void)setCurrentImage:(float)currentImage cancelUpdater:(bool)cancelUpdater;
@end

/* An active model is called by the renderer right before
 we render a frame.  This lets us mess with the images
 being displayed by a tile layer in an immediate way.
 */
@interface ActiveImageUpdater : MaplyActiveObject
// Tile loader that's got the images we need
@property (nonatomic,weak) MaplyQuadImageTilesLayer *tileLayer;
// The last valid value for currentImage
@property float maxCurrentImage;
// The period over which we'll switch them all
@property float period;
// Start time, for offset purposes
@property NSTimeInterval startTime;
// The program ID, so we can change the interpolation
@property SimpleIdentity programId;
@end

@implementation ActiveImageUpdater

- (bool)hasUpdate
{
    return true;
}

- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo
{
    if (!_tileLayer)
        return;
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    float where = fmodf(now-_startTime,_period)/_period * (_maxCurrentImage-1);
    
    [_tileLayer setCurrentImage:where cancelUpdater:false];
}

@end


@implementation MaplyQuadImageTilesLayer
{
    MaplyBaseViewController * __weak _viewC;
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    Scene *scene;
    MaplyCoordinateSystem *coordSys;
    NSObject<MaplyTileSource> *_tileSource;
    int minZoom,maxZoom;
    int tileSize;
    bool sourceWantsAsync;
    ActiveImageUpdater *imageUpdater;
    SimpleIdentity _customShader;
    float _minElev,_maxElev;
    bool canShortCircuitImportance;
    int maxShortCircuitLevel;
    WhirlyKitSceneRendererES *_renderer;
    WhirlyKitViewState *lastViewState;
    NSObject<MaplyElevationSourceDelegate> *elevDelegate;
    bool variableSizeTiles;
    bool canDoValidTiles;
}

- (id)initWithCoordSystem:(MaplyCoordinateSystem *)inCoordSys tileSource:(NSObject<MaplyTileSource> *)inTileSource
{
    self = [super init];
    
    coordSys = inCoordSys;
    _tileSource = inTileSource;
    _coverPoles = true;
    _numSimultaneousFetches = 8;
    _imageDepth = 1;
    _currentImage = 0;
    _animationPeriod = 0;
    _asyncFetching = true;
    _minElev = -100.0;
    _maxElev = 8900;
    _texturAtlasSize = 2048;
    _imageFormat = MaplyImageIntRGBA;
    _flipY = true;
    _waitLoad = false;
    _waitLoadTimeout = 4.0;
    _maxTiles = 128;
    _minVis = DrawVisibleInvalid;
    _maxVis = DrawVisibleInvalid;
    canShortCircuitImportance = false;
    maxShortCircuitLevel = -1;
    _useTargetZoomLevel = true;
    _viewUpdatePeriod = 0.1;
    _enable = true;
    _animationWrap = true;
    _maxCurrentImage = -1;
    _useElevAsZ = true;
    _importanceScale = 1.0;
    
    // See if we're letting the source do the async calls r what
    sourceWantsAsync = [_tileSource respondsToSelector:@selector(startFetchLayer:tile:)];
    
    // See if the delegate is doing variable sized tiles (kill me)
    variableSizeTiles = [_tileSource respondsToSelector:@selector(tileSizeForTile:)];
    
    // Can answer questions about tiles
    canDoValidTiles = [_tileSource respondsToSelector:@selector(validTile:bbox:)];
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)viewC
{
    _viewC = viewC;
    super.layerThread = inLayerThread;
    scene = inScene;
    _renderer = renderer;

    // Cache min and max zoom.  Tile sources might do a lookup for these
    minZoom = [_tileSource minZoom];
    maxZoom = [_tileSource maxZoom];
    tileSize = [_tileSource tileSize];
    
    // Set up tile and and quad layer with us as the data source
    tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:self];
    [self setupTileLoader];
    
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:tileLoader renderer:renderer];
    quadLayer.fullLoad = _waitLoad;
    quadLayer.fullLoadTimeout = _waitLoadTimeout;
    quadLayer.maxTiles = _maxTiles;
    quadLayer.viewUpdatePeriod = _viewUpdatePeriod;
    quadLayer.minUpdateDist = _minUpdateDist;
    
    // Look for a custom program
    if (_shaderProgramName)
    {
        _customShader = scene->getProgramIDBySceneName([_shaderProgramName cStringUsingEncoding:NSASCIIStringEncoding]);
        tileLoader.programId = _customShader;
    } else
        _customShader = EmptyIdentity;
    
    // If we're switching images, we'll need the right program
    //  and an active object to do the updates
    if (_imageDepth > 1)
    {
        if (_animationWrap && _maxCurrentImage == -1)
            _maxCurrentImage = _imageDepth;
        
        if (!_customShader)
            _customShader = scene->getProgramIDByName(kToolkitDefaultTriangleMultiTex);

        if (_animationPeriod > 0.0)
        {
            self.animationPeriod = _animationPeriod;
        } else
            [self setCurrentImage:_currentImage];
    }
    
    elevDelegate = _viewC.elevDelegate;
    
    [super.layerThread addLayer:quadLayer];

    return true;
}

- (void)setShaderProgramName:(NSString *)shaderProgramName
{
    if ([NSThread currentThread] != super.layerThread && super.layerThread)
    {
        [self performSelector:@selector(setShaderProgramName:) onThread:super.layerThread withObject:shaderProgramName waitUntilDone:NO];
        return;
    }

    _shaderProgramName = shaderProgramName;
    if (scene)
    {
        _customShader = scene->getProgramIDBySceneName([_shaderProgramName cStringUsingEncoding:NSASCIIStringEncoding]);

        [self setupTileLoader];
        tileLoader.programId = _customShader;
        [quadLayer reset];
    }
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
    [quadLayer reset];
}

- (void)setupTileLoader
{
    tileLoader.ignoreEdgeMatching = !_handleEdges;
    tileLoader.coverPoles = _coverPoles;
    tileLoader.minVis = _minVis;
    tileLoader.maxVis = _maxVis;
    tileLoader.drawPriority = super.drawPriority;
    tileLoader.numImages = _imageDepth;
    tileLoader.includeElev = _includeElevAttrForShader;
    tileLoader.useElevAsZ = (_viewC.elevDelegate != nil) && _useElevAsZ;
    tileLoader.textureAtlasSize = _texturAtlasSize;
    tileLoader.enable = _enable;
    switch (_imageFormat)
    {
        case MaplyImageIntRGBA:
        case MaplyImage4Layer8Bit:
        default:
            tileLoader.imageType = WKTileIntRGBA;
            break;
        case MaplyImageUShort565:
            tileLoader.imageType = WKTileUShort565;
            break;
        case MaplyImageUShort4444:
            tileLoader.imageType = WKTileUShort4444;
            break;
        case MaplyImageUShort5551:
            tileLoader.imageType = WKTileUShort5551;
            break;
        case MaplyImageUByteRed:
            tileLoader.imageType = WKTileUByteRed;
            break;
        case MaplyImageUByteGreen:
            tileLoader.imageType = WKTileUByteGreen;
            break;
        case MaplyImageUByteBlue:
            tileLoader.imageType = WKTileUByteBlue;
            break;
        case MaplyImageUByteAlpha:
            tileLoader.imageType = WKTileUByteAlpha;
            break;
        case MaplyImageUByteRGB:
            tileLoader.imageType = WKTileUByteRGB;
            break;
    }
    if (_color)
        tileLoader.color = [_color asRGBAColor];
}

- (void)setDrawPriority:(int)drawPriority
{
    super.drawPriority = drawPriority;
    if (tileLoader)
        tileLoader.drawPriority = drawPriority;
}

- (void)setColor:(UIColor *)color
{
    _color = color;
    if (tileLoader)
        tileLoader.color = [color asRGBAColor];
}

- (void)reset
{
    if (quadLayer)
        [quadLayer reset];
}

- (void)setAnimationPeriod:(float)animationPeriod
{
    _animationPeriod = animationPeriod;
    
    if (_viewC)
    {
        if (imageUpdater)
        {
            if (_animationPeriod > 0.0)
            {
                imageUpdater.period = _animationPeriod;
            } else {
                [_viewC removeActiveObject:imageUpdater];
                imageUpdater = nil;
            }
        } else {
            if (_animationPeriod > 0.0)
            {
                imageUpdater = [[ActiveImageUpdater alloc] init];
                imageUpdater.startTime = CFAbsoluteTimeGetCurrent();
                if (_maxCurrentImage > 1)
                    imageUpdater.startTime = imageUpdater.startTime-_currentImage/(_maxCurrentImage-1)*_animationPeriod;
                imageUpdater.tileLayer = self;
                imageUpdater.period = _animationPeriod;
                imageUpdater.maxCurrentImage = _maxCurrentImage;
                imageUpdater.programId = _customShader;
                tileLoader.programId = _customShader;
                [_viewC addActiveObject:imageUpdater];
            }
        }
    }
}

- (void)setCurrentImage:(float)currentImage
{
    [self setCurrentImage:currentImage cancelUpdater:YES];
}

- (void)setCurrentImage:(float)currentImage cancelUpdater:(bool)cancelUpdater
{
    _currentImage = currentImage;
    
    if (cancelUpdater && imageUpdater)
    {
        [_viewC removeActiveObject:imageUpdater];
        imageUpdater = nil;
    }
    
    if (!scene)
        return;

    unsigned int image0 = floorf(_currentImage);
    unsigned int image1 = ceilf(_currentImage);
    if (_animationWrap)
    {
        if (image1 == _imageDepth)
            image1 = 0;
    }
    if (image0 >= _imageDepth)
        image0 = _imageDepth-1;
    if (image1 >= _imageDepth)
        image1 = -1;
    float t = _currentImage-image0;
    
//    NSLog(@"currentImage = %d->%d -> %f",image0,image1,t);
    
    // Change the images to give us start and finish
    ChangeSet changes;
    [tileLoader setCurrentImageStart:image0 end:image1 changes:changes];
    if (!changes.empty())
        scene->addChangeRequests(changes);
    
    // Set the interpolation in the program
    OpenGLES2Program *prog = scene->getProgram(_customShader);
    if (prog)
    {
        EAGLContext *oldContext = [EAGLContext currentContext];
        [_viewC useGLContext];

        glUseProgram(prog->getProgram());
        prog->setUniform("u_interp", t);
        [_renderer forceDrawNextFrame];

        if (oldContext)
            [EAGLContext setCurrentContext:oldContext];
    }
}

- (void)setEnable:(bool)enable
{
    _enable = enable;
    tileLoader.enable = _enable;
}

- (void)setHandleEdges:(bool)handleEdges
{
    _handleEdges = handleEdges;
    if (tileLoader)
        tileLoader.ignoreEdgeMatching = !_handleEdges;
}

- (void)setCoverPoles:(bool)coverPoles
{
    _coverPoles = coverPoles;
    if (tileLoader)
        tileLoader.coverPoles = coverPoles;
}

- (void)reload
{
    if ([NSThread currentThread] != super.layerThread)
    {
        [self performSelector:@selector(reload) onThread:super.layerThread withObject:nil waitUntilDone:NO];
        return;
    }

    [self setupTileLoader];
    [quadLayer refresh];
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [_viewC removeActiveObject:imageUpdater];
    imageUpdater = nil;
    [inLayerThread removeLayer:quadLayer];
}

- (void)setImportanceScale:(float)importanceScale
{
    _importanceScale = importanceScale;
    // Have the layer re-evaluate its tiles
    [quadLayer poke];
}

/// Return the coordinate system we're working in
- (WhirlyKit::CoordSystem *)coordSystem
{
    return [coordSys getCoordSystem];
}

- (int)targetZoomLevel
{
    if (!lastViewState || !_renderer || !scene)
        return minZoom;
    
    int zoomLevel = 0;
    WhirlyKit::Point2f center = Point2f(lastViewState.eyePos.x(),lastViewState.eyePos.y());
    while (zoomLevel < maxZoom)
    {
        WhirlyKit::Quadtree::Identifier ident;
        ident.x = 0;  ident.y = 0;  ident.level = zoomLevel;
        // Make an MBR right in the middle of where we're looking
        Mbr mbr = quadLayer.quadtree->generateMbrForNode(ident);
        Point2f span = mbr.ur()-mbr.ll();
        mbr.ll() = center - span/2.0;
        mbr.ur() = center + span/2.0;
        float import = ScreenImportance(lastViewState, Point2f(_renderer.framebufferWidth,_renderer.framebufferHeight), lastViewState.eyeVec, tileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, nil);
        if (import <= quadLayer.minImportance)
        {
            zoomLevel--;
            break;
        }
        zoomLevel++;
    }
    
    return zoomLevel;
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
        Eigen::Matrix4d &viewMat = viewState.viewMatrix;
        if (!viewMat.isIdentity())
        {
            canShortCircuitImportance = false;
            return;
        }
        // The tile source coordinate system must be the same as the display's system
        if (!coordSys->coordSystem->isSameAs(coordAdapter->getCoordSystem()))
        {
            canShortCircuitImportance = false;
            return;
        }
        
        // We need to feel our way down to the appropriate level
        maxShortCircuitLevel = [self targetZoomLevel];
    } else {
        // Note: Can't short circuit in this case.  Something wrong with the math
        canShortCircuitImportance = false;
    }
    
    canShortCircuitImportance = false;
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
    if (ident.level == 0)
        return MAXFLOAT;
    
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

    int thisTileSize = tileSize;
    if (variableSizeTiles)
    {
        thisTileSize = [_tileSource tileSizeForTile:tileID];
    }

    double import = 0.0;
    if (canShortCircuitImportance && maxShortCircuitLevel != -1)
    {
        if (TileIsOnScreen(viewState, frameSize, coordSys->coordSystem, scene->getCoordAdapter(), mbr, ident, attrs))
            import = 1.0/(ident.level+10);
        if (ident.level <= maxShortCircuitLevel)
            import += 1.0;
    } else {
        if (elevDelegate)
        {
            import = ScreenImportance(viewState, frameSize, thisTileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, _minElev, _maxElev, ident, attrs);
        } else {
            import = ScreenImportance(viewState, frameSize, viewState.eyeVec, thisTileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
        }
    }

//    NSLog(@"Tiles = %d: (%d,%d), import = %f",ident.level,ident.x,ident.y,import);
    
    return import * _importanceScale;
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

- (bool)tileIsLocalLevel:(int)level col:(int)col row:(int)row
{
    MaplyTileID tileID;
    tileID.x = col;  tileID.y = row;  tileID.level = level;
    
    // If we're not going OSM style addressing, we need to flip the Y back to TMS
    if (!_flipY)
    {
        int y = (1<<level)-tileID.y-1;
        tileID.y = y;
    }

    // Check with the tile source
    bool isLocal = [_tileSource respondsToSelector:@selector(tileIsLocal:)];
    if (isLocal)
        isLocal = [_tileSource tileIsLocal:tileID];
    // And the elevation delegate, if there is one
    if (isLocal && elevDelegate)
    {
        isLocal = [elevDelegate tileIsLocal:tileID];
    }
    
    return isLocal;
}

// Turn this on to break a fraction of the images.  This is for internal testing
//#define TRASHTEST 1

/// This version of the load method passes in a mutable dictionary.
/// Store your expensive to generate key/value pairs here.
// Note: Not handling the case where we get a corrupt image and then store it to the cache.
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(NSMutableDictionary *)attrs
{
    MaplyTileID tileID;
    tileID.x = col;  tileID.y = row;  tileID.level = level;

    // If this is lower level than we're representing, just fake it
    if (tileID.level < minZoom)
    {
        NSArray *args = @[[WhirlyKitLoadedImage PlaceholderImage],@(tileID.x),@(tileID.y),@(tileID.level),_tileSource];
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
    int borderTexel = quadLoader.borderTexel;
    
    // The tile source wants to do all the async management
    if (sourceWantsAsync)
    {
        // Tile sources often do work in the startFetch so let's spin that off
        if (_asyncFetching)
        {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                           ^{                               
                               [_tileSource startFetchLayer:self tile:tileID];
                           }
                           );
        } else {
            [_tileSource startFetchLayer:self tile:tileID];
        }
        return;
    }
    
    // This is the fetching block.  We'll invoke it a couple of different ways below.
    void (^workBlock)() =
    ^{
        // Start with elevation
        MaplyElevationChunk *elevChunk = nil;
        if (elevDelegate.minZoom <= tileID.level && tileID.level <= elevDelegate.maxZoom)
        {
            elevChunk = [elevDelegate elevForTile:tileID];
        }
        
        // Needed elevation and failed to load, so stop
        if (elevDelegate && _requireElev && !elevChunk)
        {
            NSArray *args = @[[NSNull null],@(col),@(row),@(level),_tileSource];
            if (super.layerThread)
            {
                if ([NSThread currentThread] == super.layerThread)
                    [self performSelector:@selector(mergeTile:) withObject:args];
                else
                    [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
            }
            
            return;
        }

        // Get the data for the tile and sort out what the delegate returned to us
        id tileReturn = [_tileSource imageForTile:tileID];
        MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:tileReturn];
        WhirlyKitLoadedTile *loadTile = [tileData wkTile:borderTexel convertToRaw:true];
        
        if (tileData && !loadTile)
        {
            NSLog(@"Bad image data for tile: %d: (%d,%d)",level,col,row);
        }

#ifdef TRASHTEST
        // Mess with some of the images to test corruption
        if (tileID.level > 1)
        {
            for (unsigned int ii=0;ii<_imageDepth;ii++)
            {
                NSObject *imgData = [imageDataArr objectAtIndex:ii];
                // Every so often let's return garbage
                if (imgData && (int)(drand48()*5) == 4)
                {
                    unsigned char *trash = (unsigned char *) malloc(2048);
                    for (unsigned int ii=0;ii<2048;ii++)
                        trash[ii] = drand48()*255;
                    imgData = [[NSData alloc] initWithBytesNoCopy:trash length:2048 freeWhenDone:YES];
                }
                [imageDataArr replaceObjectAtIndex:ii withObject:imgData];
            }
        }
#endif
        
        // Let's not forget the elevation
        if (loadTile && tileData.type != MaplyImgTypePlaceholder && elevChunk)
        {
            WhirlyKitElevationChunk *wkChunk = nil;
            if ([elevChunk.data length] == sizeof(unsigned short)*elevChunk.numX*elevChunk.numY)
            {
                wkChunk = [[WhirlyKitElevationChunk alloc] initWithShortData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
            } else if ([elevChunk.data length] == sizeof(float)*elevChunk.numX*elevChunk.numY)
            {
                wkChunk = [[WhirlyKitElevationChunk alloc] initWithFloatData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
            }
            loadTile.elevChunk = wkChunk;
        }
            
        NSArray *args = @[(loadTile ? loadTile : [NSNull null]),@(col),@(row),@(level),_tileSource];
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
    int borderTexel = tileLoader.borderTexel;

    // Adjust the y back to what the system is expecting
    int y = tileID.y;
    if (!_flipY)
    {
        y = (1<<tileID.level)-tileID.y-1;
    }

    // Get the data for the tile and sort out what the delegate returned to us
    MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:tileReturn];
    WhirlyKitLoadedTile *loadTile = [tileData wkTile:borderTexel convertToRaw:true];

    // Start with elevation
    MaplyElevationChunk *elevChunk = nil;
    if (tileData && elevDelegate)
    {
        if (elevDelegate.minZoom <= tileID.level && tileID.level <= elevDelegate.maxZoom)
        {
            elevChunk = [elevDelegate elevForTile:tileID];
        }
        
        // Needed elevation and failed to load, so stop
        if (elevDelegate && _requireElev && !elevChunk)
        {
            NSArray *args = @[[NSNull null],@(tileID.x),@(y),@(tileID.level),_tileSource];
            if (super.layerThread)
            {
                if ([NSThread currentThread] == super.layerThread)
                    [self performSelector:@selector(mergeTile:) withObject:args];
                else
                    [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
            }
            
            return;
        }
    }
    
    // Let's not forget the elevation
    if (loadTile && tileData.type != MaplyImgTypePlaceholder && elevChunk)
    {
        WhirlyKitElevationChunk *wkChunk = nil;
        if ([elevChunk.data length] == sizeof(unsigned short)*elevChunk.numX*elevChunk.numY)
        {
            wkChunk = [[WhirlyKitElevationChunk alloc] initWithShortData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
        } else if ([elevChunk.data length] == sizeof(float)*elevChunk.numX*elevChunk.numY)
        {
            wkChunk = [[WhirlyKitElevationChunk alloc] initWithFloatData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
        }
        loadTile.elevChunk = wkChunk;
    }
    
    NSArray *args = @[(loadTile ? loadTile : [NSNull null]),@(tileID.x),@(y),@(tileID.level),_tileSource];
    if (super.layerThread)
    {
        if ([NSThread currentThread] == super.layerThread)
            [self performSelector:@selector(mergeTile:) withObject:args];
        else
            [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
    }
}

- (void)loadError:(NSError *)error forTile:(MaplyTileID)tileID
{
    // Adjust the y back to what the system is expecting
    int y = tileID.y;
    if (!_flipY)
    {
        y = (1<<tileID.level)-tileID.y-1;
    }

    NSArray *args = @[([NSNull null]),@(tileID.x),@(y),@(tileID.level),_tileSource];
    if (super.layerThread)
    {
        if ([NSThread currentThread] == super.layerThread)
            [self performSelector:@selector(mergeTile:) withObject:args];
        else
            [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
    }
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
    id oldTileSource = args[4];
    
    // This might happen if we change tile sources while we're waiting for a network call
    if (_tileSource != oldTileSource)
        return;
    
    [tileLoader dataSource: self loadedImage:loadTile forLevel: level col: col row: row];
}


@end
