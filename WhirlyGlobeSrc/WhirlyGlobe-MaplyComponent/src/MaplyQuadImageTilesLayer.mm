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
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

/* An active model is called by the renderer right before
 we render a frame.  This lets us mess with the images
 being displayed by a tile layer in an immediate way.
 */
@interface ActiveImageUpdater : MaplyActiveObject
// Tile loader that's got the images we need
@property (nonatomic,weak) WhirlyKitQuadTileLoader *tileLoader;
// Number of images layers
@property unsigned int numImages;
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

- (void)updateForFrame:(WhirlyKit::RendererFrameInfo *)frameInfo
{
    if (!_tileLoader)
        return;
    
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    float where = fmodf(now-_startTime,_period)/_period * (_numImages);
    unsigned int image0 = floorf(where);
    float t = where-image0;
    unsigned int image1 = ceilf(where);
    if (image1 == _numImages)
        image1 = 0;
    
    // Change the images to give us start and finish
    ChangeSet changes;
    [_tileLoader setCurrentImageStart:image0 end:image1 changes:changes];
    if (!changes.empty())
        scene->addChangeRequests(changes);
    
    // Set the interpolation in the program
    OpenGLES2Program *prog = scene->getProgram(_programId);
    if (prog)
    {
        glUseProgram(prog->getProgram());
        prog->setUniform("u_interp", t);
    }
}

@end


@implementation MaplyQuadImageTilesLayer
{
    MaplyBaseViewController * __weak _viewC;
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    Scene *scene;
    MaplyCoordinateSystem *coordSys;
    NSObject<MaplyTileSource> *tileSource;
    NSArray *tileSources;
    int minZoom,maxZoom;
    int tileSize;
    bool sourceSupportsMulti;
    ActiveImageUpdater *imageUpdater;
    SimpleIdentity _customShader;
    float _minElev,_maxElev;
    NSObject<MaplyElevationSourceDelegate> *elevDelegate;
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
    
    // Check if the source can handle multiple images
    sourceSupportsMulti = [tileSource respondsToSelector:@selector(imagesForTile:numImages:)];
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)viewC
{
    _viewC = viewC;
    super.layerThread = inLayerThread;
    scene = inScene;

    // Cache min and max zoom.  Tile sources might do a lookup for these
    minZoom = [tileSource minZoom];
    maxZoom = [tileSource maxZoom];
    tileSize = [tileSource tileSize];

    // Set up tile and and quad layer with us as the data source
    tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:self];
    tileLoader.ignoreEdgeMatching = !_handleEdges;
    tileLoader.coverPoles = _coverPoles;
    tileLoader.drawPriority = super.drawPriority;
    tileLoader.numImages = _imageDepth;
    tileLoader.includeElev = _includeElevAttrForShader;
    tileLoader.useElevAsZ = (viewC.elevDelegate != nil);
    tileLoader.textureAtlasSize = _texturAtlasSize;
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
    
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:tileLoader renderer:renderer];
    quadLayer.fullLoad = _waitLoad;
    quadLayer.fullLoadTimeout = _waitLoadTimeout;
    quadLayer.maxTiles = _maxTiles;
    
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
        if (!_customShader)
            _customShader = scene->getProgramIDByName(kToolkitDefaultTriangleMultiTex);

        if (_animationPeriod > 0.0)
        {
            imageUpdater = [[ActiveImageUpdater alloc] init];
            imageUpdater.startTime = CFAbsoluteTimeGetCurrent();
            imageUpdater.tileLoader = tileLoader;
            imageUpdater.period = _animationPeriod;
            imageUpdater.startTime = CFAbsoluteTimeGetCurrent();
            imageUpdater.numImages = _imageDepth;
            imageUpdater.programId = _customShader;
            tileLoader.programId = _customShader;
            [viewC addActiveObject:imageUpdater];
        } else
            [self setCurrentImage:_currentImage];
    }
    
    elevDelegate = _viewC.elevDelegate;
    
    [super.layerThread addLayer:quadLayer];

    return true;
}

- (void)setCurrentImage:(float)currentImage
{
    _currentImage = currentImage;
    
    if (imageUpdater)
    {
        [_viewC removeActiveObject:imageUpdater];
        imageUpdater = nil;
    }
    
    if (!scene)
        return;

    unsigned int image0 = floorf(_currentImage);
    float t = _currentImage-image0;
    unsigned int image1 = ceilf(_currentImage);
    if (image1 == _imageDepth)
        image1 = 0;
    
    // Change the images to give us start and finish
    ChangeSet changes;
    [tileLoader setCurrentImageStart:image0 end:image1 changes:changes];
    if (!changes.empty())
        scene->addChangeRequests(changes);
    
    // Set the interpolation in the program
    OpenGLES2Program *prog = scene->getProgram(_customShader);
    if (prog)
    {
        glUseProgram(prog->getProgram());
        prog->setUniform("u_interp", t);
    }
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

- (void)setDrawPriority:(int)drawPriority
{
    super.drawPriority = drawPriority;
    if (tileLoader)
        tileLoader.drawPriority = drawPriority;
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
    [_viewC removeActiveObject:imageUpdater];
    imageUpdater = nil;
    [inLayerThread removeLayer:quadLayer];
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
    if (ident.level == 0)
        return MAXFLOAT;
    
    float import = 0.0;
    if (elevDelegate)
    {
        import = ScreenImportance(viewState, frameSize, tileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, _minElev, _maxElev, ident, attrs);
    } else {
        import = ScreenImportance(viewState, frameSize, viewState.eyeVec, tileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
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
    bool isLocal = [tileSource tileIsLocal:tileID];
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

    // If we're not going OSM style addressing, we need to flip the Y back to TMS
    if (!_flipY)
    {
        int y = (1<<level)-tileID.y-1;
        tileID.y = y;
    }
    
    // This is the fetching block.  We'll invoke it a couple of different ways below.
    void (^workBlock)() =
    ^{
        NSMutableArray *imageDataArr = [NSMutableArray array];

        // Start with elevation
        MaplyElevationChunk *elevChunk = nil;
        if (elevDelegate.minZoom <= tileID.level && tileID.level <= elevDelegate.maxZoom)
        {
            elevChunk = [elevDelegate elevForTile:tileID];
        }
        
        // Needed elevation and failed to load, so stop
        if (elevDelegate && _requireElev && !elevChunk)
        {
            NSArray *args = @[[NSNull null],@(col),@(row),@(level)];
            if (super.layerThread)
            {
                if ([NSThread currentThread] == super.layerThread)
                    [self performSelector:@selector(mergeTile:) withObject:args];
                else
                    [self performSelector:@selector(mergeTile:) onThread:super.layerThread withObject:args waitUntilDone:NO];
            }
            
            return;
        }

        // Fetch the images
        if (sourceSupportsMulti)
            imageDataArr = [NSMutableArray arrayWithArray:[tileSource imagesForTile:tileID numImages:_imageDepth]];
        else {
            NSData *imgData = [tileSource imageForTile:tileID];
            if (imgData)
                [(NSMutableArray *)imageDataArr addObject:imgData];
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
        
        WhirlyKitLoadedTile *loadTile = [[WhirlyKitLoadedTile alloc] init];
        if ([imageDataArr count] == _imageDepth)
        {
            for (unsigned int ii=0;ii<_imageDepth;ii++)
            {
                WhirlyKitLoadedImage *loadImage = nil;
                NSObject *imgData = [imageDataArr objectAtIndex:ii];
                if ([imgData isKindOfClass:[UIImage class]])
                {
                    loadImage = [WhirlyKitLoadedImage LoadedImageWithUIImage:(UIImage *)imgData];
                } else if ([imgData isKindOfClass:[NSData class]])
                {
                    loadImage = [WhirlyKitLoadedImage LoadedImageWithNSDataAsPNGorJPG:(NSData *)imgData];
                }
                if (!loadImage)
                    break;
                // This pulls the pixels out of their weird little compressed formats
                // Since we're on our own thread here (probably) this may save time
                [loadImage convertToRawData];
                [loadTile.images addObject:loadImage];
            }
        } else
            loadTile = nil;
        
        // Let's not forget the elevation
        if ([loadTile isKindOfClass:[WhirlyKitLoadedTile class]] && elevChunk)
        {
            WhirlyKitElevationChunk *wkChunk = [[WhirlyKitElevationChunk alloc] initWithFloatData:elevChunk.data sizeX:elevChunk.numX sizeY:elevChunk.numY];
            loadTile.elevChunk = wkChunk;
        }
            
        NSArray *args = @[(loadTile ? loadTile : [NSNull null]),@(col),@(row),@(level)];
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
    
    [tileLoader dataSource: self loadedImage:loadTile forLevel: level col: col row: row];
}


@end
