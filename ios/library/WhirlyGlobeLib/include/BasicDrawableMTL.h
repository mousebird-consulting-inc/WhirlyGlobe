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
#import "DrawableMTL.h"
#import "VertexAttributeMTL.h"
#import "SceneRendererMTL.h"
#import "DefaultShadersMTL.h"

namespace WhirlyKit
{
        
/** Metal Version of the BasicDrawable.
 */
class BasicDrawableMTL : virtual public BasicDrawable, virtual public DrawableMTL
{
public:
    BasicDrawableMTL(const std::string &name);
    ~BasicDrawableMTL();
    
    /// Tweak the values passed in for the override color
    virtual void setOverrideColor(RGBAColor inColor) override;
    
    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo,Scene *scene) override;
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene,RenderTeardownInfoRef teardown) override;
    
    /** An all-purpose pre-render that sets up textures, uniforms and such in preparation for rendering
        Also adds to the list of resources being used by this drawable.
        Both need to be done each frame.
     */
    bool preProcess(SceneRendererMTL *sceneRender,
                    id<MTLCommandBuffer> cmdBuff,
                    id<MTLBlitCommandEncoder> bltEncode,
                    SceneMTL *scene) override;
    
    /// List all the resources used by the drawable
    virtual void enumerateResources(RendererFrameInfoMTL *frameInfo,ResourceRefsMTL &resources) override;
    virtual void enumerateBuffers(ResourceRefsMTL &resources);

    /// Some drawables have a pre-render phase that uses the GPU for calculation
    virtual void encodeDirectCalculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene) override;

    /// Draw directly, once per frame
    virtual void encodeDirect(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene) override;
    
    /// Indirect version of calculate encoding.  Called only when things change enough to re-encode.
    API_AVAILABLE(ios(13.0))
    virtual void encodeIndirectCalculate(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget) override;

    /// Indirect version of regular encoding.  Called only when things change enough to re-encode.
    API_AVAILABLE(ios(13.0))
    virtual void encodeIndirect(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget) override;
    
    /// Find the vertex attribute corresponding to the given name
    VertexAttributeMTL *findVertexAttribute(int nameID);
    
public:
    // Defaults for vertex attributes we don't have
    typedef struct {
        MTLDataType dataType;
        union {
            float fVals[4];
            int iVal;
            unsigned char chars[4];
        } data;
        int entry;
        int bufferIndex;
        BufferEntryMTL buffer;
    } AttributeDefault;

    // Apply a list of uniforms to the draw state
    // This is for backward compatibility.  Some of the named uniforms are now just in a big struct
    static void applyUniformsToDrawState(WhirlyKitShader::UniformDrawStateA &drawState,const SingleVertexAttributeSet &uniforms);

    // Vertex descriptor lays out the wiring for vertex data
    MTLVertexDescriptor *getVertexDescriptor(id<MTLFunction> vertFunc,std::vector<AttributeDefault> &defAttrs);
    
    // Pipeline render state for the encoder
    id<MTLRenderPipelineState> getRenderPipelineState(SceneRendererMTL *sceneRender,Scene *scene,ProgramMTL *program,RenderTargetMTL *renderTarget);

    // Set up the memory and defaults for the argument buffers (vertex, fragment, calculate)
    void setupArgBuffers(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfo,SceneMTL *scene,BufferBuilderMTL &buffBuild);
        
    bool setupForMTL;
    std::vector<Triangle> tris;
    int numPts,numTris;
    id<MTLRenderPipelineState> renderState; // Cacheable render state
    MTLVertexDescriptor *vertDesc;     // Description of vertices
    BufferEntryMTL triBuffer;           // Metal side buffer for triangles
    std::vector<AttributeDefault> defaultAttrs;
    
    BufferEntryMTL mainBuffer;        // We're storing all the bits and pieces in here
    ArgBuffContentsMTLRef vertABInfo,fragABInfo;
    bool vertHasTextures,fragHasTextures;
    bool vertHasLighting,fragHasLighting;
    ArgBuffRegularTexturesMTLRef vertTexInfo,fragTexInfo;
    
    // Textures currently in use
    std::vector< TextureEntryMTL > activeTextures;
};
typedef std::shared_ptr<BasicDrawableMTL> BasicDrawableMTLRef;
    
}
