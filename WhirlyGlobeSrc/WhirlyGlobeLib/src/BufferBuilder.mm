/*
 *  BufferBuilder.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/21/12.
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

#import "BufferBuilder.h"

namespace WhirlyKit
{

BufferBuilder::BufferBuilder(unsigned int maxBufferSize)
    : maxBufferSize(maxBufferSize), totalSize(0), curBuf(0), curBufLoc(0), allocSoFar(0)
{
}
    
void BufferBuilder::addToTotal(GLuint size)
{
    totalSize += size;
}
    
void BufferBuilder::addToTotal(BasicDrawable *drawable)
{
    int vertices = drawable->getNumPoints() * drawable->singleVertexSize();
    int tris = drawable->getNumTris() * sizeof(BasicDrawable::Triangle);
    addToTotal(vertices+tris);
}
    
// Note: Turn off the clever bits for now. 11/21/12
void BufferBuilder::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager,BasicDrawable *drawable)
{
    int thisSize = drawable->getNumPoints() * drawable->singleVertexSize() + drawable->getNumTris() * sizeof(BasicDrawable::Triangle);
    
    // Note: Disabling the buffer builder
    drawable->setupGL(setupInfo, memManager);
    return;
    
    // New buffer
    if (!curBuf || (curBufLoc + thisSize) > maxBufferSize)
    {
        // Note: This is a bit inefficient
        int left = totalSize - allocSoFar;
        int toAlloc = std::min((int)left,(int)maxBufferSize);
        if (thisSize > toAlloc)
            toAlloc = thisSize;
        curBuf = memManager->getBufferID(toAlloc,GL_STATIC_DRAW);
        buffers.push_back(curBuf);
        allocSoFar += toAlloc;
        curBufLoc = 0;
    }
    drawable->setupGL(setupInfo, memManager, curBuf, curBufLoc);
    curBufLoc += thisSize;
}
    
}
