/*
 *  ParticleSystemDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/28/15.
 *  Copyright 2011-2019 mousebird consulting
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

// Maximum size of particle buffers (8MB)
#define kMaxParticleMemory (8*1024*1024)

/// OpenGL ES version of the particle system drawable
class ParticleSystemDrawableGLES : public ParticleSystemDrawable
{
public:
    /// Just points for now
    GLenum getType() const { return GL_POINTS; }

    /// Add the vertex data (all of it) at once
    void addAttributeData(WhirlyKitGLSetupInfo *setupInfo,const std::vector<AttributeData> &attrData,const Batch &batch);

protected:
    class VaryBufferPair {
    public:
        GLuint buffers[2];
    };
    std::vector<VaryBufferPair> varyBuffers;
    
    GLuint pointBuffer,rectBuffer;

    void drawSetupTextures(RendererFrameInfo *frameInfo,Scene *scene,OpenGLES2Program *prog,bool hasTexture[],int &progTexBound);
    void drawTeardownTextures(RendererFrameInfo *frameInfo,Scene *scene,OpenGLES2Program *prog,bool hasTexture[],int progTexBound);
    void drawSetupUniforms(RendererFrameInfo *frameInfo,Scene *scene,OpenGLES2Program *prog);
    void drawBindAttrs(RendererFrameInfo *frameInfo,Scene *scene,OpenGLES2Program *prog,const BufferChunk &chunk,int pointsSoFar,bool useInstancingHere);
    void drawUnbindAttrs(OpenGLES2Program *prog);

}
    
}
