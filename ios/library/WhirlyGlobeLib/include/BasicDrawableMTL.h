/*
 *  BasicDrawableMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "BasicDrawable.h"
#import "WrapperMTL.h"
#import "VertexAttributeMTL.h"
#import "SceneRendererMTL.h"
#import "DefaultShadersMTL.h"

namespace WhirlyKit
{
    
// Block of data to be passed into a given buffer ID
// We do this rather than setting individual uniforms
class UniformBlockMTL
{
public:
    int bufferID;
    RawDataRef blockData;
};
    
/** Metal Version of the BasicDrawable.
 */
class BasicDrawableMTL : public BasicDrawable
{
public:
    BasicDrawableMTL(const std::string &name);
    
    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo);
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo);
    
    /// Fill this in to draw the basic drawable
    /// Note: Make this GL only
    virtual void draw(RendererFrameInfo *frameInfo,Scene *scene);
    
public:
    // Apply a list of uniforms to the draw state
    // We hard wire some of these uniforms into a single buffer
    static void applyUniformsToDrawState(WhirlyKitShader::UniformDrawStateA &drawState,const SingleVertexAttributeSet &uniforms);
    
    // Encode the uniform blocks into the given frame
    static void encodeUniBlocks(RendererFrameInfoMTL *frameInfo,const std::vector<BasicDrawable::UniformBlock> &uniBlocks);
    
    // Defaults for vertex attributes we don't have
    typedef struct {
        MTLDataType dataType;
        union {
            float fVals[4];
            int iVal;
            unsigned char chars[4];
        } data;
        int bufferIndex;
    } AttributeDefault;

    float calcFade(RendererFrameInfo *frameInfo);
    MTLVertexDescriptor *getVertexDescriptor(id<MTLFunction> vertFunc,std::vector<AttributeDefault> &defAttrs);
    id<MTLRenderPipelineState> getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo);

    bool setupForMTL;
    std::vector<Triangle> tris;
    int numPts,numTris;
    id<MTLRenderPipelineState> renderState; // Cacheable render state
    MTLVertexDescriptor *vertDesc;     // Description of vertices
    id<MTLBuffer> triBuffer;           // Metal side buffer for triangles
    std::vector<AttributeDefault> defaultAttrs;
};
    
}
