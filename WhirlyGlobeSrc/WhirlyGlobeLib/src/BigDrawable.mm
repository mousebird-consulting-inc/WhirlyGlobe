/*
 *  BigDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/6/13.
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

#import "BigDrawable.h"
#import "Scene.h"
#import "SceneRendererES2.h"
#import "GLUtils.h"

namespace WhirlyKit
{
    
BigDrawable::Change::Change(ChangeType type,int whereVert,NSData *vertData,int clearLen)
    : type(type), whereVert(whereVert), vertData(vertData), clearLen(clearLen)
{
}
    
BigDrawable::Buffer::Buffer()
    : vertexBufferId(0), elementBufferId(0), numElement(0), vertexArrayObj(0)
{
}

BigDrawable::BigDrawable(const std::string &name,int singleVertexSize,int singleElementSize,int numVertexBytes,int numElementBytes)
    : Drawable(name), singleVertexSize(singleVertexSize), singleElementSize(singleElementSize), numVertexBytes(numVertexBytes), numElementBytes(numElementBytes), texId(0), drawPriority(0), forceZBuffer(false),
    waitingOnSwap(false), programId(0), elementChunkSize(0)
{
    activeBuffer = -1;
    
    pthread_mutex_init(&useMutex, nil);
    pthread_cond_init(&useCondition, nil);
    
    // Start with one region that covers the whole thing
    vertexRegions.insert(Region(0,numVertexBytes));

    buffers[1].numElement = buffers[0].numElement = 0;
}
    
BigDrawable::~BigDrawable()
{
    pthread_mutex_destroy(&useMutex);
    pthread_cond_destroy(&useCondition);
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

        // Clear out the vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, theBuffer.vertexBufferId);
        void *glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
        memset(glMem, 0, numVertexBytes);
        glUnmapBufferOES(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Clear out the element buffer
        glBindBuffer(GL_ARRAY_BUFFER, theBuffer.elementBufferId);
        glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
        memset(glMem, 0, numElementBytes);
        glUnmapBufferOES(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
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
            glDeleteVertexArraysOES(1,&buffers[ii].vertexArrayObj);
        theBuffer.vertexArrayObj = 0;
    }
}
    
void BigDrawable::updateRenderer(WhirlyKitSceneRendererES *renderer)
{
    // Let's pull the default shaders out if need be
    if (programId == EmptyIdentity)
    {
        SimpleIdentity triShaderId,lineShaderId;
        renderer.scene->getDefaultProgramIDs(triShaderId,lineShaderId);
        programId = triShaderId;
    }
}
    
// Used to pass in buffer offsets
#define CALCBUFOFF(base,off) ((char *)(base) + (off))

void BigDrawable::draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene)
{
    if (frameInfo.oglVersion < kEAGLRenderingAPIOpenGLES2)
        return;
    
    Buffer &theBuffer = buffers[activeBuffer];
    if (theBuffer.numElement <= 0)
        return;
    
    OpenGLES2Program *prog = frameInfo.program;
    
    // GL Texture ID
    GLuint glTexID = 0;
    if (texId != EmptyIdentity)
        glTexID = scene->getGLTexture(texId);
    
    if (!prog)
        return;

    // Model/View/Projection matrix
    prog->setUniform("u_mvpMatrix", frameInfo.mvpMat);
    
    // Fade is always mixed in
    float fade = 1.0;
    prog->setUniform("u_fade", fade);
    
    // Let the shaders know if we even have a texture
    prog->setUniform("u_hasTexture", (glTexID != 0));

    // Texture
    const OpenGLESUniform *texUni = prog->findUniform("s_baseMap");
    bool hasTexture = glTexID != 0 && texUni;
    if (hasTexture)
    {
        [frameInfo.stateOpt setActiveTexture:GL_TEXTURE0];
        glBindTexture(GL_TEXTURE_2D, glTexID);
        CheckGLError("BigDrawable::draw() glBindTexture");
        prog->setUniform("s_baseMap", 0);
        CheckGLError("BigDrawable::draw() glUniform1i");
    }

    // Figure out what we're using
    const OpenGLESAttribute *vertAttr = prog->findAttribute("a_position");
    const OpenGLESAttribute *colorAttr = prog->findAttribute("a_color");
    bool hasColors = true;
    int colorOffset = 3*sizeof(float);
    const OpenGLESAttribute *texAttr = prog->findAttribute("a_texCoord");
    bool hasTexCoords = true;
    int texCoordOffset = colorOffset+ 4*sizeof(unsigned char);
    const OpenGLESAttribute *normAttr = prog->findAttribute("a_normal");
    bool hasNormals = true;
    int normOffset = texCoordOffset + 2*sizeof(float);
        
    // Set up a VAO for this buffer, if there isn't one
    if (theBuffer.vertexArrayObj == 0)
    {
//        glGenVertexArraysOES(1,&theBuffer.vertexArrayObj);
//        glBindVertexArrayOES(theBuffer.vertexArrayObj);

        glBindBuffer(GL_ARRAY_BUFFER,theBuffer.vertexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theBuffer.elementBufferId);

        // Vertex array
        if (vertAttr)
        {
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, singleVertexSize, 0);
            glEnableVertexAttribArray ( vertAttr->index );
        }
        
        // Texture coordinates
        if (texAttr && hasTexCoords)
        {
            glVertexAttribPointer(texAttr->index, 2, GL_FLOAT, GL_FALSE, singleVertexSize, CALCBUFOFF(0,texCoordOffset));
            glEnableVertexAttribArray ( texAttr->index );
        }
        
        // Per vertex colors
        if (colorAttr && hasColors)
        {
            glVertexAttribPointer(colorAttr->index, 4, GL_UNSIGNED_BYTE, GL_TRUE, singleVertexSize, CALCBUFOFF(0,colorOffset));
            glEnableVertexAttribArray(colorAttr->index);
        }
        
        // Per vertex normals
        if (normAttr && hasNormals)
        {
            glVertexAttribPointer(normAttr->index, 3, GL_FLOAT, GL_FALSE, singleVertexSize, CALCBUFOFF(0,normOffset));
            glEnableVertexAttribArray(normAttr->index);
        }
        
//        glBindVertexArrayOES(0);
        glDrawElements(GL_TRIANGLES, theBuffer.numElement, GL_UNSIGNED_SHORT, 0);

        // Tear it all down
        if (vertAttr)
            glDisableVertexAttribArray(vertAttr->index);
        if (texAttr && hasTexCoords)
            glDisableVertexAttribArray(texAttr->index);
        if (colorAttr && hasColors)
            glDisableVertexAttribArray(colorAttr->index);
        if (normAttr && hasNormals)
            glDisableVertexAttribArray(normAttr->index);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
    // Draw it
//    glBindVertexArrayOES(theBuffer.vertexArrayObj);
//    glDrawElements(GL_TRIANGLES, theBuffer.numElement, GL_UNSIGNED_SHORT, 0);
//    glBindVertexArrayOES(0);
    
    if (hasTexture)
        glBindTexture(GL_TEXTURE_2D, 0);
}
    
SimpleIdentity BigDrawable::addRegion(NSMutableData *vertData,int &vertPos,NSMutableData *elementData)
{
    size_t vertexSize = [vertData length];
    size_t elementSize = [elementData length];
    
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
        Region newRegion(theRegion.pos + vertexSize,theRegion.len - vertexSize);
        if (newRegion.len > 0)
            vertexRegions.insert(newRegion);
    }

    // Set up the vertex buffer change for processing later
    ChangeRef change (new Change(ChangeAdd,vertPos,vertData));
    for (unsigned int ii=0;ii<2;ii++)
        buffers[ii].changes.push_back(change);

    // We know the element data needs to be offset from the position, so let's do that
    int vertOffset = vertPos/singleVertexSize;
    if (singleElementSize == sizeof(GLushort))
    {
        GLushort *elPtr = (GLushort *)[elementData mutableBytes];
        for (unsigned int ii=0;ii<elementSize/2;ii++,elPtr++)
            *elPtr += vertOffset;
    } else {
        GLuint *elPtr = (GLuint *)[elementData mutableBytes];
        for (unsigned int ii=0;ii<elementSize/4;ii++,elPtr++)
            *elPtr += vertOffset;
    }

    // Toss the element chunk into the set.  It'll be dealt with during the next flush
    ElementChunk elementChunk(elementData);
    elementChunks.insert(elementChunk);
    elementChunkSize += elementSize;

    return elementChunk.getId();
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
    while (nextIt->pos < thisRegion.pos)
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
    ChangeRef change(new Change(ChangeClear,vertPos,nil,vertSize));
    for (unsigned int ii=0;ii<2;ii++)
        buffers[ii].changes.push_back(change);

    removeRegion(vertexRegions,vertPos,vertSize);

    // Remove the element chunk.  The next flush will pick it up.
    ElementChunkSet::iterator it = elementChunks.find(ElementChunk(elementChunkId));
    if (it != elementChunks.end())
    {
        elementChunkSize -= [it->elementData length];
        elementChunks.erase(it);
    }
    
    // This would be weird
    if (elementChunkSize < 0)
        elementChunkSize = 0;
}

void BigDrawable::executeFlush(int whichBuffer)
{    
    Buffer &theBuffer = buffers[whichBuffer];
    
    if (theBuffer.changes.empty())
        return;

    // Run the additions to the vertex buffer
    // Note: The clears don't do anything
    glBindBuffer(GL_ARRAY_BUFFER, theBuffer.vertexBufferId);
    for (unsigned int ii=0;ii<theBuffer.changes.size();ii++)
    {
        ChangeRef change = theBuffer.changes[ii];
        switch (change->type)
        {
            case ChangeAdd:
                glBufferSubData(GL_ARRAY_BUFFER, change->whereVert, [change->vertData length], [change->vertData bytes]);
                break;
            case ChangeClear:
//                memset(vertBufferPtr + change.whereVert, 0, change.clearLen);
                //                glBufferSubData(GL_ARRAY_BUFFER, change.whereVert, [change.vertData length], [change.vertData bytes]);
                break;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    theBuffer.changes.clear();

    // Redo the entire element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theBuffer.elementBufferId);
    GLubyte *elBuffer = (GLubyte *)glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    GLubyte *elBufPtr = elBuffer;
    for (ElementChunkSet::iterator it = elementChunks.begin();
         it != elementChunks.end(); ++it)
    {
        size_t len = [it->elementData length];
        memcpy(elBufPtr, [it->elementData bytes], len);
        elBufPtr += len;
    }
    glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    theBuffer.numElement = elementChunkSize / singleElementSize;
}
    
// If set, we'll do the flushes on the main thread
static const bool MainThreadFlush = false;
    
void BigDrawable::swap(std::vector<ChangeRequest *> &changes,BigDrawableSwap *swapRequest)
{
    // If we're waiting on a swap, no flushing
    if (isWaitingOnSwap())
    {
        NSLog(@"Uh oh, tried to swap while we're waiting on something.");
        return;
    }
    
    // Figure out which is the inactive buffer.
    // That's the one we modify
    int whichBuffer = (activeBuffer == 0 ? 1 : 0);
    
    // No changes, no work
    Buffer &theBuffer = buffers[whichBuffer];
    if (theBuffer.changes.empty())
        return;
    
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
            glDeleteVertexArraysOES(1,&theBuffer.vertexArrayObj);
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
void BigDrawableSwap::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    for (unsigned int ii=0;ii<swaps.size();ii++)
    {
        SwapInfo &swap = swaps[ii];

        DrawableRef draw = scene->getDrawable(swap.drawId);
        BigDrawableRef bigDraw = boost::dynamic_pointer_cast<BigDrawable>(draw);
        if (bigDraw)
            bigDraw->swapBuffers(swap.whichBuffer);
    }
    
    // And let the target know we're done
    if (target)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
        [target performSelector:sel];
#pragma clang diagnostic pop
    }
}
    
void BigDrawableFlush::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    DrawableRef draw = scene->getDrawable(drawId);
    BigDrawableRef bigDraw = boost::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
    {
        int whichBuffer = (bigDraw->getActiveBuffer() == 0 ? 1 : 0);
        bigDraw->executeFlush(whichBuffer);
    }
}

}
