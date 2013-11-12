/*
 *  TileQuadLoader.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/27/12.
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

#import "GlobeMath.h"
#import "GlobeLayerViewWatcher.h"
#import "UIImage+Stuff.h"
#import <boost/math/special_functions/fpclassify.hpp>
#import "TileQuadLoader.h"
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"

using namespace Eigen;
using namespace WhirlyKit;

@interface WhirlyKitQuadTileLoader()
{
@public
    bool doingUpdate;
    TileBuilder *tileBuilder;
    int defaultTessX,defaultTessY;
    bool _enable;
}

- (LoadedTile *)getTile:(Quadtree::Identifier)ident;
- (void)flushUpdates:(WhirlyKitLayerThread *)layerThread;
@end

@implementation WhirlyKitQuadTileLoader
{
    pthread_mutex_t tileLock;
    /// Tiles we currently have loaded in the scene
    WhirlyKit::LoadedTileSet tileSet;
    
    /// Delegate used to provide images
    NSObject<WhirlyKitQuadTileImageDataSource> * __weak dataSource;
    
    // Parents to update after changes
    std::set<WhirlyKit::Quadtree::Identifier> parents;
    
    /// Change requests queued up between a begin and end
    std::vector<WhirlyKit::ChangeRequest *> changeRequests;
    
    // Keep track of the slow (e.g. network) fetches
    std::set<WhirlyKit::Quadtree::Identifier> networkFetches,localFetches;
    
    // The images we're currently displaying, when we have more than one
    int currentImage0,currentImage1;
    
    NSString *name;
}

- (id)initWithDataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)inDataSource;
{
    self = [super init];
    if (self)
    {
        dataSource = inDataSource;
        _drawOffset = 0;
        _drawPriority = 0;
        _color = RGBAColor(255,255,255,255);
        _hasAlpha = false;
        _ignoreEdgeMatching = false;
        _minVis = DrawVisibleInvalid;
        _maxVis = DrawVisibleInvalid;
        _minPageVis = DrawVisibleInvalid;
        _maxPageVis = DrawVisibleInvalid;
        _imageType = WKTileIntRGBA;
        _useDynamicAtlas = true;
        _numImages = 1;
        currentImage0 = 0;
        currentImage1 = 0;
        doingUpdate = false;
        _includeElev = false;
        _useElevAsZ = true;
        _tileScale = WKTileScaleNone;
        _fixedTileSize = 256;
        _textureAtlasSize = 2048;
        _activeTextures = -1;
        _borderTexel = 1;
        _enable = true;
        defaultTessX = defaultTessY = 10;
        pthread_mutex_init(&tileLock, NULL);
    }
    
    return self;
}

- (id)initWithName:(NSString *)inName dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)inDataSource
{
    self = [self initWithDataSource:inDataSource];
    name = inName;
    
    return self;
}


- (void)clear
{
    pthread_mutex_lock(&tileLock);
    for (LoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
        delete *it;
    pthread_mutex_unlock(&tileLock);
    tileSet.clear();
    pthread_mutex_destroy(&tileLock);
    
    if (tileBuilder)
        delete tileBuilder;
    tileBuilder = NULL;
    
    parents.clear();
}

- (void)dealloc
{
    [self clear];
}

- (void)setQuadLayer:(WhirlyKitQuadDisplayLayer *)layer
{
    _quadLayer = layer;
}

- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene
{
    // Flush out any existing change requests
    if (!changeRequests.empty())
    {
        [layer.layerThread addChangeRequests:(changeRequests)];
        changeRequests.clear();
    }
    
    ChangeSet theChangeRequests;

    pthread_mutex_lock(&tileLock);
    for (LoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
    {
        LoadedTile *tile = *it;
        tile->clearContents(tileBuilder,theChangeRequests);
    }
    pthread_mutex_unlock(&tileLock);

    if (tileBuilder)
       tileBuilder->clearAtlases(theChangeRequests);
    
    [layer.layerThread addChangeRequests:(theChangeRequests)];
    
    [self clear];
}

- (void)setTesselationSizeX:(int)x y:(int)y
{
    defaultTessX = x;
    defaultTessY = y;
}

// Convert from our image type to a GL enum
- (GLenum)glFormat
{
    switch (_imageType)
    {
        case WKTileIntRGBA:
        default:
            return GL_UNSIGNED_BYTE;
            break;
        case WKTileUShort565:
            return GL_UNSIGNED_SHORT_5_6_5;
            break;
        case WKTileUShort4444:
            return GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case WKTileUShort5551:
            return GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case WKTileUByteRed:
        case WKTileUByteGreen:
        case WKTileUByteBlue:
        case WKTileUByteAlpha:
        case WKTileUByteRGB:
            return GL_ALPHA;
            break;
        case WKTilePVRTC4:
            return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            break;
    }
    
    return GL_UNSIGNED_BYTE;
}

// If we're doing single byte conversion, where from?
- (WKSingleByteSource)singleByteSource
{
    switch (_imageType)
    {
        case WKTileUByteRed:
            return WKSingleRed;
            break;
        case WKTileUByteGreen:
            return WKSingleGreen;
            break;
        case WKTileUByteBlue:
            return WKSingleBlue;
            break;
        case WKTileUByteAlpha:
            return WKSingleAlpha;
            break;
        case WKTileUByteRGB:
        default:
            return WKSingleRGB;
            break;
    }
}

// Look for a specific tile
- (LoadedTile *)getTile:(Quadtree::Identifier)ident
{
    LoadedTile *retTile = NULL;
    
    pthread_mutex_lock(&tileLock);
    LoadedTile dummyTile;
    dummyTile.nodeInfo.ident = ident;
    LoadedTileSet::iterator it = tileSet.find(&dummyTile);
    
    if (it != tileSet.end())
        retTile = *it;
    
    pthread_mutex_unlock(&tileLock);
    
    return retTile;
}

// Make all the various parents update their child geometry
- (void)refreshParents:(WhirlyKitQuadDisplayLayer *)layer
{
    // Update just the parents that have changed recently
    for (std::set<Quadtree::Identifier>::iterator it = parents.begin();
         it != parents.end(); ++it)
    {
        LoadedTile *theTile = [self getTile:*it];
        if (theTile && !theTile->isLoading)
        {
//            NSLog(@"Updating parent (%d,%d,%d)",theTile->nodeInfo.ident.x,theTile->nodeInfo.ident.y,
//                  theTile->nodeInfo.ident.level);
            LoadedTile *childTiles[4];
            for (unsigned int iy=0;iy<2;iy++)
                for (unsigned int ix=0;ix<2;ix++)
                {
                    Quadtree::Identifier childIdent(2*theTile->nodeInfo.ident.x+ix,2*theTile->nodeInfo.ident.y+iy,theTile->nodeInfo.ident.level+1);
                    childTiles[iy*2+ix] = [self getTile:childIdent];
                }
            theTile->updateContents(tileBuilder,childTiles,changeRequests);
        }
    }
    parents.clear();
}

// This is not used, but it gets rid of the @selector warning below
- (void)wakeUp
{
}

// Flush out any outstanding updates saved in the changeRequests
- (void)flushUpdates:(WhirlyKitLayerThread *)layerThread
{
//    tileBuilder->flushUpdates(changeRequests);
    // Note: Need to replace selector with some lock-like thing
    if (tileBuilder && tileBuilder->drawAtlas)
    {
        if (tileBuilder->drawAtlas->hasUpdates() && !tileBuilder->drawAtlas->waitingOnSwap())
        {
            tileBuilder->drawAtlas->swap(changeRequests,_quadLayer,@selector(wakeUp));
        }
    }

    // If we added geometry or textures, we may need to reset this
    if (tileBuilder)
    {
        [self runSetCurrentImage:changeRequests];
    }
    
    if (!changeRequests.empty())
    {
        [layerThread addChangeRequests:(changeRequests)];
        changeRequests.clear();
//        [layerThread flushChangeRequests];
    }
}

// Update the texture usage info for the texture atlases
- (void)updateTexAtlasMapping
{
    if (tileBuilder)
        tileBuilder->updateAtlasMappings();
}

// Dump out some information on resource usage
- (void)log
{
    tileBuilder->log(name);
}

#pragma mark - Loader delegate

// We can do another fetch if we haven't hit the max
- (bool)isReady
{
    // Make sure we're not fetching too much at once
    if (networkFetches.size()+localFetches.size() >= [dataSource maxSimultaneousFetches])
        return false;
    
    // And make sure we're not waiting on buffer switches
    if (tileBuilder && !tileBuilder->isReady())
        return false;
    
    return true;
}

- (int)networkFetches
{
    return networkFetches.size();
}

- (int)localFetches
{
    return localFetches.size();
}

// Ask the data source to start loading the image for this tile
- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer loadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    // Build the new tile
    LoadedTile *newTile = new LoadedTile();
    newTile->nodeInfo = tileInfo;
    newTile->isLoading = true;

    pthread_mutex_lock(&tileLock);
    tileSet.insert(newTile);
    pthread_mutex_unlock(&tileLock);
    
    bool isNetworkFetch = ![dataSource respondsToSelector:@selector(tileIsLocalLevel:col:row:)] || ![dataSource tileIsLocalLevel:tileInfo.ident.level col:tileInfo.ident.x row:tileInfo.ident.y];
    if (isNetworkFetch)
        networkFetches.insert(tileInfo.ident);
    else
        localFetches.insert(tileInfo.ident);
    
    [dataSource quadTileLoader:self startFetchForLevel:tileInfo.ident.level col:tileInfo.ident.x row:tileInfo.ident.y attrs:tileInfo.attrs];
}

// Check if we're in the process of loading the given tile
- (bool)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer canLoadChildrenOfTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    LoadedTile *tile = [self getTile:tileInfo.ident];
    if (!tile)
        return false;
    
    // If it's not loading, sure
    return !tile->isLoading;
}

// When the data source loads the image, we'll get called here
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)inDataSource loadedImage:(NSData *)image pvrtcSize:(int)pvrtcSize forLevel:(int)level col:(int)col row:(int)row
{
    WhirlyKitLoadedImage *loadImage = [[WhirlyKitLoadedImage alloc] init];
    if (pvrtcSize != 0)
    {
        loadImage.type = WKLoadedImagePVRTC4;
        loadImage.width = loadImage.height = pvrtcSize;
        loadImage.imageData = image;
    } else {
        loadImage.type = WKLoadedImageNSDataAsImage;
        loadImage.imageData = image;
    }

    [self dataSource:inDataSource loadedImage:loadImage forLevel:level col:col row:row];
}

- (bool)tileIsPlaceholder:(id)loadTile
{
    if ([loadTile isKindOfClass:[WhirlyKitLoadedImage class]])
    {
        WhirlyKitLoadedImage *loadImage = (WhirlyKitLoadedImage *)loadTile;
        return loadImage.type == WKLoadedImagePlaceholder;
    }
    else if ([loadTile isKindOfClass:[WhirlyKitElevationChunk class]])
        return false;
    else if ([loadTile isKindOfClass:[WhirlyKitLoadedTile class]])
    {
        WhirlyKitLoadedTile *theTile = (WhirlyKitLoadedTile *)loadTile;
        if ([theTile.images count] > 0)
        {
            WhirlyKitLoadedImage *loadImage = (WhirlyKitLoadedImage *)[theTile.images objectAtIndex:0];
            return loadImage.type == WKLoadedImagePlaceholder;
        }
    }
    
    return false;
}

- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadTile forLevel:(int)level col:(int)col row:(int)row
{
    bool isPlaceholder = [self tileIsPlaceholder:loadTile];
    
    if (!isPlaceholder && !tileBuilder)
    {
        tileBuilder = new TileBuilder(_quadLayer.coordSys,_quadLayer.mbr,_quadLayer.quadtree);
        tileBuilder->tileScale = _tileScale;
        tileBuilder->fixedTileSize = _fixedTileSize;
        tileBuilder->drawOffset = _drawOffset;
        tileBuilder->drawPriority = _drawPriority;
        tileBuilder->minVis = _minVis;
        tileBuilder->maxVis = _maxVis;
        tileBuilder->hasAlpha = _hasAlpha;
        tileBuilder->color = _color;
        tileBuilder->programId = _programId;
        tileBuilder->includeElev = _includeElev;
        tileBuilder->useElevAsZ = _useElevAsZ;
        tileBuilder->ignoreEdgeMatching = _ignoreEdgeMatching;
        tileBuilder->coverPoles = _coverPoles;
        tileBuilder->glFormat = [self glFormat];
        tileBuilder->singleByteSource = [self singleByteSource];
        tileBuilder->defaultSphereTessX = defaultTessX;
        tileBuilder->defaultSphereTessY = defaultTessY;
        tileBuilder->texelBinSize = 64;
        tileBuilder->scene = _quadLayer.scene;
        tileBuilder->lineMode = false;
        tileBuilder->borderTexel = _borderTexel;

        // If we haven't decided how many active textures we'll have, do that
        if (_activeTextures == -1)
        {
            switch (_numImages)
            {
                case 0:
                    _activeTextures = 0;
                    break;
                case 1:
                    _activeTextures = 1;
                    break;
                default:
                    _activeTextures = 2;
                    break;
            }
        }
        tileBuilder->activeTextures = _activeTextures;
    }

    // Update stats, including network fetches
    Quadtree::Identifier tileIdent(col,row,level);
    std::set<WhirlyKit::Quadtree::Identifier>::iterator nit = networkFetches.find(tileIdent);
    if (nit != networkFetches.end())
        networkFetches.erase(nit);
    nit = localFetches.find(tileIdent);
    if (nit != localFetches.end())
        localFetches.erase(nit);

    // Look for the tile
    // If it's not here, just drop this on the floor
    pthread_mutex_lock(&tileLock);
    LoadedTile dummyTile(tileIdent);
    LoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it == tileSet.end())
    {
        pthread_mutex_unlock(&tileLock);
        return;
    }
    
    
    std::vector<WhirlyKitLoadedImage *> loadImages;
    WhirlyKitElevationChunk *loadElev = nil;
    if ([loadTile isKindOfClass:[WhirlyKitLoadedImage class]])
        loadImages.push_back(loadTile);
    else if ([loadTile isKindOfClass:[WhirlyKitElevationChunk class]])
        loadElev = loadTile;
    else if ([loadTile isKindOfClass:[WhirlyKitLoadedTile class]])
    {
        WhirlyKitLoadedTile *toLoad = loadTile;
        
        for (WhirlyKitLoadedImage *loadImage in toLoad.images)
            loadImages.push_back(loadImage);
        loadElev = toLoad.elevChunk;
    }
    
    bool loadingSuccess = true;
    if (!isPlaceholder && _numImages != loadImages.size())
    {
        // Only print out a message if they bothered to hand in something.  If not, they meant
        //  to tell us it was empty.
        if (loadTile)
            NSLog(@"TileQuadLoader: Got %ld images in callback, but was expecting %d.  Punting tile.",loadImages.size(),_numImages);
        loadingSuccess = false;
    }
    
    // Create the dynamic texture atlas before we need it
    bool createdAtlases = false;
    if (!isPlaceholder && loadingSuccess && _useDynamicAtlas && !tileBuilder->texAtlas && !loadImages.empty())
    {
        int estTexX = tileBuilder->defaultSphereTessX, estTexY = tileBuilder->defaultSphereTessY;
        if (loadElev)
        {
            estTexX = std::max(loadElev.numX-1,estTexX);
            estTexY = std::max(loadElev.numY-1,estTexY);
        }
        tileBuilder->initAtlases(_imageType,_numImages,_textureAtlasSize,estTexX,estTexY);

        createdAtlases = true;
    }
    
    LoadedTile *tile = *it;
    tile->isLoading = false;
    if (loadingSuccess && (isPlaceholder || !loadImages.empty() || loadElev))
    {
        tile->elevData = loadElev;
        if (tile->addToScene(tileBuilder,loadImages,currentImage0,currentImage1,loadElev,changeRequests))
        {
            // If we have more than one image to dispay, make sure we're doing the right one
            if (!isPlaceholder && _numImages > 1 && tileBuilder->texAtlas)
            {
                tile->setCurrentImages(tileBuilder, currentImage0, currentImage1, changeRequests);
            }
        } else
            loadingSuccess = false;
    }

    if (loadingSuccess)
        [_quadLayer loader:self tileDidLoad:tile->nodeInfo.ident];
    else {
        // Shouldn't have a visual representation, so just lose it
        [_quadLayer loader:self tileDidNotLoad:tile->nodeInfo.ident];
        tileSet.erase(it);
        delete tile;
    }
    pthread_mutex_unlock(&tileLock);

//    NSLog(@"Loaded image for tile (%d,%d,%d)",col,row,level);
    
    // Various child state changed so let's update the parents
    if (level > 0)
        parents.insert(Quadtree::Identifier(col/2,row/2,level-1));
    
    if (!doingUpdate)
        [self flushUpdates:_quadLayer.layerThread];

    if (!isPlaceholder)
        [self updateTexAtlasMapping];

    // They might have set the current image already
    //  so we need to update things right here
    if (createdAtlases)
        [self runSetCurrentImage:changeRequests];
}

// We'll get this before a series of unloads and loads
- (void)quadDisplayLayerStartUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
    doingUpdate = true;
}

- (void)quadDisplayLayer:(WhirlyKitQuadDisplayLayer *)layer unloadTile:(WhirlyKit::Quadtree::NodeInfo)tileInfo
{
    // Might be unloading something we're in the middle of fetches
    std::set<WhirlyKit::Quadtree::Identifier>::iterator nit = networkFetches.find(tileInfo.ident);
    if (nit != networkFetches.end())
        networkFetches.erase(nit);
    nit = localFetches.find(tileInfo.ident);
    if (nit != localFetches.end())
        localFetches.erase(nit);
    
    // Get rid of an old tile
    pthread_mutex_lock(&tileLock);
    LoadedTile dummyTile;
    dummyTile.nodeInfo.ident = tileInfo.ident;
    LoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        LoadedTile *theTile = *it;
        
        // Note: Debugging check
        std::vector<Quadtree::Identifier> childIDs;
        layer.quadtree->childrenForNode(theTile->nodeInfo.ident, childIDs);
        if (childIDs.size() > 0)
            NSLog(@" *** Deleting node with children *** ");
        
        theTile->clearContents(tileBuilder,changeRequests);
        tileSet.erase(it);
        delete theTile;
    }
    pthread_mutex_unlock(&tileLock);
    
//    NSLog(@"Unloaded tile (%d,%d,%d)",tileInfo.ident.x,tileInfo.ident.y,tileInfo.ident.level);

    // We'll put this on the list of parents to update, but it'll actually happen in EndUpdates
    if (tileInfo.ident.level > 0)
        parents.insert(Quadtree::Identifier(tileInfo.ident.x/2,tileInfo.ident.y/2,tileInfo.ident.level-1));
    
    [self updateTexAtlasMapping];
}

// Run the parent updates, without doing a flush
- (void)updateWithoutFlush
{
    [self refreshParents:_quadLayer];
}

// Thus ends the unloads.  Now we can update parents
- (void)quadDisplayLayerEndUpdates:(WhirlyKitQuadDisplayLayer *)layer
{
    if (doingUpdate)
    {
        [self refreshParents:layer];
        
        [self flushUpdates:layer.layerThread];
    }
    
    doingUpdate = false;
}

// This may be called on any thread
- (void)setCurrentImage:(int)newImage changes:(WhirlyKit::ChangeSet &)theChanges;
{
    if (!_quadLayer)
        return;

    if (currentImage0 != newImage || currentImage1 != 0)
    {
        // Note: Might be a race condition with updating these guys
        currentImage0 = newImage;
        currentImage1 = 0;
        
        // Change the draw atlases' drawables at once
        if (_useDynamicAtlas)
        {
            if (tileBuilder)
            {
                std::vector<DynamicDrawableAtlas::DrawTexInfo> theDrawTexInfo;
                std::vector<SimpleIdentity> baseTexIDs,newTexIDs;
                
                // Copy this out to avoid locking too long
                pthread_mutex_lock(&tileBuilder->texAtlasMappingLock);
                if (tileBuilder->texAtlas)
                    baseTexIDs = tileBuilder->texAtlasMappings[0];
                if (newImage < tileBuilder->texAtlasMappings.size())
                    newTexIDs = tileBuilder->texAtlasMappings[newImage];
                theDrawTexInfo = tileBuilder->drawTexInfo;
                pthread_mutex_unlock(&tileBuilder->texAtlasMappingLock);
                
                // If these are different something's gone very wrong
                if (baseTexIDs.size() == newTexIDs.size())
                {
                    // Now for the change requests
                    for (unsigned int ii=0;ii<theDrawTexInfo.size();ii++)
                    {
                        const DynamicDrawableAtlas::DrawTexInfo &drawInfo = theDrawTexInfo[ii];
                        for (unsigned int jj=0;jj<baseTexIDs.size();jj++)
                            if (drawInfo.baseTexId == baseTexIDs[jj])
                                theChanges.push_back(new BigDrawableTexChangeRequest(drawInfo.drawId,0,newTexIDs[jj]));
                    }
                }
            }
        } else {
            // We'll look through the tiles and change them all accordingly
            pthread_mutex_lock(&tileLock);

            // No atlases, so changes tiles individually
            for (LoadedTileSet::iterator it = tileSet.begin();
                 it != tileSet.end(); ++it)
            {
                (*it)->setCurrentImages(tileBuilder, currentImage0, currentImage1, theChanges);
            }

            pthread_mutex_unlock(&tileLock);
        }
    }
}

- (void)runSetCurrentImage:(WhirlyKit::ChangeSet &)theChanges
{
    std::vector<DynamicDrawableAtlas::DrawTexInfo> theDrawTexInfo;
    std::vector<SimpleIdentity> baseTexIDs,startTexIDs,endTexIDs;
    
    pthread_mutex_lock(&tileBuilder->texAtlasMappingLock);
    // Copy this out to avoid locking too long
    if (tileBuilder->texAtlasMappings.size() > 0)
        baseTexIDs = tileBuilder->texAtlasMappings[0];
    if (currentImage0 != -1 && currentImage0 < tileBuilder->texAtlasMappings.size())
        startTexIDs = tileBuilder->texAtlasMappings[currentImage0];
    if (currentImage1 != -1 && currentImage1 < tileBuilder->texAtlasMappings.size())
        endTexIDs = tileBuilder->texAtlasMappings[currentImage1];
    theDrawTexInfo = tileBuilder->drawTexInfo;
    pthread_mutex_unlock(&tileBuilder->texAtlasMappingLock);
    
    // If these are different something's gone very wrong
    // Well, actually this happens if we're setting the start or end image to nothing
//    if (baseTexIDs.size() == startTexIDs.size() && baseTexIDs.size() == endTexIDs.size())
    {
        // Now for the change requests
        for (unsigned int ii=0;ii<theDrawTexInfo.size();ii++)
        {
            const DynamicDrawableAtlas::DrawTexInfo &drawInfo = theDrawTexInfo[ii];
            for (unsigned int jj=0;jj<baseTexIDs.size();jj++)
                if (drawInfo.baseTexId == baseTexIDs[jj])
                {
                    theChanges.push_back(new BigDrawableTexChangeRequest(drawInfo.drawId,0,(currentImage0 == -1 ? EmptyIdentity : startTexIDs[jj])));
                    theChanges.push_back(new BigDrawableTexChangeRequest(drawInfo.drawId,1,(currentImage1 == -1 ? EmptyIdentity :endTexIDs[jj])));
                }
        }
    }
}

- (void)setCurrentImageStart:(int)startImage end:(int)endImage changes:(WhirlyKit::ChangeSet &)theChanges
{
    if (!_quadLayer)
        return;
    
    if (currentImage0 != startImage || currentImage1 != endImage)
    {
        currentImage0 = startImage;
        currentImage1 = endImage;
        
        // Change all the draw atlases at once
        if (_useDynamicAtlas)
        {
            if (tileBuilder)
                [self runSetCurrentImage:theChanges];
        } else {
            // We'll look through the tiles and change them all accordingly
            pthread_mutex_lock(&tileLock);

            // No atlases, so changes tiles individually
            for (LoadedTileSet::iterator it = tileSet.begin();
                 it != tileSet.end(); ++it)
            {
                (*it)->setCurrentImages(tileBuilder, currentImage0, currentImage1, theChanges);
            }

            pthread_mutex_unlock(&tileLock);
        }
    }
}

- (void)runSetEnable:(NSNumber *)newEnableObj
{
    bool newEnable = [newEnableObj boolValue];
    if (newEnable == _enable)
        return;
    
    _enable = newEnable;
    
    if (!_quadLayer)
        return;
    
    ChangeSet theChanges;
    if (_useDynamicAtlas)
    {
        if (tileBuilder)
        {
            tileBuilder->enabled = _enable;
            if (tileBuilder->drawAtlas)
                tileBuilder->drawAtlas->setEnableAllDrawables(_enable, theChanges);
        }
    } else {
        // We'll look through the tiles and change them all accordingly
        pthread_mutex_lock(&tileLock);
        
        // No atlases, so changes tiles individually
        for (LoadedTileSet::iterator it = tileSet.begin();
             it != tileSet.end(); ++it)
            (*it)->setEnable(tileBuilder, _enable, theChanges);
        
        pthread_mutex_unlock(&tileLock);
    }
    
    [_quadLayer.layerThread addChangeRequests:theChanges];
}

- (void)setEnable:(bool)enable
{
    if (!_quadLayer)
    {
        _enable = enable;
        return;
    }
    
    if ([NSThread currentThread] != _quadLayer.layerThread)
    {
        [self performSelector:@selector(runSetEnable:) onThread:_quadLayer.layerThread withObject:@(enable) waitUntilDone:NO];
    } else {
        [self runSetEnable:@(enable)];
    }
}

// We'll try to skip updates
- (bool)shouldUpdate:(WhirlyKitViewState *)viewState initial:(bool)isInitial
{
    bool doUpdate = true;;

    // Always do at least one
    if (isInitial)
        return true;

    // Test against the visibility range
    if ((_minVis != DrawVisibleInvalid && _maxVis != DrawVisibleInvalid) || (_minPageVis != DrawVisibleInvalid && _maxPageVis != DrawVisibleInvalid))
    {
        WhirlyGlobeViewState *globeViewState = (WhirlyGlobeViewState *)viewState;
        if ([globeViewState isKindOfClass:[WhirlyGlobeViewState class]])
        {
            if (((_minVis != DrawVisibleInvalid && _maxVis != DrawVisibleInvalid) && (globeViewState.heightAboveGlobe < _minVis || globeViewState.heightAboveGlobe > _maxVis)))
                doUpdate = false;
            if ((_minPageVis != DrawVisibleInvalid && _maxPageVis != DrawVisibleInvalid) && (globeViewState.heightAboveGlobe < _minPageVis || globeViewState.heightAboveGlobe > _maxPageVis))
                doUpdate = false;
        }
    }
    
    return doUpdate;
}

@end
