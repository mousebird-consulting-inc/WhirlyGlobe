/*
 *  RenderTargetMTL.h
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

#import "RenderTarget.h"
#import "WrapperMTL.h"
#import "SceneRendererMTL.h"
#import "TextureMTL.h"
#import <MetalPerformanceShaders/MetalPerformanceShaders.h>

namespace WhirlyKit
{
    class SceneRenderer;
    
/** What and where we're rendering.  This can be a regular framebuffer
 to the screen or to a texture.
 */
class RenderTargetMTL : public RenderTarget
{
public:
    RenderTargetMTL();
    RenderTargetMTL(SimpleIdentity newID);
    virtual ~RenderTargetMTL();
    
    // Set up the render target
    bool init(SceneRenderer *renderer,Scene *scene,SimpleIdentity targetTexID);
    
    // Pull in framebuffer info from the current OpenGL State
    /// Set up the target texture
    virtual bool setTargetTexture(SceneRenderer *renderer,Scene *scene,SimpleIdentity newTargetTexID);

    // Set the clear color
    void setClearColor(const RGBAColor &color);
    
    /// Copy the data out of the destination texture and return it
    virtual RawDataRef snapshot();

    /// Copy just a subset out of the destination texture and return it
    virtual RawDataRef snapshot(int startX,int startY,int snapWidth,int snapHeight);
    
    /// If we've asked for a min/max calculation, this is where we get it
    virtual RawDataRef snapshotMinMax();

    /// Set the texture directly
    void setTargetTexture(TextureBaseMTL *tex);
    
    /// Set a texture for depth directly
    void setTargetDepthTexture(TextureBaseMTL *tex);
        
    /// Encodes any post processing commands
    void addPostProcessing(id<MTLDevice> mtlDevice,id<MTLCommandBuffer> cmdBuff);
    
    /// Release associated resources (not clear the buffer, very confusing)
    virtual void clear();
    
    /// If we're tied to a texture, the number of levels in that texture
    virtual int numLevels();
    
    /// Return the pixel format.  Important for shader output and such.
    MTLPixelFormat getPixelFormat();
    
    /// Called once per frame to build a render pass descriptor
    void makeRenderPassDesc();
    
    /// Get the last render pass descriptor built
    MTLRenderPassDescriptor *getRenderPassDesc(int level=-1);
    
    /// Return the texture reference, if there is one
    id<MTLTexture> getTex();
    
protected:
    id<MTLTexture> tex;
    MTLPixelFormat pixelFormat;
    id<MTLTexture> depthTex;
    MTLPixelFormat depthPixelFormat;
    std::vector<MTLRenderPassDescriptor *> renderPassDesc;
    MPSImagePyramid *mipmapKernel;
    id<MTLTexture> minMaxOutTex;
    API_AVAILABLE(ios(11.0)) MPSImageStatisticsMinAndMax *minMaxKernel;
};
    
}
