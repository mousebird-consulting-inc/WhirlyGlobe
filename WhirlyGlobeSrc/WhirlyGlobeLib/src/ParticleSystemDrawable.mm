/*
 *  ParticleSystemDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/28/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "ParticleSystemDrawable.h"

namespace WhirlyKit
{

ParticleSystemDrawable::ParticleSystemDrawable(const std::string &name,const SingleVertexAttributeInfoSet &inVertAttrs,int numPoints)
    : Drawable(name), numPoints(numPoints), vertexSize(0)
{
    for (auto attr : inVertAttrs)
    {
        vertexSize += attr.size();
        vertAttrs.push_back(attr);
    }
}
    
void ParticleSystemDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager)
{
    int totalBytes = vertexSize*numPoints;
    pointBuffer = memManager->getBufferID(totalBytes,GL_DYNAMIC_DRAW);
    
    // Zero it out to avoid warnings
    // Note: Don't actually have to do this
    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
    void *glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    memset(glMem, 0, totalBytes);
    glUnmapBufferOES(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystemDrawable::teardownGL(OpenGLMemManager *memManager)
{
    if (pointBuffer)
        memManager->removeBufferID(pointBuffer);
    pointBuffer = 0;
}
    
void ParticleSystemDrawable::addAttributeData(const std::vector<AttributeData> &attrData)
{
    
}

void ParticleSystemDrawable::draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene)
{
    
}
    
}
