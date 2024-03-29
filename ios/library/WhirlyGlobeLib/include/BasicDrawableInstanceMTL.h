/*
 *  BasicDrawableInstanceMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
 *  Copyright 2011-2022 mousebird consulting
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

#import "BasicDrawableInstance.h"
#import "BasicDrawableMTL.h"
#import "WrapperMTL.h"

namespace WhirlyKit
{
    
/// Metal variant of BasicDrawableInstance
class BasicDrawableInstanceMTL : virtual public BasicDrawableInstance, virtual public DrawableMTL
{
    friend class BasicDrawableInstanceBuilderMTL;
public:
    BasicDrawableInstanceMTL(const std::string &name);
    virtual ~BasicDrawableInstanceMTL();
    
    // Color can change after setup
    virtual void setColor(RGBAColor inColor);
    
    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo,Scene *scene);
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene,RenderTeardownInfoRef teardown);
    
    /** An all-purpose pre-render that sets up textures, uniforms and such in preparation for rendering
        Also adds to the list of resources being used by this drawable.
        Both need to be done each frame.
     */
    bool preProcess(SceneRendererMTL *sceneRender,
                    id<MTLCommandBuffer> cmdBuff,
                    id<MTLBlitCommandEncoder> bltEncode,
                    SceneMTL *scene);

    /// List all the resources used by the drawable
    virtual void enumerateResources(RendererFrameInfoMTL *frameInfo,ResourceRefsMTL &resources);
    virtual void enumerateBuffers(ResourceRefsMTL &resources);

    /// Some drawables have a pre-render phase that uses the GPU for calculation
    virtual void encodeDirectCalculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene);

    /// Draw directly, once per frame
    virtual void encodeDirect(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene);
    
    /// Indirect version of calculate encoding.  Called only when things change enough to re-encode.
    API_AVAILABLE(ios(13.0))
    virtual void encodeIndirectCalculate(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget);

    /// Indirect version of regular encoding.  Called only when things change enough to re-encode.
    API_AVAILABLE(ios(13.0))
    virtual void encodeIndirect(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget);

protected:
    // Pipeline render state for the encoder
    id<MTLRenderPipelineState> getRenderPipelineState(SceneRendererMTL *sceneRender,Scene *scene,ProgramMTL *program,RenderTargetMTL *renderTarget,BasicDrawableMTL *basicDrawMTL);
    
    // Pipeline render state for the calculate encoder
    id<MTLRenderPipelineState> getCalcRenderPipelineState(SceneRendererMTL *sceneRender,Scene *scene,ProgramMTL *program,RenderTargetMTL *renderTarget);
    
    // Set up the memory and defaults for the argument buffers (vertex, fragment, calculate)
    void setupArgBuffers(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfo,SceneMTL *scene,BufferBuilderMTL *buffBuild);

    void updateColorDefaultAttr();
    
    bool setupForMTL;
    id<MTLRenderPipelineState> renderState;
    id<MTLRenderPipelineState> calcRenderState;
    std::vector<BasicDrawableMTL::AttributeDefault> defaultAttrs;
    WhirlyKitShader::UniformModelInstance uniMI;
    BufferEntryMTL instBuffer;       // Stores instances
    BufferEntryMTL indirectBuffer;   // Indirect arguments for drawIndexed
    BufferEntryMTL colorBuffer;      // Used when overriding color
    int numInst;                        // Number of instances (if we're using that mode)
    
    BufferEntryMTL mainBuffer;        // We're storing all the bits and pieces in here
    BufferEntryMTL baseMainBuffer;    // Holding reference to the BasicDrawable's buffer too
    ArgBuffContentsMTLRef vertABInfo,fragABInfo;
    bool vertHasTextures,fragHasTextures;
    bool vertHasLighting,fragHasLighting;
    ArgBuffRegularTexturesMTLRef vertTexInfo,fragTexInfo;

    // Textures currently in use
    std::vector< TextureEntryMTL > activeTextures;
};
    
}
