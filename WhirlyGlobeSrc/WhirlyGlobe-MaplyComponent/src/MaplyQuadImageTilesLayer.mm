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
// Note: Porting
//#import "MaplyActiveObject.h"
//#import "MaplyActiveObject_private.h"
#import "MaplyBaseViewController_private.h"
#import "WhirlyGlobe.h"
#import "MaplyImageTile_private.h"
#import "MaplyQuadDisplayLayer_private.h"
#import "UIColor+Stuff.h"

using namespace WhirlyKit;

/// We wrap out MaplyImageTile with this C++ object to pass it to the
///  C++ section of the library
class LoadedImageWrapper : public LoadedImage
{
public:
    LoadedImageWrapper(MaplyImageTile *tile) : tile(tile) { }
    
    /// Generate an appropriate texture.
    /// You could overload this, just be sure to respect the border pixels.
    virtual Texture *buildTexture(int borderSize,int width,int height)
    {
        return [tile buildTextureFor:0 border:borderSize destWidth:width destHeight:height];
    }
    
    /// This means there's nothing to display, but the children are valid
    virtual bool isPlaceholder()
    {
        return tile.type == MaplyImgTypePlaceholder;
    }
    
    /// Return image width
    virtual int getWidth()
    {
        return tile.size.width;
    }
    
    /// Return image height
    virtual int getHeight()
    {
        return tile.size.height;
    }

protected:
    
    MaplyImageTile *tile;
};


@interface MaplyQuadImageTilesLayer()
- (WhirlyKit::CoordSystem *)coordSystem;
- (WhirlyKit::Mbr)totalExtents;
- (void)setCurrentImage:(float)currentImage cancelUpdater:(bool)cancelUpdater;
- (int)minZoom;
- (int)maxZoom;
- (double)importanceForTile:(const WhirlyKit::Quadtree::Identifier &)ident mbr:(const WhirlyKit::Mbr &)mbr viewInfo:(WhirlyKit::ViewState *) viewState frameSize:(const WhirlyKit::Point2f &)frameSize attrs:(Dictionary *)attrs;
- (void)shutdown;
- (int)maxSimultaneousFetches;
- (void)startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(Dictionary *)attrs;
- (bool)tileIsLocalLevel:(int)level col:(int)col row:(int)row;
- (void)newViewState:(ViewState *)viewState;
@end

namespace WhirlyKit
{

/* WhirlyKit is a C++ library.  This layer is Objective-C.
    This object acts as an adapter between the C++ in the quad loader
    and the Objective C in our layer.
 */
class MaplyQuadImageDisplayAdapter : public QuadDataStructure, public QuadTileImageDataSource
{
public:
    /** QuadDataStructure Calls **/
    virtual CoordSystem *getCoordSystem()
    {
        return [layer coordSystem];
    }
    
    /// Bounding box used to calculate quad tree nodes.  In local coordinate system.
    virtual Mbr getTotalExtents()
    {
        return [layer totalExtents];
    }
    
    /// Bounding box of data you actually want to display.  In local coordinate system.
    /// Unless you're being clever, make this the same as totalExtents.
    virtual Mbr getValidExtents()
    {
        return getTotalExtents();
    }

    /// Return the minimum quad tree zoom level (usually 0)
    virtual int getMinZoom()
    {
        return [layer minZoom];
    }
    
    /// Return the maximum quad tree zoom level.  Must be at least minZoom
    virtual int getMaxZoom()
    {
        return [layer maxZoom];
    }
    
    /// Return an importance value for the given tile
    virtual double importanceForTile(const Quadtree::Identifier &ident,const Mbr &mbr,ViewState *viewState,const Point2f &frameSize,Dictionary *attrs)
    {
        return [layer importanceForTile:ident mbr:mbr viewInfo:viewState frameSize:frameSize attrs:attrs];
    }
    
    /// Called when the view state changes.  If you're caching info, do it here.
    virtual void newViewState(ViewState *viewState)
    {
        [layer newViewState:viewState];
    }
    
    /// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
    virtual void shutdown()
    {
        [layer shutdown];
    }
    
    virtual int maxSimultaneousFetches()
    {
        return [layer maxSimultaneousFetches];
    }
    
    /// The quad loader is letting us know to start loading the image.
    /// We'll call the loader back with the image when it's ready.
    virtual void startFetch(QuadTileLoaderSupport *quadLoader,int level,int col,int row,Dictionary *attrs)
    {
        [layer startFetchForLevel:level col:col row:row attrs:attrs];
    }
    
    /// Check if the given tile is a local or remote fetch.  This is a hint
    ///  to the pager.  It can display local tiles as a group faster.
    virtual bool tileIsLocal(int level,int col,int row)
    {
        return [layer tileIsLocalLevel:level col:col row:row];
    }

    
    // Objective-C layer we're calling back
    MaplyQuadImageTilesLayer *layer;
};
    
}

// Note: Porting
///* An active model is called by the renderer right before
// we render a frame.  This lets us mess with the images
// being displayed by a tile layer in an immediate way.
// */
//@interface ActiveImageUpdater : MaplyActiveObject
//// Tile loader that's got the images we need
//@property (nonatomic,weak) MaplyQuadImageTilesLayer *tileLayer;
//// The last valid value for currentImage
//@property float maxCurrentImage;
//// The period over which we'll switch them all
//@property float period;
//// Start time, for offset purposes
//@property NSTimeInterval startTime;
//// The program ID, so we can change the interpolation
//@property SimpleIdentity programId;
//@end
//
//@implementation ActiveImageUpdater
//
//- (bool)hasUpdate
//{
//    return true;
//}
//
//- (void)updateForFrame:(WhirlyKit::RendererFrameInfo *)frameInfo
//{
//    if (!_tileLayer)
//        return;
//    
//    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
//    float where = fmodf(now-_startTime,_period)/_period * (_maxCurrentImage-1);
//    
//    [_tileLayer setCurrentImage:where cancelUpdater:false];
//}
//
//@end


@implementation MaplyQuadImageTilesLayer
{
    MaplyBaseViewController * __weak _viewC;
    MaplyQuadImageDisplayAdapter adapter;
    WhirlyKitQuadDisplayLayer *quadLayer;
    QuadTileLoader *tileLoader;
    Scene *scene;
    MaplyCoordinateSystem *coordSys;
    NSObject<MaplyTileSource> *tileSource;
    int minZoom,maxZoom;
    int tileSize;
    bool sourceWantsAsync;
    // Note: Porting
//    ActiveImageUpdater *imageUpdater;
    SimpleIdentity _customShader;
    float _minElev,_maxElev;
    bool canShortCircuitImportance;
    int maxShortCircuitLevel;
    SceneRendererES *_renderer;
    ViewState lastViewState;
    // Note: Porting
//    NSObject<MaplyElevationSourceDelegate> *elevDelegate;
    bool variableSizeTiles;
    bool canDoValidTiles;
}

- (id)initWithCoordSystem:(MaplyCoordinateSystem *)inCoordSys tileSource:(NSObject<MaplyTileSource> *)inTileSource
{
    self = [super init];
    
    coordSys = inCoordSys;
    tileSource = inTileSource;
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
    
    // See if we're letting the source do the async calls r what
    sourceWantsAsync = [tileSource respondsToSelector:@selector(startFetchLayer:tile:)];
    
    // See if the delegate is doing variable sized tiles (kill me)
    variableSizeTiles = [tileSource respondsToSelector:@selector(tileSizeForTile:)];
    
    // Can answer questions about tiles
    canDoValidTiles = [tileSource respondsToSelector:@selector(validTile:bbox:)];
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(SceneRendererES *)renderer viewC:(MaplyBaseViewController *)viewC
{
    _viewC = viewC;
    super.layerThread = inLayerThread;
    scene = inScene;
    _renderer = renderer;

    // Cache min and max zoom.  Tile sources might do a lookup for these
    minZoom = [tileSource minZoom];
    maxZoom = [tileSource maxZoom];
    tileSize = [tileSource tileSize];
    
    // Set up tile and and quad layer with us as the data source
    adapter.layer = self;
    tileLoader = new QuadTileLoader("Image Layer",&adapter);
    [self setupTileLoader];
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:&adapter loader:tileLoader renderer:renderer];
    quadLayer.displayControl->setFullLoad(_waitLoad);
    quadLayer.displayControl->setFullLoadTimeout(_waitLoadTimeout);
    quadLayer.displayControl->setMaxTiles(_maxTiles);
    quadLayer.displayControl->setViewUpdatePeriod(_viewUpdatePeriod);
    quadLayer.displayControl->setMinUpdateDist(_minUpdateDist);
    
    // Look for a custom program
    if (_shaderProgramName)
    {
        _customShader = scene->getProgramIDBySceneName([_shaderProgramName cStringUsingEncoding:NSASCIIStringEncoding]);
        tileLoader->setProgramId(_customShader);
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
    
    // Note: Porting
//    elevDelegate = _viewC.elevDelegate;
    
    [super.layerThread addLayer:quadLayer];

    return true;
}

- (void)dealloc
{
    if (tileLoader)
        delete tileLoader;
    tileLoader = NULL;
}

- (void)setupTileLoader
{
    tileLoader->setIgnoreEdgeMatching(!_handleEdges);
    tileLoader->setCoverPoles(_coverPoles);
    tileLoader->setMinVis(_minVis);
    tileLoader->setMaxVis(_maxVis);
    tileLoader->setDrawPriority(super.drawPriority);
    tileLoader->setNumImages(_imageDepth);
    tileLoader->setIncludeElev(_includeElevAttrForShader);
    // Note: Porting
//    tileLoader->setUseElevAsZ((_viewC.elevDelegate != nil) && _useElevAsZ);
    tileLoader->setTextureAtlasSize(_texturAtlasSize);
    ChangeSet changes;
    tileLoader->setEnable(_enable,changes);
    // Note: Not expecting changes here
    switch (_imageFormat)
    {
        case MaplyImageIntRGBA:
        case MaplyImage4Layer8Bit:
        default:
            tileLoader->setImageType(WKTileIntRGBA);
            break;
        case MaplyImageUShort565:
            tileLoader->setImageType(WKTileUShort565);
            break;
        case MaplyImageUShort4444:
            tileLoader->setImageType(WKTileUShort4444);
            break;
        case MaplyImageUShort5551:
            tileLoader->setImageType(WKTileUShort5551);
            break;
        case MaplyImageUByteRed:
            tileLoader->setImageType(WKTileUByteRed);
            break;
        case MaplyImageUByteGreen:
            tileLoader->setImageType(WKTileUByteGreen);
            break;
        case MaplyImageUByteBlue:
            tileLoader->setImageType(WKTileUByteBlue);
            break;
        case MaplyImageUByteAlpha:
            tileLoader->setImageType(WKTileUByteAlpha);
            break;
        case MaplyImageUByteRGB:
            tileLoader->setImageType(WKTileUByteRGB);
            break;
    }
    if (_color)
        tileLoader->setColor([_color asRGBAColor]);
}

- (void)setAnimationPeriod:(float)animationPeriod
{
    _animationPeriod = animationPeriod;
    
    // Note: Porting
//    if (_viewC)
//    {
//        if (imageUpdater)
//        {
//            if (_animationPeriod > 0.0)
//            {
//                imageUpdater.period = _animationPeriod;
//            } else {
//                [_viewC removeActiveObject:imageUpdater];
//                imageUpdater = nil;
//            }
//        } else {
//            if (_animationPeriod > 0.0)
//            {
//                imageUpdater = [[ActiveImageUpdater alloc] init];
//                imageUpdater.startTime = CFAbsoluteTimeGetCurrent();
//                if (_maxCurrentImage > 1)
//                    imageUpdater.startTime = imageUpdater.startTime-_currentImage/(_maxCurrentImage-1)*_animationPeriod;
//                imageUpdater.tileLayer = self;
//                imageUpdater.period = _animationPeriod;
//                imageUpdater.maxCurrentImage = _maxCurrentImage;
//                imageUpdater.programId = _customShader;
//                tileLoader.programId = _customShader;
//                [_viewC addActiveObject:imageUpdater];
//            }
//        }
//    }
}

- (void)setCurrentImage:(float)currentImage
{
    [self setCurrentImage:currentImage cancelUpdater:YES];
}

- (void)setCurrentImage:(float)currentImage cancelUpdater:(bool)cancelUpdater
{
    _currentImage = currentImage;

    // Note: Porting
//    if (cancelUpdater && imageUpdater)
//    {
//        [_viewC removeActiveObject:imageUpdater];
//        imageUpdater = nil;
//    }
    
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
    tileLoader->setCurrentImageStart(image0,image1,changes);
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
        _renderer->forceDrawNextFrame();

        if (oldContext)
            [EAGLContext setCurrentContext:oldContext];
    }
}

- (void)setEnable:(bool)enable
{
    _enable = enable;
    ChangeSet changes;
    tileLoader->setEnable(_enable,changes);
    if (!changes.empty())
        scene->addChangeRequests(changes);
}

- (void)setHandleEdges:(bool)handleEdges
{
    _handleEdges = handleEdges;
    if (tileLoader)
        tileLoader->setIgnoreEdgeMatching(!_handleEdges);
}

- (void)setCoverPoles:(bool)coverPoles
{
    _coverPoles = coverPoles;
    if (tileLoader)
        tileLoader->setCoverPoles(coverPoles);
}

- (void)setDrawPriority:(int)drawPriority
{
    super.drawPriority = drawPriority;
    if (tileLoader)
        tileLoader->setDrawPriority(drawPriority);
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
    // Note: Porting
//    [_viewC removeActiveObject:imageUpdater];
//    imageUpdater = nil;
    [inLayerThread removeLayer:quadLayer];
}

/// Return the coordinate system we're working in
- (WhirlyKit::CoordSystem *)coordSystem
{
    return [coordSys getCoordSystem];
}

- (int)targetZoomLevel
{
    if (!lastViewState.isValid() || !_renderer || !scene)
        return minZoom;
    
    int zoomLevel = 0;
    WhirlyKit::Point2f center = Point2f(lastViewState.eyePos.x(),lastViewState.eyePos.y());
    while (zoomLevel < maxZoom)
    {
        WhirlyKit::Quadtree::Identifier ident;
        ident.x = 0;  ident.y = 0;  ident.level = zoomLevel;
        // Make an MBR right in the middle of where we're looking
        Mbr mbr = quadLayer.displayControl->getQuadtree()->generateMbrForNode(ident);
        Point2f span = mbr.ur()-mbr.ll();
        mbr.ll() = center - span/2.0;
        mbr.ur() = center + span/2.0;
        Dictionary attrs;
        float import = ScreenImportance(&lastViewState, _renderer->getFramebufferSize(), lastViewState.eyeVec, tileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, &attrs);
        if (import <= quadLayer.displayControl->getMinImportance())
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
- (void)newViewState:(ViewState *)viewState
{
    lastViewState = *viewState;

    if (!_useTargetZoomLevel)
    {
        canShortCircuitImportance = false;
        maxShortCircuitLevel = -1;
        return;
    }
    
    CoordSystemDisplayAdapter *coordAdapter = viewState->coordAdapter;
    Point3d center = coordAdapter->getCenter();
    if (center.x() == 0.0 && center.y() == 0.0 && center.z() == 0.0)
    {
        canShortCircuitImportance = true;
        if (!coordAdapter->isFlat())
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
        // Note: Porting
//        if (_singleLevelLoading)
//            quadLayer.targetLevel = maxShortCircuitLevel;
    } else {
        // Note: Can't short circuit in this case.  Something wrong with the math
        canShortCircuitImportance = false;
    }
    
    // Note: Debugging
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
- (double)importanceForTile:(const WhirlyKit::Quadtree::Identifier &)ident mbr:(const WhirlyKit::Mbr &)mbr viewInfo:(ViewState *) viewState frameSize:(const WhirlyKit::Point2f &)frameSize attrs:(Dictionary *)attrs
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
        if (![tileSource validTile:tileID bbox:&bbox])
            return 0.0;
    }

    int thisTileSize = tileSize;
    if (variableSizeTiles)
    {
        thisTileSize = [tileSource tileSizeForTile:tileID];
    }

    double import = 0.0;
    if (canShortCircuitImportance && maxShortCircuitLevel != -1)
    {
        if (TileIsOnScreen(viewState, frameSize, coordSys->coordSystem, scene->getCoordAdapter(), mbr, ident, attrs))
            import = 1.0/(ident.level+10);
        if (ident.level <= maxShortCircuitLevel)
            import += 1.0;
    } else {
//        if (elevDelegate)
        // Note: Porting
        if (false)
        {
            import = ScreenImportance(viewState, frameSize, thisTileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, _minElev, _maxElev, ident, attrs);
        } else {
            import = ScreenImportance(viewState, frameSize, viewState->eyeVec, thisTileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
        }
    }

//    NSLog(@"Tiles = %d: (%d,%d), import = %f",ident.level,ident.x,ident.y,import);
    
    return import;
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
    bool isLocal = [tileSource respondsToSelector:@selector(tileIsLocal:)];
    if (isLocal)
        isLocal = [tileSource tileIsLocal:tileID];
    // And the elevation delegate, if there is one
    // Note: Porting
//    if (isLocal && elevDelegate)
//    {
//        isLocal = [elevDelegate tileIsLocal:tileID];
//    }
    
    return isLocal;
}

// Turn this on to break a fraction of the images.  This is for internal testing
//#define TRASHTEST 1

/// This version of the load method passes in a mutable dictionary.
/// Store your expensive to generate key/value pairs here.
// Note: Not handling the case where we get a corrupt image and then store it to the cache.
- (void)startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(Dictionary *)attrs
{
    MaplyTileID tileID;
    tileID.x = col;  tileID.y = row;  tileID.level = level;

    // If this is lower level than we're representing, just fake it
    if (tileID.level < minZoom)
    {
        NSArray *args = @[[[MaplyImageTile alloc] initAsPlaceholder],@(tileID.x),@(tileID.y),@(tileID.level)];
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
    int borderTexel = tileLoader->getBorderTexel();
    
    // The tile source wants to do all the async management
    // Well fine.  I'm not offended.  Really.  It's fine.
    if (sourceWantsAsync)
    {
        // Tile sources often do work in the startFetch so let's spin that off
        if (_asyncFetching)
        {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                           ^{                               
                               [tileSource startFetchLayer:self tile:tileID];
                           }
                           );
        } else {
            [tileSource startFetchLayer:self tile:tileID];
        }
        return;
    }
    
    // This is the fetching block.  We'll invoke it a couple of different ways below.
    void (^workBlock)() =
    ^{
        // Note: Porting
        // Start with elevation
//        MaplyElevationChunk *elevChunk = nil;
//        if (elevDelegate.minZoom <= tileID.level && tileID.level <= elevDelegate.maxZoom)
//        {
//            elevChunk = [elevDelegate elevForTile:tileID];
//        }
//        
//        // Needed elevation and failed to load, so stop
//        if (elevDelegate && _requireElev && !elevChunk)
//        {
//            NSArray *args = @[[NSNull null],@(col),@(row),@(level)];
//            if (super.layerThread)
//            {
//                if ([NSThread currentThread] == super.layerThread)
//                    [self performSelector:@selector(mergeTile:) withObject:args];
//                else
//                    [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
//            }
//            
//            return;
//        }

        // Get the data for the tile and sort out what the delegate returned to us
        id tileReturn = [tileSource imageForTile:tileID];
        MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:tileReturn];
        [tileData convertToRaw:borderTexel destWidth:-1 destHeight:-1];
        
        if (!tileData)
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
        
        // Note: Porting
        // Let's not forget the elevation
//        if (loadTile && tileData.type != MaplyImgTypePlaceholder && elevChunk)
//        {
//            WhirlyKitElevationChunk *wkChunk = nil;
//            if ([elevChunk.data length] == sizeof(unsigned short)*elevChunk.numX*elevChunk.numY)
//            {
//                wkChunk = [[WhirlyKitElevationChunk alloc] initWithShortData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
//            } else if ([elevChunk.data length] == sizeof(float)*elevChunk.numX*elevChunk.numY)
//            {
//                wkChunk = [[WhirlyKitElevationChunk alloc] initWithFloatData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
//            }
//            loadTile.elevChunk = wkChunk;
//        }
        
        NSArray *args = @[(tileData ? tileData : [NSNull null]),@(col),@(row),@(level)];
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
    int borderTexel = tileLoader->getBorderTexel();

    // Get the data for the tile and sort out what the delegate returned to us
    MaplyImageTile *tileData = [[MaplyImageTile alloc] initWithRandomData:tileReturn];
    [tileData convertToRaw:borderTexel destWidth:-1 destHeight:-1];

    // Note: Porting
    // Start with elevation
//    MaplyElevationChunk *elevChunk = nil;
//    if (tileData && elevDelegate)
//    {
//        if (elevDelegate.minZoom <= tileID.level && tileID.level <= elevDelegate.maxZoom)
//        {
//            elevChunk = [elevDelegate elevForTile:tileID];
//        }
//        
//        // Needed elevation and failed to load, so stop
//        if (elevDelegate && _requireElev && !elevChunk)
//        {
//            NSArray *args = @[[NSNull null],@(tileID.x),@(tileID.y),@(tileID.level)];
//            if (super.layerThread)
//            {
//                if ([NSThread currentThread] == super.layerThread)
//                    [self performSelector:@selector(mergeTile:) withObject:args];
//                else
//                    [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
//            }
//            
//            return;
//        }
//    }

    // Note: Porting
    // Let's not forget the elevation
//    if (loadTile && tileData.type != MaplyImgTypePlaceholder && elevChunk)
//    {
//        WhirlyKitElevationChunk *wkChunk = nil;
//        if ([elevChunk.data length] == sizeof(unsigned short)*elevChunk.numX*elevChunk.numY)
//        {
//            wkChunk = [[WhirlyKitElevationChunk alloc] initWithShortData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
//        } else if ([elevChunk.data length] == sizeof(float)*elevChunk.numX*elevChunk.numY)
//        {
//            wkChunk = [[WhirlyKitElevationChunk alloc] initWithFloatData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
//        }
//        loadTile.elevChunk = wkChunk;
//    }
    
    NSArray *args = @[(tileData ? tileData : [NSNull null]),@(tileID.x),@(tileID.y),@(tileID.level)];
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
    
    MaplyImageTile *loadTile = args[0];
    if ([loadTile isKindOfClass:[NSNull class]])
        loadTile = nil;
    int col = [args[1] intValue];
    int row = [args[2] intValue];
    int level = [args[3] intValue];

    LoadedImageWrapper tileWrapper(loadTile);
    ChangeSet changes;
    tileLoader->loadedImage(&adapter, &tileWrapper, level, col, row, changes);
    [super.layerThread addChangeRequests:changes];
}


@end
