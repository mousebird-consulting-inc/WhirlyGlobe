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
// Note: This works around an Android boost problem
#define _LITTLE_ENDIAN
#import <boost/math/special_functions/fpclassify.hpp>
#import "TileQuadLoader.h"
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"

using namespace Eigen;

namespace WhirlyKit
{

QuadTileLoader::QuadTileLoader(const std::string &name,QuadTileImageDataSource *imageSource)
    : imageSource(imageSource), name(name),
    enable(true), drawOffset(0), drawPriority(0),
    minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid), minPageVis(DrawVisibleInvalid), maxPageVis(DrawVisibleInvalid),
    programId(EmptyIdentity), includeElev(false), useElevAsZ(true),
    numImages(1), activeTextures(-1), color(255,255,255,255), hasAlpha(false),
    ignoreEdgeMatching(false), coverPoles(false),
    imageType(WKTileIntRGBA), useDynamicAtlas(true), tileScale(WKTileScaleNone), fixedTileSize(256), textureAtlasSize(2048), borderTexel(1),
    tileBuilder(NULL), doingUpdate(false), defaultTessX(10), defaultTessY(10),
    currentImage0(0), currentImage1(0), texAtlasPixelFudge(0.0)
{
    pthread_mutex_init(&tileLock, NULL);
}
    
void QuadTileLoader::clear()
{
    pthread_mutex_lock(&tileLock);
    for (LoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
        delete *it;
    pthread_mutex_unlock(&tileLock);
    tileSet.clear();

    if (tileBuilder)
        delete tileBuilder;

    tileBuilder = NULL;

    parents.clear();
}
    
QuadTileLoader::~QuadTileLoader()
{
    clear();
    
    for (unsigned int ii=0;ii<changeRequests.size();ii++)
        delete changeRequests[ii];
    changeRequests.clear();
    
    pthread_mutex_destroy(&tileLock);
    
}

void QuadTileLoader::setTesselationSize(int x,int y)
{
    defaultTessX = x;
    defaultTessY = y;
}

// Look for a specific tile
InternalLoadedTile *QuadTileLoader::getTile(const Quadtree::Identifier &ident)
{
    InternalLoadedTile *retTile = NULL;
    
    pthread_mutex_lock(&tileLock);
    InternalLoadedTile dummyTile;
    dummyTile.nodeInfo.ident = ident;
    LoadedTileSet::iterator it = tileSet.find(&dummyTile);
    
    if (it != tileSet.end())
        retTile = *it;
    
    pthread_mutex_unlock(&tileLock);
    
    return retTile;
}

// Make all the various parents update their child geometry
void QuadTileLoader::refreshParents()
{
    // If we're in single level mode we don't bother with this
    if (control->getTargetLevel() != -1)
        return;
    
    // Update just the parents that have changed recently
    for (std::set<Quadtree::Identifier>::iterator it = parents.begin();
         it != parents.end(); ++it)
    {
        InternalLoadedTile *theTile = getTile(*it);
        if (theTile && !theTile->isLoading)
        {
//            NSLog(@"Updating parent (%d,%d,%d)",theTile->nodeInfo.ident.x,theTile->nodeInfo.ident.y,
//                  theTile->nodeInfo.ident.level);
            InternalLoadedTile *childTiles[4];
            for (unsigned int iy=0;iy<2;iy++)
                for (unsigned int ix=0;ix<2;ix++)
                {
                    Quadtree::Identifier childIdent(2*theTile->nodeInfo.ident.x+ix,2*theTile->nodeInfo.ident.y+iy,theTile->nodeInfo.ident.level+1);
                    childTiles[iy*2+ix] = getTile(childIdent);
                }
            theTile->updateContents(tileBuilder,childTiles,changeRequests);
        }
    }
    parents.clear();
}
 
// Called by the big drawable when it swaps to the new buffer
void BigDrawableSwapCallback(BigDrawableSwap *swap,void *data)
{
    QuadTileLoader *quadTileLoader = (QuadTileLoader *)data;
    QuadDisplayController *control = quadTileLoader->getController();
    control->wakeUp();
}

// Flush out any outstanding updates saved in the changeRequests
void QuadTileLoader::flushUpdates(ChangeSet &changes)
{
//    tileBuilder->flushUpdates(changeRequests);
    if (tileBuilder && tileBuilder->drawAtlas)
    {
        if (tileBuilder->drawAtlas->hasUpdates() && !tileBuilder->drawAtlas->waitingOnSwap())
        {
            tileBuilder->texAtlas->cleanup(changeRequests);
            tileBuilder->drawAtlas->swap(changeRequests, &BigDrawableSwapCallback, this);
        }
    }

    // If we added geometry or textures, we may need to reset this
    if (tileBuilder && tileBuilder->newDrawables)
    {
        runSetCurrentImage(changeRequests);
        tileBuilder->newDrawables = false;
    }
    
    if (!changeRequests.empty())
    {
        changes.insert(changes.end(), changeRequests.begin(),changeRequests.end());
        changeRequests.clear();
    }
}

// Update the texture usage info for the texture atlases
void QuadTileLoader::updateTexAtlasMapping()
{
    if (tileBuilder)
        tileBuilder->updateAtlasMappings();
}

// Dump out some information on resource usage
//void QuadTileLoader::log()
//{
//    tileBuilder->log(name);
//}

// This may be called on any thread
void QuadTileLoader::setCurrentImage(int newImage,ChangeSet &changes)
{
    if (currentImage0 != newImage || currentImage1 != 0)
    {
        // Note: Might be a race condition with updating these guys
        currentImage0 = newImage;
        currentImage1 = 0;
        
        // Change the draw atlases' drawables at once
        if (useDynamicAtlas)
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
                                changes.push_back(new BigDrawableTexChangeRequest(drawInfo.drawId,0,newTexIDs[jj]));
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
                (*it)->setCurrentImages(tileBuilder, currentImage0, currentImage1, changes);
            }
            
            pthread_mutex_unlock(&tileLock);
        }
    }
}

void QuadTileLoader::runSetCurrentImage(ChangeSet &changes)
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
                            changes.push_back(new BigDrawableTexChangeRequest(drawInfo.drawId,0,(currentImage0 == -1 ? EmptyIdentity : startTexIDs[jj])));
                            changes.push_back(new BigDrawableTexChangeRequest(drawInfo.drawId,1,(currentImage1 == -1 ? EmptyIdentity :endTexIDs[jj])));
                        }
                }
            }
}

void QuadTileLoader::setCurrentImageStart(int startImage,int endImage,ChangeSet &changes)
{
    if (currentImage0 != startImage || currentImage1 != endImage)
    {
        currentImage0 = startImage;
        currentImage1 = endImage;
        
        // Change all the draw atlases at once
        if (useDynamicAtlas)
        {
            if (tileBuilder)
                runSetCurrentImage(changes);
        } else {
            // We'll look through the tiles and change them all accordingly
            pthread_mutex_lock(&tileLock);
            
            // No atlases, so changes tiles individually
            for (LoadedTileSet::iterator it = tileSet.begin();
                 it != tileSet.end(); ++it)
            {
                (*it)->setCurrentImages(tileBuilder, currentImage0, currentImage1, changes);
            }
            
            pthread_mutex_unlock(&tileLock);
        }
    }
}

void QuadTileLoader::setEnable(bool newEnable,ChangeSet &changes)
{
    if (newEnable == enable)
        return;
    
    enable = newEnable;
    
    if (useDynamicAtlas)
    {
        if (tileBuilder)
        {
            tileBuilder->enabled = enable;
            if (tileBuilder->drawAtlas)
                tileBuilder->drawAtlas->setEnableAllDrawables(enable, changes);
        }
    } else {
        // We'll look through the tiles and change them all accordingly
        pthread_mutex_lock(&tileLock);
        
        // No atlases, so changes tiles individually
        for (LoadedTileSet::iterator it = tileSet.begin();
             it != tileSet.end(); ++it)
            (*it)->setEnable(tileBuilder, enable, changes);
            
        pthread_mutex_unlock(&tileLock);
    }
}
    
#pragma mark - Loader delegate

void QuadTileLoader::init(QuadDisplayController *inControl,Scene *inScene)
{
    QuadLoader::init(inControl,inScene);
}
    
void QuadTileLoader::reset(ChangeSet &changes)
{
    // Flush out any existing change requests
    if (!changeRequests.empty())
    {
        changes = changeRequests;
        changeRequests.clear();
    }
    
    pthread_mutex_lock(&tileLock);
    for (LoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
    {
        InternalLoadedTile *tile = *it;
        tile->clearContents(tileBuilder,changes);
    }
    pthread_mutex_unlock(&tileLock);

    networkFetches.clear();
    localFetches.clear();
    
    if (tileBuilder)
        tileBuilder->clearAtlases(changes);
    
    clear();
}

void QuadTileLoader::shutdownLayer(ChangeSet &changes)
{
    reset(changes);
}

// We can do another fetch if we haven't hit the max
bool QuadTileLoader::isReady()
{
    // Make sure we're not fetching too much at once
    if (networkFetches.size()+localFetches.size() >= imageSource->maxSimultaneousFetches())
        return false;
    
    // And make sure we're not waiting on buffer switches
    if (tileBuilder && !tileBuilder->isReady())
        return false;
    
    return true;
}

int QuadTileLoader::numNetworkFetches()
{
    return (int)networkFetches.size();
}

int QuadTileLoader::numLocalFetches()
{
    return (int)localFetches.size();
}

// Ask the data source to start loading the image for this tile
void QuadTileLoader::loadTile(const Quadtree::NodeInfo &tileInfo)
{
    // Build the new tile
    InternalLoadedTile *newTile = new InternalLoadedTile();
    newTile->nodeInfo = tileInfo;
    newTile->isLoading = true;
    newTile->calculateSize(control->getQuadtree(), control->getScene()->getCoordAdapter(), control->getCoordSys());

    pthread_mutex_lock(&tileLock);
    tileSet.insert(newTile);
    pthread_mutex_unlock(&tileLock);
    
    bool isNetworkFetch = !imageSource->tileIsLocal(tileInfo.ident.level,tileInfo.ident.x,tileInfo.ident.y);
    if (isNetworkFetch)
        networkFetches.insert(tileInfo.ident);
    else
        localFetches.insert(tileInfo.ident);
    
    Quadtree::NodeInfo &nonConstTileInfo = const_cast<Quadtree::NodeInfo &>(tileInfo);
    imageSource->startFetch(this, tileInfo.ident.level, tileInfo.ident.x, tileInfo.ident.y, &nonConstTileInfo.attrs);
}

// Check if we're in the process of loading the given tile
bool QuadTileLoader::canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo)
{
    // For single level mode, you can always do this
    if (control->getTargetLevel() != -1)
        return true;

    InternalLoadedTile *tile = getTile(tileInfo.ident);
    if (!tile)
        return false;
    
    // If it's not loading, sure
    return !tile->isLoading;
}

// Note: Porting
//bool QuadTileLoader::tileIsPlaceholder(loadTile
//{
//    if ([loadTile isKindOfClass:[WhirlyKitLoadedImage class]])
//    {
//        WhirlyKitLoadedImage *loadImage = (WhirlyKitLoadedImage *)loadTile;
//        return loadImage.type == WKLoadedImagePlaceholder;
//    }
//    else if ([loadTile isKindOfClass:[WhirlyKitElevationChunk class]])
//        return false;
//    else if ([loadTile isKindOfClass:[WhirlyKitLoadedTile class]])
//    {
//        WhirlyKitLoadedTile *theTile = (WhirlyKitLoadedTile *)loadTile;
//        if ([theTile.images count] > 0)
//        {
//            WhirlyKitLoadedImage *loadImage = (WhirlyKitLoadedImage *)[theTile.images objectAtIndex:0];
//            return loadImage.type == WKLoadedImagePlaceholder;
//        }
//    }
//    
//    return false;
//}

void QuadTileLoader::unloadTile(const Quadtree::NodeInfo &tileInfo)
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
    InternalLoadedTile dummyTile;
    dummyTile.nodeInfo.ident = tileInfo.ident;
    LoadedTileSet::iterator it = tileSet.find(&dummyTile);
    if (it != tileSet.end())
    {
        InternalLoadedTile *theTile = *it;
        
        // Note: Debugging check
        std::vector<Quadtree::Identifier> childIDs;
        control->getQuadtree()->childrenForNode(theTile->nodeInfo.ident, childIDs);
//        if (childIDs.size() > 0)
//            NSLog(@" *** Deleting node with children *** ");
        
        theTile->clearContents(tileBuilder,changeRequests);
        tileSet.erase(it);
        delete theTile;
    }
    pthread_mutex_unlock(&tileLock);
    
//    NSLog(@"Unloaded tile (%d,%d,%d)",tileInfo.ident.x,tileInfo.ident.y,tileInfo.ident.level);
    
    // We'll put this on the list of parents to update, but it'll actually happen in EndUpdates
    if (tileInfo.ident.level > 0 && control->getTargetLevel() == -1)
        parents.insert(Quadtree::Identifier(tileInfo.ident.x/2,tileInfo.ident.y/2,tileInfo.ident.level-1));
    
    updateTexAtlasMapping();
}
    
// We'll get this before a series of unloads and loads
void QuadTileLoader::startUpdates(ChangeSet &changes)
{
    doingUpdate = true;
}

// Run the parent updates, without doing a flush
void QuadTileLoader::updateWithoutFlush()
{
    refreshParents();
}

// Thus ends the unloads.  Now we can update parents
void QuadTileLoader::endUpdates(ChangeSet &changes)
{
    if (doingUpdate)
    {
        refreshParents();
        
        flushUpdates(changes);
    }
    
    doingUpdate = false;
}

// We'll try to skip updates
bool QuadTileLoader::shouldUpdate(ViewState *viewState,bool isInitial)
{
    bool doUpdate = true;

    // Always do at least one
    if (isInitial)
    {
        return true;
    }

    // Test against the visibility range
    if ((minVis != DrawVisibleInvalid && maxVis != DrawVisibleInvalid) || (minPageVis != DrawVisibleInvalid && maxPageVis != DrawVisibleInvalid))
    {
        // Note: Porting.  Won't work for globe
        if (((minVis != DrawVisibleInvalid && maxVis != DrawVisibleInvalid) && (viewState->eyePos.z() < minVis || viewState->eyePos.z() > maxVis)))
                doUpdate = false;
        if ((minPageVis != DrawVisibleInvalid && maxPageVis != DrawVisibleInvalid) && (viewState->eyePos.z() < minPageVis || viewState->eyePos.z() > maxPageVis))
                doUpdate = false;
    }
    
    return doUpdate;
}
    
#pragma mark - QuadTileLoaderSupport methods
    
// Convert from our image type to a GL enum
static GLenum glEnumFromOurFormat(WhirlyKitTileImageType imageType)
{
    switch (imageType)
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
            // Note: Porting
//        case WKTilePVRTC4:
//            return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
//            break;
            // Note: Porting
//        case WKTileETC2_RGB8:
//            return GL_COMPRESSED_RGB8_ETC2;
//            break;
//        case WKTileETC2_RGBA8:
//            return GL_COMPRESSED_RGBA8_ETC2_EAC;
//            break;
//        case WKTileETC2_RGB8_PunchAlpha:
//            return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
//            break;
//        case WKTileEAC_R11:
//            return GL_COMPRESSED_R11_EAC;
//            break;
//        case WKTileEAC_R11_Signed:
//            return GL_COMPRESSED_SIGNED_R11_EAC;
//            break;
//        case WKTileEAC_RG11:
//            return GL_COMPRESSED_RG11_EAC;
//            break;
//        case WKTileEAC_RG11_Signed:
//            return GL_COMPRESSED_SIGNED_RG11_EAC;
//            break;
    }
    
    return GL_UNSIGNED_BYTE;
}

// If we're doing single byte conversion, where from?
static WKSingleByteSource singleByteSourceFromOurFormat(WhirlyKitTileImageType imageType)
{
    switch (imageType)
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
    
void QuadTileLoader::loadedImage(QuadTileImageDataSource *dataSource,LoadedImage *loadImage,int level,int col,int row,ChangeSet &changes)
{
    // Note: Porting
//    bool isPlaceholder = tileIsPlaceholder(loadImage);
    bool isPlaceholder = loadImage && loadImage->isPlaceholder();
    
    if (!isPlaceholder && !tileBuilder)
    {
        QuadDisplayController *control = getController();
        tileBuilder = new TileBuilder(control->getCoordSys(),control->getMbr(),control->getQuadtree());
        tileBuilder->tileScale = tileScale;
        tileBuilder->fixedTileSize = fixedTileSize;
        tileBuilder->drawOffset = drawOffset;
        tileBuilder->drawPriority = drawPriority;
        tileBuilder->minVis = minVis;
        tileBuilder->maxVis = maxVis;
        tileBuilder->hasAlpha = hasAlpha;
        tileBuilder->color = color;
        tileBuilder->programId = programId;
        tileBuilder->includeElev = includeElev;
        tileBuilder->useElevAsZ = useElevAsZ;
        tileBuilder->ignoreEdgeMatching = ignoreEdgeMatching;
        tileBuilder->coverPoles = coverPoles;
        tileBuilder->glFormat = glEnumFromOurFormat(imageType);
        tileBuilder->singleByteSource = singleByteSourceFromOurFormat(imageType);
        tileBuilder->defaultSphereTessX = defaultTessX;
        tileBuilder->defaultSphereTessY = defaultTessY;
        tileBuilder->texelBinSize = 64;
        tileBuilder->scene = control->getScene();
        tileBuilder->lineMode = false;
        tileBuilder->borderTexel = borderTexel;
        tileBuilder->singleLevel = control->getTargetLevel();
        tileBuilder->texAtlasPixelFudge = texAtlasPixelFudge;
        
        // If we haven't decided how many active textures we'll have, do that
        if (activeTextures == -1)
        {
            switch (numImages)
            {
                case 0:
                    activeTextures = 0;
                    break;
                case 1:
                    activeTextures = 1;
                    break;
                default:
                    activeTextures = 2;
                    break;
            }
        }
        tileBuilder->activeTextures = activeTextures;
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
            InternalLoadedTile dummyTile(tileIdent);
            LoadedTileSet::iterator it = tileSet.find(&dummyTile);
            if (it == tileSet.end())
            {
                pthread_mutex_unlock(&tileLock);
                return;
            }
    
    
    std::vector<LoadedImage *> loadImages;
    // Note: Porting
    bool loadElev = false;
//    WhirlyKitElevationChunk *loadElev = nil;
//    if ([loadTile isKindOfClass:[WhirlyKitLoadedImage class]])
//        loadImages.push_back(loadTile);
//        else if ([loadTile isKindOfClass:[WhirlyKitElevationChunk class]])
//            loadElev = loadTile;
//            else if ([loadTile isKindOfClass:[WhirlyKitLoadedTile class]])
//            {
//                WhirlyKitLoadedTile *toLoad = loadTile;
//                
//                for (WhirlyKitLoadedImage *loadImage in toLoad.images)
//                    loadImages.push_back(loadImage);
//                    loadElev = toLoad.elevChunk;
//                    }
    if (loadImage)
        loadImages.push_back(loadImage);
    
    bool loadingSuccess = true;
    if (!isPlaceholder && numImages != loadImages.size())
    {
        // Only print out a message if they bothered to hand in something.  If not, they meant
        //  to tell us it was empty.
//        if (loadTile)
//            NSLog(@"TileQuadLoader: Got %ld images in callback, but was expecting %d.  Punting tile.",loadImages.size(),_numImages);
        loadingSuccess = false;
    }
    
    // Create the dynamic texture atlas before we need it
    bool createdAtlases = false;
    if (!isPlaceholder && loadingSuccess && useDynamicAtlas && !tileBuilder->texAtlas && !loadImages.empty())
    {
        int estTexX = tileBuilder->defaultSphereTessX, estTexY = tileBuilder->defaultSphereTessY;
//        if (loadElev)
//        {
//            estTexX = std::max(loadElev.numX-1,estTexX);
//            estTexY = std::max(loadElev.numY-1,estTexY);
//        }
        tileBuilder->initAtlases(imageType,numImages,textureAtlasSize,estTexX,estTexY);
        
        createdAtlases = true;
    }
    
    InternalLoadedTile *tile = *it;
    tile->isLoading = false;
    if (loadingSuccess && (isPlaceholder || !loadImages.empty() || loadElev))
    {
//        tile->elevData = loadElev;
        if (tile->addToScene(tileBuilder,loadImages,currentImage0,currentImage1,/* loadElev, */changeRequests))
        {
            // If we have more than one image to dispay, make sure we're doing the right one
            if (!isPlaceholder && numImages > 1 && tileBuilder->texAtlas)
            {
                tile->setCurrentImages(tileBuilder, currentImage0, currentImage1, changeRequests);
            }
        } else
            loadingSuccess = false;
            }
    
    if (loadingSuccess)
        control->tileDidLoad(tile->nodeInfo.ident);
    else {
        // Shouldn't have a visual representation, so just lose it
        control->tileDidNotLoad(tile->nodeInfo.ident);
        tileSet.erase(it);
        delete tile;
    }
    pthread_mutex_unlock(&tileLock);
    
    //    NSLog(@"Loaded image for tile (%d,%d,%d)",col,row,level);
    
    // Various child state changed so let's update the parents
    if (level > 0 && control->getTargetLevel() == -1)
        parents.insert(Quadtree::Identifier(col/2,row/2,level-1));
        
    if (!doingUpdate)
    {
        flushUpdates(changes);
    }
    
    if (!isPlaceholder)
        updateTexAtlasMapping();
    
    // They might have set the current image already
    //  so we need to update things right here
    if (createdAtlases)
        runSetCurrentImage(changeRequests);
}
    
    // Note: Porting
//    - (void)runSetDrawPriority:(NSNumber *)newDrawPriorityObj
//    {
//        int newDrawPriority = (int)[newDrawPriorityObj integerValue];
//        if (newDrawPriority == _drawPriority)
//            return;
//        
//        _drawPriority = newDrawPriority;
//        
//        if (!_quadLayer)
//            return;
//        
//        ChangeSet theChanges;
//        if (_useDynamicAtlas)
//        {
//            if (tileBuilder)
//            {
//                tileBuilder->drawPriority = _drawPriority;
//                if (tileBuilder->drawAtlas)
//                    tileBuilder->drawAtlas->setDrawPriorityAllDrawables(_drawPriority, theChanges);
//                    }
//        }
//        
//        [_quadLayer.layerThread addChangeRequests:theChanges];
//    }
//    
//    - (void)setDrawPriority:(int)drawPriority
//    {
//        if (!_quadLayer)
//        {
//            _drawPriority = drawPriority;
//            return;
//        }
//        
//        if ([NSThread currentThread] != _quadLayer.layerThread)
//        {
//            [self performSelector:@selector(runSetDrawPriority:) onThread:_quadLayer.layerThread withObject:@(drawPriority) waitUntilDone:NO];
//        } else {
//            [self runSetDrawPriority:@(drawPriority)];
//        }
//    }
//    
//    - (void)runSetColor:(UIColor *)newColorObj
//    {
//        RGBAColor newColor = [newColorObj asRGBAColor];
//        if (newColor == _color)
//            return;
//        
//        _color = newColor;
//        
//        if (!_quadLayer)
//            return;
//        
//        ChangeSet theChanges;
//        if (_useDynamicAtlas)
//        {
//            if (tileBuilder)
//            {
//                tileBuilder->color = _color;
//                // Note: We can't change the color of existing drawables.  The color is in the data itself.
//            }
//        }
//        [_quadLayer.layerThread addChangeRequests:theChanges];
//    }
//    
//    - (void)setColor:(WhirlyKit::RGBAColor)color
//    {
//        if (!_quadLayer)
//        {
//            _color = color;
//            return;
//        }
//        
//        UIColor *newColor = [UIColor colorWithRed:color.r/255.0 green:color.g/255.0 blue:color.b/255.0 alpha:color.a/255.0];
//        if ([NSThread currentThread] != _quadLayer.layerThread)
//        {
//            [self performSelector:@selector(runSetColor:) onThread:_quadLayer.layerThread withObject:newColor waitUntilDone:NO];
//        } else {
//            [self runSetColor:newColor];
//        }
//    }
//    
//    - (void)runSetProgramId:(NSNumber *)programIDObj
//    {
//        int newProgramID = (int)[programIDObj integerValue];
//        if (newProgramID == _programId)
//            return;
//        
//        _programId = newProgramID;
//        
//        if (!_quadLayer)
//            return;
//        
//        ChangeSet theChanges;
//        if (_useDynamicAtlas)
//        {
//            if (tileBuilder)
//            {
//                tileBuilder->programId = _programId;
//                if (tileBuilder->drawAtlas)
//                    tileBuilder->drawAtlas->setProgramIDAllDrawables(_programId, theChanges);
//                    }
//        }
//        [_quadLayer.layerThread addChangeRequests:theChanges];
//    }
//    
//    - (void)setProgramId:(WhirlyKit::SimpleIdentity)programId
//    {
//        if (!_quadLayer)
//        {
//            _programId = programId;
//            return;
//        }
//        
//        if ([NSThread currentThread] != _quadLayer.layerThread)
//        {
//            [self performSelector:@selector(runSetProgramId:) onThread:_quadLayer.layerThread withObject:@(programId) waitUntilDone:NO];
//        } else {
//            [self runSetProgramId:@(programId)];
//        }
//    }

}
