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
    
BigDrawable::Change::Change(ChangeType type,int whereVert,NSData *vertData,int whereElement,NSData *elementData)
    : type(type), whereVert(whereVert), vertData(vertData), whereElement(whereElement), elementData(elementData)
{
}
    
BigDrawable::Buffer::Buffer()
    : vertexBufferId(0), elementBufferId(0), numElement(0), vertexArrayObj(0)
{
}

BigDrawable::BigDrawable(const std::string &name,int singleVertexSize,int singleElementSize,int numVertexBytes,int numElementBytes)
    : Drawable(name), singleVertexSize(singleVertexSize), singleElementSize(singleElementSize), numVertexBytes(numVertexBytes), numElementBytes(numElementBytes), texId(0), drawPriority(0), forceZBuffer(false),
    waitingOnSwap(false), programId(0)
{
    activeBuffer = -1;
    
    pthread_mutex_init(&useMutex, nil);
    pthread_cond_init(&useCondition, nil);
    
    // Start with one region that covers the whole thing
    vertexRegions.insert(Region(0,numVertexBytes));
    elementRegions.insert(Region(0,numElementBytes));

    // Note: This could be more clever
    buffers[1].numElement = buffers[0].numElement = numElementBytes / singleElementSize;
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
    
    Buffer &theBuffer = buffers[activeBuffer];
    
    // Set up a VAO for this buffer, if there isn't one
    if (theBuffer.vertexArrayObj == 0)
    {
        glGenVertexArraysOES(1,&theBuffer.vertexArrayObj);
        glBindVertexArrayOES(theBuffer.vertexArrayObj);
    
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
        
        glBindVertexArrayOES(0);

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
    glBindVertexArrayOES(theBuffer.vertexArrayObj);
    glDrawElements(GL_TRIANGLES, theBuffer.numElement, GL_UNSIGNED_SHORT, 0);
    glBindVertexArrayOES(0);
    
    if (hasTexture)
        glBindTexture(GL_TEXTURE_2D, 0);
}
    
bool BigDrawable::addRegion(NSMutableData *vertData,int &vertPos,NSMutableData *elementData,int &elementPos)
{
    size_t vertexSize = [vertData length];
    size_t elementSize = [elementData length];
    
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
        return false;

    // And one large enough to contain the new elements
    RegionSet::iterator erit;
    for (erit = elementRegions.begin(); erit != elementRegions.end(); ++erit)
    {
        if ((*erit).len >= elementSize)
        {
            break;
        }
    }
    // Not enough room
    if (erit == elementRegions.end())
        return false;
    

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

    // Get rid of the old element region
    {
        Region theRegion = *erit;
        elementRegions.erase(erit);
        
        // Set up the change and let the caller know where it wound up
        elementPos = theRegion.pos;
        
        // Split up the remaining space, if there is any
        Region newRegion(theRegion.pos + elementSize,theRegion.len - elementSize);
        if (newRegion.len > 0)
            elementRegions.insert(newRegion);
    }

    // We know the element data needs to be offset from the position, so let's do that
    int vertOffset = vertPos/singleVertexSize;
    if (singleElementSize == sizeof(GLushort))
    {
        GLushort *elPtr = (GLushort *)[elementData mutableBytes];
        for (unsigned int ii=0;ii<elementSize/2;ii++,elPtr++)
            *elPtr += vertOffset;
    } else {
        GLuint *elPtr = (GLuint *)[elementData mutableBytes];
        for (unsigned int ii=0;ii<elementSize/2;ii++,elPtr++)
            *elPtr += vertOffset;      
    }

    // Set up the change for processing later
    Change change(ChangeAdd,vertPos,vertData,elementPos,elementData);
    for (unsigned int ii=0;ii<2;ii++)
        buffers[ii].changes.push_back(change);
    
    return true;
}
 
void BigDrawable::removeRegion(RegionSet &regions,int pos,int size)
{
    // Now look for where to put the region back
    Region thisRegion(pos,size);
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

void BigDrawable::clearRegion(int vertPos,int vertSize,int elementPos,int elementSize)
{
    if (vertPos+vertSize > numVertexBytes ||
        elementPos+elementSize > numElementBytes)
        return;
    
    // Set up the change in the buffers
    unsigned char *zeroData = (unsigned char *)malloc(elementSize);
    memset(zeroData, 0, elementSize);
    NSData *elementData = [[NSData alloc] initWithBytesNoCopy:zeroData length:elementSize freeWhenDone:YES];
    Change change(ChangeClear,vertPos,nil,elementPos,elementData);
    for (unsigned int ii=0;ii<2;ii++)
        buffers[ii].changes.push_back(change);
    
    removeRegion(vertexRegions,vertPos,vertSize);
    removeRegion(elementRegions,elementPos,elementSize);
}
    
void BigDrawable::flush(std::vector<ChangeRequest *> &changes,NSObject * __weak target,SEL sel)
{
    // If we're waiting on a swap, no flushing
    if (isWaitingOnSwap())
        return;

    // Figure out which is the inactive buffer.
    // That's the one we modify
    int whichBuffer = (activeBuffer == 0 ? 1 : 0);
    Buffer &theBuffer = buffers[whichBuffer];
    
    // No changes, no work
    if (theBuffer.changes.empty())
        return;
    
    // Run the additions or clears
    glBindBuffer(GL_ARRAY_BUFFER, theBuffer.vertexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theBuffer.elementBufferId);
    for (unsigned int ii=0;ii<theBuffer.changes.size();ii++)
    {
        Change &change = theBuffer.changes[ii];
        switch (change.type)
        {
            case ChangeAdd:
                glBufferSubData(GL_ARRAY_BUFFER, change.whereVert, [change.vertData length], [change.vertData bytes]);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, change.whereElement, [change.elementData length], [change.elementData bytes]);
                break;
            case ChangeClear:
//                glBufferSubData(GL_ARRAY_BUFFER, change.whereVert, [change.vertData length], [change.vertData bytes]);
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, change.whereElement, [change.elementData length], [change.elementData bytes]);
                break;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    theBuffer.changes.clear();
    
    // Ask the renderer to swap buffers
    pthread_mutex_lock(&useMutex);
    waitingOnSwap = true;
    pthread_mutex_unlock(&useMutex);
    changes.push_back(new BigDrawableSwap(getId(),whichBuffer,target,sel));
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
    
    // Bind the buffer to get any new updates
    glBindBuffer(GL_ARRAY_BUFFER, buffers[activeBuffer].vertexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[activeBuffer].elementBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);    
}

// Called in the renderer
void BigDrawableSwap::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    DrawableRef draw = scene->getDrawable(drawId);
    BigDrawableRef bigDraw = boost::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
        bigDraw->swapBuffers(whichBuffer);
    
    // And let the target know we're done
    if (target)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
        [target performSelector:sel];
#pragma clang diagnostic pop
    }
}

}
