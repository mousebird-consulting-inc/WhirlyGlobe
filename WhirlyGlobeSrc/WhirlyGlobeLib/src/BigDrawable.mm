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
    
BigDrawable::Change::Change(ChangeType type,int where,int len,NSData *data)
    : type(type), where(where), len(len), data(data)
{
}
    
BigDrawable::Buffer::Buffer()
    : bufferId(0), numVertex(0)
{
}

BigDrawable::BigDrawable(const std::string &name,int vertexSize,int numBytes)
    : Drawable(name), vertexSize(vertexSize), numBytes(numBytes), texId(0), drawPriority(0), forceZBuffer(false), waitingOnSwap(false),
    programId(0)
{
    activeBuffer = -1;
    
    pthread_mutex_init(&useMutex, nil);
    pthread_cond_init(&useCondition, nil);
    
    // Start with one region that covers the whole thing
    regions.insert(Region(0,numBytes));
}
    
BigDrawable::~BigDrawable()
{
    pthread_mutex_destroy(&useMutex);
    pthread_cond_destroy(&useCondition);
}
    
void BigDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager)
{
    if (buffers[0].bufferId)
        return;
    
    for (unsigned int ii=0;ii<2;ii++)
    {
        buffers[ii].bufferId = memManager->getBufferID(numBytes,GL_DYNAMIC_DRAW);

        // Clear out the buffer
        glBindBuffer(GL_ARRAY_BUFFER, buffers[ii].bufferId);
        void *glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
        memset(glMem, 0, numBytes);
        glUnmapBufferOES(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Note: Should make this smarter
        buffers[ii].numVertex = numBytes / vertexSize;
    }
    activeBuffer = 0;
}
    
void BigDrawable::teardownGL(OpenGLMemManager *memManager)
{
    for (unsigned int ii=0;ii<2;ii++)
    {
        if (buffers[ii].bufferId)
            memManager->removeBufferID(buffers[ii].bufferId);
        buffers[ii].bufferId = 0;
        buffers[ii].changes.clear();
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
        glActiveTexture(GL_TEXTURE0);
        CheckGLError("BigDrawable::draw() glActiveTexture");
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
    
    glBindBuffer(GL_ARRAY_BUFFER,theBuffer.bufferId);

    // Vertex array
    if (vertAttr)
    {
        glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, 0);
        glEnableVertexAttribArray ( vertAttr->index );
    }
    
    // Texture coordinates
    if (texAttr && hasTexCoords)
    {
        glVertexAttribPointer(texAttr->index, 2, GL_FLOAT, GL_FALSE, vertexSize, CALCBUFOFF(0,texCoordOffset));
        glEnableVertexAttribArray ( texAttr->index );
    }
    
    // Per vertex colors
    if (colorAttr && hasColors)
    {
        glVertexAttribPointer(colorAttr->index, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertexSize, CALCBUFOFF(0,colorOffset));
        glEnableVertexAttribArray(colorAttr->index);
    }
    
    // Per vertex normals
    if (normAttr && hasNormals)
    {
        glVertexAttribPointer(normAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, CALCBUFOFF(0,normOffset));
        glEnableVertexAttribArray(normAttr->index);
    }
    
    // Draw it
    glDrawArrays(GL_TRIANGLE_STRIP, 0, theBuffer.numVertex);
    
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
}
    
bool BigDrawable::addRegion(NSData *data,int &pos,int &size)
{
    size = [data length];
    
    // Let's look for a region large enough to contain the new data
    RegionSet::iterator rit;
    for (rit = regions.begin(); rit != regions.end(); ++rit)
    {
        if ((*rit).len >= size)
        {
            break;
        }
    }
    // Not enough room
    if (rit == regions.end())
        return false;
    
    // Obviously we're getting rid of this region
    Region theRegion = *rit;
    regions.erase(rit);

    // Set up the change and let the caller know where it wound up
    pos = theRegion.pos;
    Change change(ChangeAdd,pos,size,data);
    for (unsigned int ii=0;ii<2;ii++)
        buffers[ii].changes.push_back(change);
    
    // Split up the remaining space, if there is any
    Region newRegion(theRegion.pos + size,theRegion.len - size);
    if (newRegion.len > 0)
        regions.insert(newRegion);
    
    return true;
}
    
void BigDrawable::clearRegion(int pos, int size)
{
    if (pos+size > numBytes)
        return;
    
    // Set up the change in the buffers
    Change change(ChangeClear,pos,size);
    unsigned char *zeroData = (unsigned char *)malloc(size);
    memset(zeroData, 0, size);
    change.data = [[NSData alloc] initWithBytesNoCopy:zeroData length:size freeWhenDone:YES];
    for (unsigned int ii=0;ii<2;ii++)
        buffers[ii].changes.push_back(change);
    
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
    
void BigDrawable::flush(std::vector<ChangeRequest *> &changes)
{
    // If we're waiting on a buffer swap, then wait
    pthread_mutex_lock(&useMutex);
    while (waitingOnSwap)
        pthread_cond_wait(&useCondition, &useMutex);
    pthread_mutex_unlock(&useMutex);

    // Figure out which is the inactive buffer.
    // That's the one we modify
    int whichBuffer = (activeBuffer == 0 ? 1 : 0);
    Buffer &theBuffer = buffers[whichBuffer];
    
    // No changes, no work
    if (theBuffer.changes.empty())
        return;
    
    // Run the additions or clears
    glBindBuffer(GL_ARRAY_BUFFER, theBuffer.bufferId);
//    void *glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    for (unsigned int ii=0;ii<theBuffer.changes.size();ii++)
    {
        Change &change = theBuffer.changes[ii];
//        unsigned char *basePtr = (unsigned char *)glMem + change.where;
        switch (change.type)
        {
            case ChangeAdd:
                glBufferSubData(GL_ARRAY_BUFFER, change.where, change.len, [change.data bytes]);
//                memcpy(basePtr, [change.data bytes], change.len);
                break;
            case ChangeClear:
                glBufferSubData(GL_ARRAY_BUFFER, change.where, change.len, [change.data bytes]);
//                memset(basePtr, 0, change.len);
                break;
        }
    }
//    glUnmapBufferOES(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    theBuffer.changes.clear();
    
    // Ask the renderer to swap buffers
    pthread_mutex_lock(&useMutex);
    waitingOnSwap = true;
    pthread_mutex_unlock(&useMutex);
    changes.push_back(new BigDrawableSwap(getId(),whichBuffer));
}

void BigDrawable::swapBuffers(int whichBuffer)
{
    pthread_mutex_lock(&useMutex);
    activeBuffer = whichBuffer;
    waitingOnSwap = false;
    pthread_cond_signal(&useCondition);
    pthread_mutex_unlock(&useMutex);
}

// Called in the renderer
void BigDrawableSwap::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    DrawableRef draw = scene->getDrawable(drawId);
    BigDrawableRef bigDraw = boost::dynamic_pointer_cast<BigDrawable>(draw);
    if (bigDraw)
        bigDraw->swapBuffers(whichBuffer);
}

}
