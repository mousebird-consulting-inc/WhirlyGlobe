/*
 *  BigDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/6/13.
 *  Copyright 2011-2016 mousebird consulting
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

#import "BigDrawable.h"
#import "Scene.h"
#import "SceneRendererES2.h"
#import "GLUtils.h"

namespace WhirlyKit
{
    
BigDrawable::Change::Change(ChangeType type,int whereVert,RawDataRef vertData,int clearLen)
    : type(type), whereVert(whereVert), vertData(vertData), clearLen(clearLen)
{
}
    
BigDrawable::Buffer::Buffer()
    : vertexBufferId(0), elementBufferId(0), numElement(0), vertexArrayObj(0)
{
}

BigDrawable::BigDrawable(const std::string &name,int singleVertexSize,const std::vector<VertexAttribute> &templateAttributes,int singleElementSize,int numVertexBytes,int numElementBytes)
    : Drawable(name), singleVertexSize(singleVertexSize), vertexAttributes(templateAttributes), singleElementSize(singleElementSize), numVertexBytes(numVertexBytes), numElementBytes(numElementBytes), drawPriority(0), requestZBuffer(false), writeZBuffer(true),
    waitingOnSwap(false), programId(0), elementChunkSize(0), minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid), minVisibleFadeBand(0.0), maxVisibleFadeBand(0.0), enable(true), center(0,0,0), fade(1.0)
{
    activeBuffer = -1;
    transMat = transMat.Identity();
    
    pthread_mutex_init(&useMutex, NULL);
    pthread_cond_init(&useCondition, NULL);
    
    // Start with one region that covers the whole thing
    vertexRegions.insert(Region(0,numVertexBytes));

    buffers[1].numElement = buffers[0].numElement = 0;
}
    
BigDrawable::~BigDrawable()
{
    pthread_mutex_destroy(&useMutex);
    pthread_cond_destroy(&useCondition);
}

bool BigDrawable::isCompatible(BasicDrawable *draw,const Point3d *drawCenter,double objSize)
{
    // Note: We change the big drawable texture IDs without them knowing
//    if (getTexId() == draw->getTexId() &&
    if (getRequestZBuffer() == draw->getRequestZBuffer() &&
        getDrawPriority() == draw->getDrawPriority() && getWriteZbuffer() == draw->getWriteZbuffer())
    {
        float minVis,maxVis,minVisibleFadeBand,maxVisibleFadeBand;
        draw->getVisibleRange(minVis, maxVis, minVisibleFadeBand, maxVisibleFadeBand);
        if (this->minVis == minVis && this->maxVis == maxVis && this->minVisibleFadeBand == minVisibleFadeBand &&
            this->maxVisibleFadeBand == maxVisibleFadeBand)
        {
            // Now check that the centers are compatible
            if (drawCenter)
            {
                // Distance from one center to another
                double dist = (*drawCenter - center).norm();
                double testVal = dist/objSize;
                
                if (testVal < 10e2)
                    return true;
                else
                    return false;
            } else
                return true;
        }
    }
    
    return false;
}

void BigDrawable::setModes(BasicDrawable *draw)
{
    texInfo = draw->getTexInfo();
    requestZBuffer = draw->getRequestZBuffer();
    writeZBuffer = draw->getWriteZbuffer();
    drawPriority = draw->getDrawPriority();
    draw->getVisibleRange(minVis, maxVis, minVisibleFadeBand, maxVisibleFadeBand);
}

const Eigen::Matrix4d *BigDrawable::getMatrix() const
{
    if (center.x() == 0.0 && center.y() == 0.0 && center.z() == 0.0)
        return NULL;
    
    return &transMat;
}
    
void BigDrawable::setCenter(const Point3d &newCenter)
{
    center = newCenter;
    Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
    transMat = trans.matrix();
}

void BigDrawable::setTexID(unsigned int which,SimpleIdentity texId)
{
    if (which < texInfo.size())
        texInfo[which].texId = texId;
}
    
void BigDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager)
{
    if (buffers[0].vertexBufferId)
        return;
    
    for (unsigned int ii=0;ii<2;ii++)
    {
        Buffer &theBuffer = buffers[ii];
        theBuffer.vertexBufferId = memManager->getBufferID(numVertexBytes,GL_DYNAMIC_DRAW);
        theBuffer.elementBufferId = memManager->getBufferID(numElementBytes,GL_DYNAMIC_DRAW);
        
        // Note: Clearing out the buffers will reduce warnings from Instruments
        // Clear out the vertex buffer
//        glBindBuffer(GL_ARRAY_BUFFER, theBuffer.vertexBufferId);
//        void *glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
//        memset(glMem, 0, numVertexBytes);
//        glUnmapBufferOES(GL_ARRAY_BUFFER);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Clear out the element buffer
//        glBindBuffer(GL_ARRAY_BUFFER, theBuffer.elementBufferId);
//        glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
//        memset(glMem, 0, numElementBytes);
//        glUnmapBufferOES(GL_ARRAY_BUFFER);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    activeBuffer = 0;
}
    
void BigDrawable::teardownGL(OpenGLMemManager *memManager)
{
    for (unsigned int ii=0;ii<2;ii++)
    {
        Buffer &theBuffer = buffers[ii];
        if (theBuffer.vertexBufferId)
            memManager->removeBufferID(theBuffer.vertexBufferId);
        if (theBuffer.elementBufferId)
            memManager->removeBufferID(theBuffer.elementBufferId);
        theBuffer.vertexBufferId = 0;
        theBuffer.elementBufferId = 0;
        theBuffer.changes.clear();
        if (theBuffer.vertexArrayObj)
            glDeleteVertexArrays(1,&buffers[ii].vertexArrayObj);
        theBuffer.vertexArrayObj = 0;
    }
}
    
void BigDrawable::updateRenderer(WhirlyKit::SceneRendererES *renderer)
{
    // Let's pull the default shaders out if need be
    if (programId == EmptyIdentity)
    {
        programId = (int)renderer->getScene()->getProgramIDBySceneName(kSceneDefaultTriShader);
    }
}

bool BigDrawable::isOn(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    if (!enable)
        return false;
    
    if (minVis == DrawVisibleInvalid)
        return true;
    
    float visVal = frameInfo->theView->heightAboveSurface();
    
    return ((minVis <= visVal && visVal <= maxVis) ||
            (minVis <= visVal && visVal <= maxVis));
}
    
void BigDrawable::setOnOff(bool onOff)
{
    enable = onOff;
}
    
// Used to pass in buffer offsets
#define CALCBUFOFF(base,off) ((char *)(base) + (off))

void BigDrawable::draw(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene)
{
#ifdef __ANDROID__
//    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "BigDrawable::draw() Drawing buffer %d for bigDraw %lx.",activeBuffer,(long)this);
#endif
  
    if (frameInfo->oglVersion < 2)
        return;
    
    Buffer &theBuffer = buffers[activeBuffer];
    if (theBuffer.numElement <= 0)
        return;
    
    OpenGLES2Program *prog = frameInfo->program;
    
    // GL Texture IDs
    bool anyTextures = false;
    std::vector<GLuint> glTexIDs;
    for (unsigned int ii=0;ii<texInfo.size();ii++)
    {
        const BasicDrawable::TexInfo &thisTexInfo = texInfo[ii];
        GLuint glTexID = 0;
        if (thisTexInfo.texId != EmptyIdentity)
        {
            glTexID = scene->getGLTexture(thisTexInfo.texId);
//            if (!glTexID)
//                NSLog(@"Missing texture");
            anyTextures = true;
        }
        glTexIDs.push_back(glTexID);
    }
    
    if (!prog)
        return;

    float theFade = fade;

    // Only range based fade on the big drawables
    if (frameInfo->heightAboveSurface > 0.0)
    {
        float factor = 1.0;
        if (minVisibleFadeBand != 0.0)
        {
            float a = (frameInfo->heightAboveSurface - minVis)/minVisibleFadeBand;
            if (a >= 0.0 && a < 1.0)
                factor = a;
        }
        if (maxVisibleFadeBand != 0.0)
        {
            float b = (maxVis - frameInfo->heightAboveSurface)/maxVisibleFadeBand;
            if (b >= 0.0 && b < 1.0)
                factor = b;
        }
        
        theFade = theFade * factor;
    }
    
    // If it's totally faded out, don't waste the rendering
    if (theFade <= 0.0)
        return;

    // Model/View/Projection matrix
    prog->setUniform("u_mvpMatrix", frameInfo->mvpMat);
    prog->setUniform("u_mvMatrix", frameInfo->viewAndModelMat);
    prog->setUniform("u_mvNormalMatrix", frameInfo->viewModelNormalMat);
    
    // Fill the a_singleMatrix attribute with default values
    const OpenGLESAttribute *matAttr = prog->findAttribute("a_singleMatrix");
    if (matAttr)
    {
        glVertexAttrib4f(matAttr->index,1.0,0.0,0.0,0.0);
        glVertexAttrib4f(matAttr->index+1,0.0,1.0,0.0,0.0);
        glVertexAttrib4f(matAttr->index+2,0.0,0.0,1.0,0.0);
        glVertexAttrib4f(matAttr->index+3,0.0,0.0,0.0,1.0);
    }
    
    // Fade is always mixed in
    prog->setUniform("u_fade", theFade);
    
    // Let the shaders know if we even have a texture
    prog->setUniform("u_hasTexture", anyTextures);

    // The program itself may have some textures to bind
    bool hasTexture[WhirlyKitMaxTextures];
    int progTexBound = prog->bindTextures();
    for (unsigned int ii=0;ii<progTexBound;ii++)
        hasTexture[ii] = true;
    
    // Zero or more textures.
    for (unsigned int ii=0;ii<WhirlyKitMaxTextures-progTexBound;ii++)
    {
        GLuint glTexID = ii < glTexIDs.size() ? glTexIDs[ii] : 0;
        char baseMapName[40];
        sprintf(baseMapName,"s_baseMap%d",ii);
        const OpenGLESUniform *texUni = prog->findUniform(baseMapName);
        hasTexture[ii+progTexBound] = glTexID != 0 && texUni;
        if (hasTexture[ii+progTexBound])
        {
            frameInfo->stateOpt->setActiveTexture(GL_TEXTURE0+ii+progTexBound);
            glBindTexture(GL_TEXTURE_2D, glTexID);
            CheckGLError("BasicDrawable::drawVBO2() glBindTexture");
            prog->setUniform(baseMapName, (int)ii+progTexBound);
            CheckGLError("BasicDrawable::drawVBO2() glUniform1i");
        }
    }

    // Figure out what we're using
    const OpenGLESAttribute *vertAttr = prog->findAttribute("a_position");
        
    // Set up a VAO for this buffer, if there isn't one
    const OpenGLESAttribute *progAttrs[vertexAttributes.size()];
    if (!hasVertexArraySupport || theBuffer.vertexArrayObj == 0)
    {
        if (hasVertexArraySupport)
        {
            glGenVertexArrays(1,&theBuffer.vertexArrayObj);
            glBindVertexArray(theBuffer.vertexArrayObj);
            CheckGLError("BasicDrawable::drawVBO2() glBindVertexArrayOES");
        }

        glBindBuffer(GL_ARRAY_BUFFER,theBuffer.vertexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theBuffer.elementBufferId);
        CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");

        // Vertex array
        if (vertAttr)
        {
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, singleVertexSize, 0);
            glEnableVertexAttribArray ( vertAttr->index );
            CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
        }
        
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        {
            progAttrs[ii] = NULL;
            VertexAttribute &attr = vertexAttributes[ii];
            const OpenGLESAttribute *progAttr = prog->findAttribute(attr.name);
            // The program is looking for this one, so we need to set up something
            if (progAttr)
            {
                // We have data for it in the interleaved vertices
                if (attr.buffer != 0)
                {
                    progAttrs[ii] = progAttr;
                    glVertexAttribPointer(progAttr->index, attr.glEntryComponents(), attr.glType(), attr.glNormalize(), singleVertexSize, CALCBUFOFF(0, attr.buffer));
                    glEnableVertexAttribArray(progAttr->index);
                    CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
                }
            }
        }
        
        if (hasVertexArraySupport)
        {
            glBindVertexArray(0);
            CheckGLError("BasicDrawable::drawVBO2() glBindVertexArrayOES(0)");
        }
        
        // Let a subclass set up their own VAO state
        setupAdditionalVAO(prog,theBuffer.vertexArrayObj);

        if (hasVertexArraySupport)
        {
            // Tear it all down
            if (vertAttr)
                glDisableVertexAttribArray(vertAttr->index);
            for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
                if (progAttrs[ii])
                    glDisableVertexAttribArray(progAttrs[ii]->index);
            
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
        }
    }
    
    // For the program attributes that we're not filling in, we need to provide defaults
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        VertexAttribute &attr = vertexAttributes[ii];
        const OpenGLESAttribute *progAttr = prog->findAttribute(attr.name);
        if (progAttr && attr.buffer == 0)
            attr.glSetDefault(progAttr->index);
    }
    
    // Draw it
    if (hasVertexArraySupport)
    {
    	glBindVertexArray(theBuffer.vertexArrayObj);
        CheckGLError("BasicDrawable::drawVBO2() glBindVertexArrayOES");
    }
    if (theBuffer.numElement != 0)
        glDrawElements(GL_TRIANGLES, theBuffer.numElement, GL_UNSIGNED_SHORT, 0);
    if (hasVertexArraySupport)
    	glBindVertexArray(0);
    
    // Unbind any texture
    for (unsigned int ii=0;ii<WhirlyKitMaxTextures;ii++)
        if (hasTexture[ii])
        {
            frameInfo->stateOpt->setActiveTexture(GL_TEXTURE0+ii);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    
    // Tear it all down
    if (!hasVertexArraySupport)
    {
        if (vertAttr)
            glDisableVertexAttribArray(vertAttr->index);
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
            if (progAttrs[ii])
                glDisableVertexAttribArray(progAttrs[ii]->index);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
    
SimpleIdentity BigDrawable::addRegion(RawDataRef vertData,int &vertPos,RawDataRef elementData,bool enabled)
{
    size_t vertexSize = vertData->getLen();
    size_t elementSize = elementData->getLen();
    
    // Make sure there's room for the elements
    if (elementSize + elementChunkSize >= numElementBytes)
        return EmptyIdentity;
    
    // Let's look for a region large enough to contain the new vertices
    RegionSet::iterator vrit;
    for (vrit = vertexRegions.begin(); vrit != vertexRegions.end(); ++vrit)
    {
        if ((*vrit).len >= vertexSize)
        {
            break;
        }
    }
    // Not enough room
    if (vrit == vertexRegions.end())
        return EmptyIdentity;

    // Get rid of the old vertex region
    {
        Region theRegion = *vrit;
        vertexRegions.erase(vrit);

        // Set up the change and let the caller know where it wound up
        vertPos = theRegion.pos;
        
        // Split up the remaining space, if there is any
        Region newRegion((int)(theRegion.pos + vertexSize),(int)(theRegion.len - vertexSize));
        if (newRegion.len > 0)
            vertexRegions.insert(newRegion);
    }

    // Set up the vertex buffer change for processing later
    ChangeRef change (new Change(ChangeAdd,vertPos,vertData));
    for (unsigned int ii=0;ii<2;ii++)
        buffers[ii].changes.push_back(change);

    // We know the element data needs to be offset from the position, so let's do that
    int vertOffset = vertPos/singleVertexSize;
    int maxVert = 0;
    if (singleElementSize == sizeof(GLushort))
    {
        GLushort *elPtr = (GLushort *)elementData->getRawData();
        for (unsigned int ii=0;ii<elementSize/2;ii++,elPtr++)
        {
            *elPtr += vertOffset;
            maxVert = std::max((int)*elPtr,maxVert);
        }
    } else {
        GLuint *elPtr = (GLuint *)elementData->getRawData();
        for (unsigned int ii=0;ii<elementSize/4;ii++,elPtr++)
        {
            *elPtr += vertOffset;
            maxVert = std::max((int)*elPtr,maxVert);
        }
    }

    // Toss the element chunk into the set.  It'll be dealt with during the next flush
    ElementChunk elementChunk(elementData);
    elementChunk.enabled = enabled;
    elementChunks.insert(elementChunk);
    elementChunkSize += elementSize;
    
    return elementChunk.getId();
}
 
void BigDrawable::setEnableRegion(SimpleIdentity elementChunkId, bool enabled)
{
    ElementChunkSet::iterator it = elementChunks.find(ElementChunk(elementChunkId));
    if (it == elementChunks.end())
        return;
    
    ElementChunk theChunk(*it);
    elementChunks.erase(it);
    theChunk.enabled = enabled;
    elementChunks.insert(theChunk);
    
    // We just want to force an element rebuild
    RawDataRef emptyData;
    ChangeRef change (new Change(ChangeElements,0,emptyData));
    for (unsigned int ii=0;ii<2;ii++)
        buffers[ii].changes.push_back(change);
}
 
void BigDrawable::removeRegion(RegionSet &regions,int pos,int size)
{
//    NSLog(@"Removing vertex region (pos,size) = (%d,%d), existing regions = %ld",pos,size,regions.size());
    
    // Now look for where to put the region back
    Region thisRegion(pos,size);
    
    if (regions.empty())
    {
        regions.insert(thisRegion);
        return;
    }
    
    RegionSet::iterator prevIt = regions.end();
    RegionSet::iterator nextIt = regions.begin();
    while (nextIt->pos < thisRegion.pos && nextIt != regions.end())
    {
        prevIt = nextIt;
        nextIt++;
    }
    
    // Possibly merge with previous region
    if (prevIt != regions.end())
    {
        const Region &prevRegion = *prevIt;
        if (prevRegion.pos + prevRegion.len == thisRegion.pos)
        {
            thisRegion.pos = prevRegion.pos;
            thisRegion.len = thisRegion.len + prevRegion.len;
            regions.erase(prevIt);
        }
    }
    // Possibly merge with the next region
    if (nextIt != regions.end())
    {
        const Region &nextRegion = *nextIt;
        if (thisRegion.pos + thisRegion.len == nextRegion.pos)
        {
            thisRegion.len = thisRegion.len + nextRegion.len;
            regions.erase(nextIt);
        }
    }
    
    regions.insert(thisRegion);
}

void BigDrawable::clearRegion(int vertPos,int vertSize,SimpleIdentity elementChunkId)
{
    if (vertPos+vertSize > numVertexBytes)
        return;

    // Set up the change in the buffers
    // Note: Don't actually need to clear out the data, just reuse the space
//    void *emptyBytes = malloc(vertSize);
//    memset(emptyBytes, 0, vertSize);
//    NSData *emptyVerts = [[NSData alloc] initWithBytesNoCopy:emptyBytes length:vertSize freeWhenDone:YES];
//    ChangeRef change(new Change(ChangeClear,vertPos,emptyVerts,vertSize));
//    for (unsigned int ii=0;ii<2;ii++)
//        buffers[ii].changes.push_back(change);

    removeRegion(vertexRegions,vertPos,vertSize);

    // Remove the element chunk.  The next flush will pick it up.
    ElementChunkSet::iterator it = elementChunks.find(ElementChunk(elementChunkId));
    if (it != elementChunks.end())
    {
        elementChunkSize -= it->elementData->getLen();
        elementChunks.erase(it);
    } else {
//        NSLog(@"BigDrawable: Found rogue element chunk.");
    }
    
    // This would be weird
    if (elementChunkSize < 0)
        elementChunkSize = 0;
}

void BigDrawable::getUtilization(int &vertSize,int &elSize)
{
    vertSize = numVertexBytes;
    elSize = elementChunkSize;
    for (RegionSet::iterator it = vertexRegions.begin();
         it != vertexRegions.end(); ++it)
    {
        vertSize -= it->len;
    }
}

void BigDrawable::executeFlush(int whichBuffer)
{    
    Buffer &theBuffer = buffers[whichBuffer];
    
#ifdef __ANDROID__
//    __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "BigDrawable::executeFlush() Executing flush %d for bigDraw %lx.",whichBuffer,(long)this);
#endif

    
    if (!theBuffer.changes.empty())
    {
        // Run the additions to the vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, theBuffer.vertexBufferId);
        for (unsigned int ii=0;ii<theBuffer.changes.size();ii++)
        {
            ChangeRef change = theBuffer.changes[ii];
            switch (change->type)
            {
                case ChangeAdd:
                    glBufferSubData(GL_ARRAY_BUFFER, change->whereVert, change->vertData->getLen(), change->vertData->getRawData());
//                    if (change->whereVert + [change->vertData length] > numVertexBytes)
//                    {
//                        NSLog(@"Exceeded vertex buffer size");
//                    }
//                    if (change->whereVert % singleVertexSize != 0)
//                    {
//                        NSLog(@"Offset problem in vertex buffer");
//                    }
                    break;
                case ChangeClear:
                    // We don't really need to clear this, just stop using it
//                    glBufferSubData(GL_ARRAY_BUFFER, change->whereVert, [change->vertData length], [change->vertData bytes]);
//                    if (change->whereVert + [change->vertData length] > numVertexBytes)
//                    {
//                        NSLog(@"Exceeded vertex buffer size");
//                    }
                    break;
                case ChangeElements:
                    // This is just here to nudge the element rebuild
                    break;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        theBuffer.changes.clear();
    }

    // Redo the entire element buffer
    // Note: OpenGL ES 3 has map buffer support.  Add that back
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theBuffer.elementBufferId);
    int elBufferSize = 0;
    if (hasMapBufferSupport)
    {
        // Note: Porting
//        GLubyte *elBuffer = (GLubyte *)glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
//        GLubyte *elBufPtr = elBuffer;
//        for (ElementChunkSet::iterator it = elementChunks.begin();
//             it != elementChunks.end(); ++it)
//            if (it->enabled)
//        {
//            size_t len = it->elementData->getLen();
//            memcpy(elBufPtr, it->elementData->getRawData(), len);
//            elBufPtr += len;
//            elBufferSize += len;
//        }
//        if (elBufPtr - elBuffer > numElementBytes)
//        {
//    //        NSLog(@"Exceeded element buffer size");
//        }
//        glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER);
    } else {
        // Copy the data in with subData calls
        for (ElementChunkSet::iterator it = elementChunks.begin();
             it != elementChunks.end(); ++it)
            if (it->enabled)
            {
                size_t len = it->elementData->getLen();
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, elBufferSize, len, it->elementData->getRawData());
                elBufferSize += len;
            }
        
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    theBuffer.numElement = elBufferSize / singleElementSize;
}
    
// If set, we'll do the flushes on the main thread
static const bool MainThreadFlush = false;
    
void BigDrawable::swap(ChangeSet &changes,BigDrawableSwap *swapRequest)
{
    // If we're waiting on a swap, no flushing
    if (isWaitingOnSwap())
    {
#ifdef __ANDROID__
//        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "BigDrawable::swap() called while waiting on something.");
#endif
//        NSLog(@"Uh oh, tried to swap while we're waiting on something.");
        return;
    }
    
    // Figure out which is the inactive buffer.
    // That's the one we modify
    int whichBuffer = (activeBuffer == 0 ? 1 : 0);
    
    // Note: In theory we shouldn't need to swap if there are no changes.
    //       However, this doesn't work right if we turn on this optimization.
    // No changes, no work
//    Buffer &theBuffer = buffers[whichBuffer];
//    if (theBuffer.changes.empty())
//        return;
    
    executeFlush(whichBuffer);
    
    // Ask the renderer to swap buffers
    pthread_mutex_lock(&useMutex);
    waitingOnSwap = true;
    pthread_mutex_unlock(&useMutex);
    swapRequest->addSwap(getId(), whichBuffer);
}
    
bool BigDrawable::hasChanges()
{
    return !buffers[0].changes.empty() || !buffers[1].changes.empty();
}
    
bool BigDrawable::isWaitingOnSwap()
{
    bool ret = false;
    pthread_mutex_lock(&useMutex);
    ret = waitingOnSwap;
    pthread_mutex_unlock(&useMutex);
    return ret;
}
    
bool BigDrawable::empty()
{
    return elementChunks.empty();
}

void BigDrawable::swapBuffers(int whichBuffer)
{
    pthread_mutex_lock(&useMutex);
    activeBuffer = whichBuffer;
    waitingOnSwap = false;
    pthread_mutex_unlock(&useMutex);

    // Let's tear down the VAO's for both buffers
    for (unsigned int ii=0;ii<2;ii++)
    {
        Buffer &theBuffer = buffers[ii];
        if (theBuffer.vertexArrayObj)
        {
            glDeleteVertexArrays(1,&theBuffer.vertexArrayObj);
            theBuffer.vertexArrayObj = 0;
        }

        // Bind the buffer to get any new updates
        // Note: Don't think we need this
        glBindBuffer(GL_ARRAY_BUFFER, theBuffer.vertexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theBuffer.elementBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

// Called in the renderer
void BigDrawableSwap::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    for (unsigned int ii=0;ii<swaps.size();ii++)
    {
        SwapInfo &swap = swaps[ii];

        DrawableRef draw = scene->getDrawable(swap.drawId);
        BigDrawableRef bigDraw = std::dynamic_pointer_cast<BigDrawable>(draw);
        if (bigDraw) {
            bigDraw->swapBuffers(swap.whichBuffer);
//            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "BigDrawableSwap swapping to buffer %d.",swap.whichBuffer);
        } else {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "BigDrawableSwap called with missing big drawable.");
#endif
        }
    }
    
    // And let the target know we're done
    if (swapCallback)
    {
        (*swapCallback)(this,swapData);
    }
}
    
void BigDrawableFlush::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    DrawableRef draw = scene->getDrawable(drawId);
    BigDrawableRef bigDraw = std::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
    {
        int whichBuffer = (bigDraw->getActiveBuffer() == 0 ? 1 : 0);
        bigDraw->executeFlush(whichBuffer);
    }
}
    
void BigDrawableTexChangeRequest::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    DrawableRef draw = scene->getDrawable(drawId);
    BigDrawableRef bigDraw = std::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
        bigDraw->setTexID(which,texId);
}

void BigDrawableOnOffChangeRequest::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    DrawableRef draw = scene->getDrawable(drawId);
    BigDrawableRef bigDraw = std::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
        bigDraw->setOnOff(enable);
}

void BigDrawableDrawPriorityChangeRequest::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    DrawableRef draw = scene->getDrawable(drawId);
    BigDrawableRef bigDraw = std::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
        bigDraw->setDrawPriority(drawPriority);
}

void BigDrawableFadeChangeRequest::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    DrawableRef draw = scene->getDrawable(drawID);
    BigDrawableRef bigDraw = std::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
        bigDraw->setFade(fade);
}

void BigDrawableProgramIDChangeRequest::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    DrawableRef draw = scene->getDrawable(drawId);
    BigDrawableRef bigDraw = std::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
        bigDraw->setProgram(programID);
}
    
}
