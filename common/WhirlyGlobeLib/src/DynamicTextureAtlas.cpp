/*  DynamicTextureAtlas.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/28/13.
 *  Copyright 2011-2023 mousebird consulting
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
 */

#import "DynamicTextureAtlas.h"
#import "Scene.h"
#import "SceneRenderer.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{

DynamicTexture::DynamicTexture(const std::string &name) :
    TextureBase(name)
{
}

void DynamicTexture::setup(int inTexSize,int inCellSize,TextureType inType,bool inClearTextures)
{
    texSize = inTexSize;
    cellSize = inCellSize;
    type = inType;
    clearTextures = inClearTextures;
    numCell = texSize/cellSize;
    layoutGrid = new bool[numCell * numCell];
    for (unsigned int ii=0;ii<numCell * numCell;ii++)
        layoutGrid[ii] = false;
}

DynamicTexture::~DynamicTexture()
{
    try
    {
        delete [] layoutGrid;
        layoutGrid = nullptr;
    }
    WK_STD_DTOR_CATCH()
}

void DynamicTexture::addTexture(Texture *tex,const Region &region)
{
    const int startX = region.sx * cellSize;
    const int startY = region.sy * cellSize;
    const int width = tex->getWidth();
    const int height = tex->getHeight();

    RawDataRef data = tex->processData();
    addTextureData(startX,startY,width,height,std::move(data));
}

void DynamicTexture::setRegion(const Region &region, bool enable)
{
    const int sx = std::max(region.sx,0), sy = std::max(region.sy,0);
    const int ex = std::min(region.ex,numCell-1), ey = std::min(region.ey,numCell-1);
    
    for (unsigned int ix=sx;ix<=ex;ix++)
        for (unsigned int iy=sy;iy<=ey;iy++)
        {
            layoutGrid[iy*numCell+ix] = enable;
        }
}
    
void DynamicTexture::clearRegion(const Region &clearRegion,ChangeSet &changes,bool mainThreadMerge,unsigned char *emptyData)
{
    int startX = clearRegion.sx * cellSize;
    int startY = clearRegion.sy * cellSize;
    int width = (clearRegion.ex - clearRegion.sx + 1) * cellSize;
    int height = (clearRegion.ey - clearRegion.sy + 1) * cellSize;
    clearTextureData(startX,startY,width,height,changes,mainThreadMerge,emptyData);
}
    
void DynamicTexture::getReleasedRegions(std::vector<DynamicTexture::Region> &toClear) const
{
    std::lock_guard<std::mutex> guardLock(regionLock);
    toClear = releasedRegions;
}

std::vector<DynamicTexture::Region> DynamicTexture::getReleasedRegions() const
{
    std::lock_guard<std::mutex> guardLock(regionLock);
    return releasedRegions;
}

bool DynamicTexture::findRegion(int sizeX,int sizeY,Region &region)
{
    // First thing we need to do is clear any outstanding regions
    // Don't sit on the lock, as the main thread uses it
    std::vector<Region> toClear;
    {
        std::lock_guard<std::mutex> guardLock(regionLock);
        toClear.swap(releasedRegions);
    }

    for (const auto &ii : toClear)
    {
        setRegion(ii, false);
    }
    
    // Now look for a region that'll fit
    // Look for a spot big enough
    bool found = false;
    int foundX=0,foundY=0;
    for (int iy=0;iy<=numCell-sizeY && !found;iy++)
        for (int ix=0;ix<=numCell-sizeX && !found;ix++)
        {
            bool clear = true;
            for (int testY=0;testY<sizeY && clear;testY++)
                for (int testX=0;testX<sizeX && clear;testX++)
                {
                    if (layoutGrid[(testY+iy)*numCell+(testX+ix)])
                        clear = false;
                }
            if (clear)
            {
                foundX = ix;
                foundY = iy;
                found = true;
            }
        }
    
    if (!found)
        return false;
    
    // Found one, so fill it in
    region.sx = foundX;  region.sy = foundY;
    region.ex = foundX+sizeX-1;  region.ey = foundY+sizeY-1;
    
    return true;
}
    
void DynamicTexture::addRegionToClear(const Region &region)
{
    std::lock_guard<std::mutex> guardLock(regionLock);
    releasedRegions.push_back(region);
}

void DynamicTexture::getUtilization(int &outNumCell,int &usedCell)
{
    outNumCell = numCell*numCell;
    usedCell = 0;
    for (unsigned int ii=0;ii<numCell*numCell;ii++)
    {
        if (layoutGrid[ii])
            usedCell++;
    }
}
    
void DynamicTextureClearRegion::execute(Scene *scene,SceneRenderer *renderer,View *view)
{
    TextureBaseRef tex = scene->getTexture(texId);
    DynamicTextureRef dynTex = std::dynamic_pointer_cast<DynamicTexture>(tex);
    if (dynTex)
    {
        dynTex->addRegionToClear(region);
    }
}

DynamicTextureAddRegion::~DynamicTextureAddRegion()
{
    if (!wasRun && data)
    {
        wkLogLevel(Warn,"DynamicTextureAddRegion deleted without being run.");
    }
}
    
void DynamicTextureAddRegion::execute(Scene *scene,SceneRenderer *renderer,View *view)
{
    TextureBaseRef tex = scene->getTexture(texId);
    DynamicTextureRef dynTex = std::dynamic_pointer_cast<DynamicTexture>(tex);
    if (dynTex)
    {
        dynTex->addTextureData(startX, startY, width, height, data);
    } else
        wkLogLevel(Warn,"Tried to add texture data to dynamic texture that doesn't exist.");
    wasRun = true;
}
   
// If set, we ask the main thread to do the sub texture loads
#if TARGET_IPHONE_SIMULATOR
    static const bool MainThreadMerge = true;
#else
#ifdef __ANDROID__
// On some devices we're seeing a lot of texture problems when trying to merge
    static const bool MainThreadMerge = true;
#else
    static const bool MainThreadMerge = false;
#endif
#endif

DynamicTextureAtlas::DynamicTextureAtlas(std::string name,int texSize,int cellSize,
                                         TextureType format,int imageDepth,bool mainThreadMerge) :
    name(std::move(name)),
    format(format),
    imageDepth(imageDepth),
    texSize(texSize),
    cellSize(cellSize),
    mainThreadMerge(mainThreadMerge)
{
    if (mainThreadMerge || MainThreadMerge)
    {
        emptyPixelBuffer.resize(texSize*texSize*4,0);
    }
}
    
DynamicTextureAtlas::~DynamicTextureAtlas()
{
    try
    {
        // Clean up anything we might have left over
        for (auto *it : textures)
        {
            delete it;
        }
        textures.clear();
    }
    WK_STD_DTOR_CATCH()
}

bool DynamicTextureAtlas::addTexture(SceneRenderer *sceneRender,
                                     const std::vector<Texture *> &newTextures,
                                     int frame, const Point2f *realSize,
                                     const Point2f *realOffset, SubTexture &subTex,
                                     ChangeSet &changes, int borderPixels, int bufferPixels,
                                     TextureRegion *outTexRegion)
{
    if (newTextures.size() != imageDepth && frame < 0)
        return false;
    
    // Make sure we can fit the thing
    Texture *firstTex = newTextures[0];
    if (firstTex->getWidth() > texSize || firstTex->getHeight() > texSize)
        return false;
    
    TextureRegion texRegion;
    const bool doMainThreadMerge = MainThreadMerge || mainThreadMerge;

    // Clear out any released regions
    for (const auto *dynTexVec : textures)
    {
        const DynamicTextureRef &firstDynTex = dynTexVec->at(0);
        std::vector<DynamicTexture::Region> toClear = firstDynTex->getReleasedRegions();
        for (const DynamicTexture::Region &clearRegion : toClear)
        {
            for (const auto &dynTex : *dynTexVec)
            {
                dynTex->clearRegion(clearRegion,changes,doMainThreadMerge,
                                    doMainThreadMerge ? &emptyPixelBuffer[0] : nullptr);
            }
        }
    }
    
    // Now look for space
    DynamicTextureVec *dynTexVec = nullptr;
    bool found = false;
    const int numCellX = ceil((float)(firstTex->getWidth()+bufferPixels) / (float)cellSize);
    const int numCellY = ceil((float)(firstTex->getHeight()+bufferPixels) / (float)cellSize);
    for (auto *dynTex : textures)
    {
        const DynamicTextureRef &firstDynTex = dynTex->at(0);
        DynamicTexture::Region thisRegion;
        if (firstDynTex->findRegion(numCellX, numCellY, thisRegion))
        {
            texRegion.region = thisRegion;
            texRegion.dynTexId = firstDynTex->getId();
            regions.insert(texRegion);
            dynTexVec = dynTex;
            found = true;
            break;
        }
    }
    
    // Didn't find any, so set up a new dynamic texture
    if (!found)
    {
        dynTexVec = new std::vector<DynamicTextureRef>();
        dynTexVec->reserve(imageDepth);
        for (unsigned int ii=0;ii<imageDepth;ii++)
        {
            auto dynTex = sceneRender->makeDynamicTexture(name);
            dynTex->setup(texSize,cellSize,format,clearTextures);
            dynTex->setInterpType(interpType);
            dynTex->createInRenderer(sceneRender->getRenderSetupInfo());
            dynTexVec->push_back(std::move(dynTex));
        }

//        NSLog(@"Added dynamic texture %ld (%ld)",dynTex->getId(),textures.size());
        textures.insert(dynTexVec);
        DynamicTexture::Region thisRegion;
        if (dynTexVec->at(0)->findRegion(numCellX, numCellY, thisRegion))
        {
            texRegion.region = thisRegion;
            texRegion.dynTexId = dynTexVec->at(0)->getId();
            regions.insert(texRegion);
            found = true;
        }
        for (unsigned int ii=0;ii<dynTexVec->size();ii++)
        {
            // We need this texture added ASAP
            // Otherwise another thread that may be trying to use it can outrun us
            sceneRender->scene->addChangeRequest(new AddTextureReq(dynTexVec->at(ii)));
//            changes.push_back(new AddTextureReq(dynTexVec->at(ii)));
        }
    }
    
    if (found)
    {
        const DynamicTextureRef &dynTex0 = dynTexVec->at(0);
        dynTex0->setRegion(texRegion.region, true);
        dynTex0->getNumRegions()++;
        
        for (unsigned int ii=0;ii<newTextures.size();ii++)
        {
            // If there's only one frame, we're updating that
            const int which = frame == -1 ? ii : frame;
            Texture *tex = newTextures[newTextures.size() == 1 ? 0 : which];
            const DynamicTextureRef &dynTex = dynTexVec->at(which);
            //        NSLog(@"Region: (%d,%d)->(%d,%d)  texture: %ld",texRegion.region.sx,texRegion.region.sy,texRegion.region.ex,texRegion.region.ey,dynTex->getId());
            // Make the main thread do the merge
            if (MainThreadMerge || mainThreadMerge)
                sceneRender->scene->addChangeRequest(new DynamicTextureAddRegion(dynTex->getId(),
                                                              texRegion.region.sx * cellSize, texRegion.region.sy * cellSize, tex->getWidth(), tex->getHeight(),
                                                              tex->processData()));
            else
                dynTex->addTexture(tex, texRegion.region);
        }

        // This asks for a flush
        changes.push_back(nullptr);

        // This way does not take into account borders
        TexCoord org((texRegion.region.sx * cellSize) / (float)texSize,
                     (texRegion.region.sy * cellSize) / (float)texSize);
//        texRegion.subTex.setFromTex(TexCoord(org.x(),org.y()),
//                                    TexCoord(org.x() + tex->getWidth() / (float)texSize , org.y() + tex->getHeight() / (float)texSize));
        
        Point2f boundaryPix;
        // The input textures size might not be the real size of the texture being used.
        // Use the size they passed in specifically for this calculation
        Point2f inTexSize = realSize ? *realSize : Point2f(firstTex->getWidth(),firstTex->getHeight());
        Point2f offset = realOffset ? *realOffset : Point2f(0,0);
        if (borderPixels == 0)
            boundaryPix = Point2f(pixelFudge/texSize,pixelFudge/texSize);
        else
//            boundaryPix = Point2f((borderPixels-0.5) / texSize, (borderPixels-0.5) / texSize);
            boundaryPix = Point2f((borderPixels) / (float)texSize, (borderPixels) / (float)texSize);
        texRegion.subTex.setFromTex(TexCoord(org.x() + boundaryPix.x() + offset.x() / (float)texSize ,org.y() + boundaryPix.y() + offset.y() / (float)texSize),
                                    TexCoord(org.x() + inTexSize.x() / (float)texSize - boundaryPix.x(),
                                             org.y() + inTexSize.y() / (float)texSize - boundaryPix.y()));
        texRegion.subTex.texId = dynTexVec->at(0)->getId();
        
        subTex = texRegion.subTex;
    }
    
    if (found && outTexRegion)
        *outTexRegion = texRegion;
    return found;
}
    
bool DynamicTextureAtlas::updateTexture(Texture *tex,int frame,const TextureRegion &texRegion,ChangeSet &changes)
{
    const DynamicTextureVec *dynTexVec = nullptr;
    
    // Look for the right dynamic texture (list)
    for (const auto *texVec : textures)
    {
        const DynamicTextureRef &firstDynTex = dynTexVec->at(0);
        if (firstDynTex->getId() == texRegion.dynTexId)
        {
            dynTexVec = texVec;
            break;
        }
    }
    
    if (!dynTexVec)
    {
        if (textures.empty())
        {
            return false;
        }
        // Just use the last one
        // todo: the one with the most space would be best
        dynTexVec = *textures.end();
    }
    
    // Look for the matching dynamic texture
    const int which = (frame == -1) ? 0 : frame;
    const DynamicTextureRef &dynTex = dynTexVec->at(which);
    
    // Merge in the data
    //        NSLog(@"Region: (%d,%d)->(%d,%d)  texture: %ld",texRegion.region.sx,texRegion.region.sy,texRegion.region.ex,texRegion.region.ey,dynTex->getId());
    // Make the main thread do the merge
    if (MainThreadMerge)
        changes.push_back(new DynamicTextureAddRegion(dynTex->getId(),
                                                      texRegion.region.sx * cellSize, texRegion.region.sy * cellSize, tex->getWidth(), tex->getHeight(),
                                                      tex->processData()));
    else
        dynTex->addTexture(tex, texRegion.region);
    
    return false;
}
    
void DynamicTextureAtlas::removeTexture(const SubTexture &subTex,ChangeSet &changes,TimeInterval when)
{
    TextureRegion texRegion;
    texRegion.subTex.setId(subTex.getId());
    const auto it = regions.find(texRegion);
    if (it == regions.end())
    {
        wkLogLevel(Warn,"DynamicTextureAtlas: Request to remove non-existent texture.");
        return;
    }
    
    // We'll stop keeping track of the region
    const auto dynTexId = it->dynTexId;

    // Tell the dynamic texture to clear it out, but we'll send that request over to
    //  the renderer so we can be sure we're not still using it
    changes.push_back(new DynamicTextureClearRegion(dynTexId,it->region,when));
    regions.erase(it);
    
    // See if that texture is now empty
    for (const auto *texVec : textures)
    {
        if (dynTexId == texVec->at(0)->getId())
        {
            const DynamicTextureRef &tex = texVec->at(0);
            tex->getNumRegions()--;
            break;
        }
    }
}

bool DynamicTextureAtlas::empty() const
{
    return textures.empty();
}
    
void DynamicTextureAtlas::cleanup(ChangeSet &changes,TimeInterval when)
{
    DynamicTextureSet::iterator itNext;
    for (auto it = textures.begin();it != textures.end(); it = itNext)
    {
        itNext = std::next(it);
        DynamicTextureVec *texVec = *it;
        const DynamicTextureRef &tex = texVec->at(0);
        if (tex->getNumRegions() == 0)
        {
            for (const auto &ti : *texVec)
            {
                changes.push_back(new RemTextureReq(ti->getId(),when));
            }
            delete texVec;
            textures.erase(it);
        }
    }
}
    
void DynamicTextureAtlas::getTextureIDs(std::vector<SimpleIdentity> &texIDs,int which)
{
    for (const auto *dynTexVec : textures)
    {
        if (which < dynTexVec->size())
            texIDs.push_back(dynTexVec->at(which)->getId());
    }
}
    
SimpleIdentity DynamicTextureAtlas::getTextureIDForFrame(SimpleIdentity baseTexID,int which)
{
    for (const auto *dynTexVec : textures)
    {
        if (((*dynTexVec)[0])->getId() == baseTexID && which < dynTexVec->size())
            return ((*dynTexVec)[which])->getId();
    }
    
    return EmptyIdentity;
}
    
void DynamicTextureAtlas::teardown(ChangeSet &changes)
{
    for (auto *texVec : textures)
    {
        for (unsigned int ii=0;ii<texVec->size();ii++)
            changes.push_back(new RemTextureReq(texVec->at(ii)->getId()));
        delete texVec;
    }
    textures.clear();
    regions.clear();
}
    
void DynamicTextureAtlas::getUsage(int &numRegions,int &dynamicTextures) const
{
    numRegions = regions.size();
    dynamicTextures = textures.size();
}

void DynamicTextureAtlas::log() const
{
    int numCells=0,usedCells=0;
    for (const auto *texVec : textures)
    {
        int thisNumCells,thisUsedCells;
        texVec->at(0)->getUtilization(thisNumCells,thisUsedCells);
        numCells += thisNumCells;
        usedCells += thisUsedCells;
    }

    int texelSize = 4;
    switch (format)
    {
        case TexTypeShort565:
        case TexTypeShort4444:
        case TexTypeShort5551:
            texelSize = 2;;
            break;
        case TexTypeUnsignedByte:
            texelSize = 4;
            break;
        case TexTypeSingleChannel:
            texelSize = 1;
            break;
// Note: Porting
#if 0
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
            // Doesn't really matter.  Can't do these.
            texelSize = 1;
            break;
#endif
//        case GL_COMPRESSED_RGB8_ETC2:
//            // Note: Not really
//            texelSize = 1;
//            break;
        default:
            break;
            
    }
    
    wkLogLevel(Warn,"DynamicTextureAtlas: %ld textures, (%.2f MB)",textures.size(),textures.size() * texSize*texSize*texelSize/(float)(1024*1024));
    if (numCells > 0)
        wkLogLevel(Warn,"DynamicTextureAtlas: using %.2f%% of the cells",100 * usedCells / (float)numCells);
}

}
