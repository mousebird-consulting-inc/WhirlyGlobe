/*
 *  DynamicTextureAtlas.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/28/13.
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

#import "DynamicTextureAtlas.h"
#import "GLUtils.h"

using namespace Eigen;

namespace WhirlyKit
{
 
DynamicTexture::DynamicTexture(const std::string &name,int texSize,int cellSize,GLenum inFormat)
    : TextureBase(name), texSize(texSize), cellSize(cellSize), numCell(0), numRegions(0), compressed(false)
{
    if (texSize <= 0 || cellSize <= 0)
        return;
    
    // Check for the formats we'll accept
    switch (inFormat)
    {
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
            format = GL_RGBA;
            type = inFormat;
            break;
        case GL_ALPHA:
            format = GL_ALPHA;
            type = GL_UNSIGNED_BYTE;
            break;
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
            compressed = true;
            format = GL_RGBA;
            type = inFormat;
            break;
        default:
            return;
            break;
    }
    
    numCell = texSize/cellSize;
    layoutGrid = new bool[numCell * numCell];
    for (unsigned int ii=0;ii<numCell * numCell;ii++)
        layoutGrid[ii] = false;
    
    pthread_mutex_init(&regionLock,NULL);
}
    
DynamicTexture::~DynamicTexture()
{
    if (!layoutGrid)
        return;
    
    delete [] layoutGrid;
    layoutGrid = NULL;
    
    pthread_mutex_destroy(&regionLock);
}

// Create the OpenGL texture, empty
bool DynamicTexture::createInGL(OpenGLMemManager *memManager)
{
    // Already setup
    if (glId != 0)
        return true;
    
    glId = memManager->getTexID();
    if (!glId)
        return false;
    glBindTexture(GL_TEXTURE_2D, glId);
    CheckGLError("DynamicTexture::createInGL() glBindTexture()");
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (compressed)
    {
        size_t size = texSize * texSize / 2;
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, texSize, texSize, 0, size, NULL);
    } else {
        // Turn this on to provide glTexImage2D with empty memory so Instruments doesn't complain
//        size_t size = texSize*texSize*4;
//        unsigned char *zeroMem = (unsigned char *)malloc(size);
//        memset(zeroMem, 255, size);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, format, zeroMem);
//        free(zeroMem);
        glTexImage2D(GL_TEXTURE_2D, 0, format, texSize, texSize, 0, format, type, NULL);
    }
    CheckGLError("DynamicTexture::createInGL() glTexImage2D()");
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}
    
void DynamicTexture::destroyInGL(OpenGLMemManager *memManager)
{
	if (glId)
        memManager->removeTexID(glId);    
}
    
void DynamicTexture::addTexture(Texture *tex,const Region &region)
{
    int startX = region.sx * cellSize;
    int startY = region.sy * cellSize;
    int width = tex->getWidth();
    int height = tex->getHeight();
    
    NSData *data = tex->processData();
    addTextureData(startX,startY,width,height,data);
}
    
void DynamicTexture::addTextureData(int startX,int startY,int width,int height,NSData *data)
{
    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, glId);
        CheckGLError("DynamicTexture::createInGL() glBindTexture()");
        if (compressed)
        {
            size_t size = width * height / 2;
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, startX, startY, width, height, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, size, [data bytes]);
        } else
            glTexSubImage2D(GL_TEXTURE_2D, 0, startX, startY, width, height, format, type, [data bytes]);
        CheckGLError("DynamicTexture::addTexture() glTexSubImage2D()");
        glBindTexture(GL_TEXTURE_2D, 0);
    }    
}

void DynamicTexture::setRegion(const Region &region, bool enable)
{
    int sx = std::max(region.sx,0), sy = std::max(region.sy,0);
    int ex = std::min(region.ex,numCell-1), ey = std::min(region.ey,numCell-1);
    
    for (unsigned int ix=sx;ix<=ex;ix++)
        for (unsigned int iy=sy;iy<=ey;iy++)
        {
            layoutGrid[iy*numCell+ix] = enable;
        }
    
    if (enable)
        numRegions++;
    else
        numRegions--;
}
    
bool DynamicTexture::findRegion(int sizeX,int sizeY,Region &region)
{
    // First thing we need to do is clear any outstanding regions
    // Don't sit on the lock, as the main thread uses it
    std::vector<Region> toClear;
    pthread_mutex_lock(&regionLock);
    toClear = releasedRegions;
    releasedRegions.clear();
    pthread_mutex_unlock(&regionLock);
    for (unsigned int ii=0;ii<toClear.size();ii++)
        setRegion(toClear[ii], false);
    
    // Now look for a region that'll fit
    // Look for a spot big enough
    bool found = false;
    int foundX,foundY;
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
    pthread_mutex_lock(&regionLock);
    releasedRegions.push_back(region);
    pthread_mutex_unlock(&regionLock);
}
    
void DynamicTextureClearRegion::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    TextureBase *tex = scene->getTexture(texId);
    DynamicTexture *dynTex = dynamic_cast<DynamicTexture *>(tex);
    if (dynTex)
    {
        dynTex->addRegionToClear(region);
    }
}
    
void DynamicTextureAddRegion::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    TextureBase *tex = scene->getTexture(texId);
    DynamicTexture *dynTex = dynamic_cast<DynamicTexture *>(tex);
    if (dynTex)
    {
        dynTex->addTextureData(startX, startY, width, height, data);
    }    
}
    
DynamicTextureAtlas::DynamicTextureAtlas(int texSize,int cellSize,GLenum format)
    : texSize(texSize), cellSize(cellSize), format(format)
{
}
    
DynamicTextureAtlas::~DynamicTextureAtlas()
{
    for (DynamicTextureSet::iterator it = textures.begin();
         it != textures.end(); ++it)
        delete *it;
    textures.clear();
}
    
// If set, we ask the main thread to do the sub texture loads
static const bool MainThreadMerge = false;
    
bool DynamicTextureAtlas::addTexture(Texture *tex,SubTexture &subTex,OpenGLMemManager *memManager,std::vector<ChangeRequest *> &changes,int borderPixels)
{
    // Make sure we can fit the thing
    if (tex->getWidth() > texSize || tex->getHeight() > texSize)
        return false;
    
    TextureRegion texRegion;    
    
    // Now look for space
    DynamicTexture *dynTex = NULL;
    bool found = false;
    int numCellX = ceil(tex->getWidth() / (float)cellSize), numCellY = ceil(tex->getHeight() / (float)cellSize);
    for (DynamicTextureSet::iterator it = textures.begin();
         it != textures.end(); ++it)
    {
        DynamicTexture *tex = *it;
        DynamicTexture::Region thisRegion;
        if (tex->findRegion(numCellX, numCellY, thisRegion))
        {
            texRegion.region = thisRegion;
            texRegion.dynTexId = tex->getId();
            regions.insert(texRegion);
            dynTex = tex;
            found = true;
            break;
        }
    }
    
    // Didn't find any, so set up a new dynamic texture
    if (!found)
    {
        dynTex = new DynamicTexture("Dynamic Texture Atlas",texSize,cellSize,format);
        dynTex->createInGL(memManager);
        textures.insert(dynTex);
        DynamicTexture::Region thisRegion;
        if (dynTex->findRegion(numCellX, numCellY, thisRegion))
        {
            texRegion.region = thisRegion;
            texRegion.dynTexId = dynTex->getId();
            regions.insert(texRegion);
            found = true;
        }
        changes.push_back(new AddTextureReq(dynTex));
    }
    
    if (found)
    {
        dynTex->setRegion(texRegion.region, true);
//        NSLog(@"Region: (%d,%d)->(%d,%d)  texture: %ld",texRegion.region.sx,texRegion.region.sy,texRegion.region.ex,texRegion.region.ey,dynTex->getId());
        // Note: Making the main thread do the merge
        if (MainThreadMerge)
            changes.push_back(new DynamicTextureAddRegion(dynTex->getId(),
                                                          texRegion.region.sx * cellSize, texRegion.region.sy * cellSize, tex->getWidth(), tex->getHeight(),
                                                          tex->processData()));
        else
            dynTex->addTexture(tex, texRegion.region);

        // This asks for a flush
        changes.push_back(NULL);

        // This way does not take into account borders
        TexCoord org((texRegion.region.sx * cellSize) / (float)texSize, (texRegion.region.sy * cellSize) / (float)texSize);
//        texRegion.subTex.setFromTex(TexCoord(org.x(),org.y()),
//                                    TexCoord(org.x() + tex->getWidth() / (float)texSize , org.y() + tex->getHeight() / (float)texSize));
        
        Point2f boundaryPix;
        if (borderPixels == 0)
            boundaryPix = Point2f(0,0);
        else
//            boundaryPix = Point2f((borderPixels-0.5) / texSize, (borderPixels-0.5) / texSize);
            boundaryPix = Point2f((borderPixels) / (float)texSize, (borderPixels) / (float)texSize);
        texRegion.subTex.setFromTex(TexCoord(org.x() + boundaryPix.x(),org.y() + boundaryPix.y()),
                                    TexCoord(org.x() + tex->getWidth() / (float)texSize - boundaryPix.x(), org.y() + tex->getHeight() / (float)texSize - boundaryPix.y()));
        texRegion.subTex.texId = dynTex->getId();
        
        subTex = texRegion.subTex;
        return true;
    }
    
    return found;
}
    
void DynamicTextureAtlas::removeTexture(const SubTexture &subTex,std::vector<ChangeRequest *> &changes)
{
    TextureRegion texRegion;
    texRegion.subTex.setId(subTex.getId());
    TextureRegionSet::iterator it = regions.find(texRegion);
    if (it != regions.end())
    {
        // We'll stop keeping track of the region
        TextureRegion theRegion = *it;
        // Tell the dynamic texture to clear it out, but we'll send that request over to
        //  the renderer so we can be sure we're not still using it
        changes.push_back(new DynamicTextureClearRegion(theRegion.dynTexId,theRegion.region));
        regions.erase(it);
    }
}
    
void DynamicTextureAtlas::shutdown(std::vector<ChangeRequest *> &changes)
{
    for (DynamicTextureSet::iterator it = textures.begin(); it != textures.end(); ++it)
    {
        DynamicTexture *tex = *it;
        changes.push_back(new RemTextureReq(tex->getId()));
    }
    textures.clear();
    regions.clear();
}

}
