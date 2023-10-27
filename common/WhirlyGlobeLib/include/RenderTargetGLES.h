/*  RenderTargetGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/13/19.
 *  Copyright 2011-2023 mousebird consulting
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
 */

#import "RenderTarget.h"
#import "WrapperGLES.h"
#import "SceneRendererGLES.h"

namespace WhirlyKit
{
class SceneRenderer;

/// For passing settings to the scene renderer, where the actual render target is created
struct RenderTargetSettings : public RenderTarget
{
    RenderTargetSettings() = default;
    RenderTargetSettings(SimpleIdentity id) : RenderTarget(id) { }

    virtual SimpleIdentity getTextureId() const override { return texId; }

    virtual bool init(SceneRenderer *,Scene *,SimpleIdentity targetTexID) override
    {
        texId = targetTexID;
        return true;
    }
    virtual bool setTargetTexture(SceneRenderer *,Scene *,SimpleIdentity targetTexID) override
    {
        texId = targetTexID;
        return true;
    }

    virtual void clear() override { }

protected:
    SimpleIdentity texId = EmptyIdentity;
};

/** What and where we're rendering.  This can be a regular framebuffer
 to the screen or to a texture.
 */
class RenderTargetGLES : public RenderTargetSettings
{
public:
    RenderTargetGLES();
    RenderTargetGLES(SimpleIdentity newID);
    virtual ~RenderTargetGLES() = default;
    
    // Set up the render target
    virtual bool init(SceneRenderer *renderer,Scene *scene,SimpleIdentity targetTexID) override;
    
    // Pull in framebuffer info from the current OpenGL State
    bool initFromState(int inWidth,int inHeight);

    /// Set up the target texture
    virtual bool setTargetTexture(SceneRenderer *renderer,Scene *scene,SimpleIdentity newTargetTexID) override;

    // Clear up resources from the render target
    virtual void clear() override;
    
    /// Copy the data out of the destination texture and return it
    virtual RawDataRef snapshot() override;

    /// Copy just a subset out of the destination texture
    virtual RawDataRef snapshot(int startX,int startY,int snapWidth,int snapHeight) override;

    /// Make this framebuffer active
    virtual void setActiveFramebuffer(SceneRendererGLES *renderer);
        
    /// Set the GL texture directly
    void setTargetTexture(TextureBase *tex);

    /// OpenGL ES Name for the frame buffer
    GLuint framebuffer = 0;
    /// OpenGL ES Name for the color buffer
    GLuint colorbuffer = 0;
    /// OpenGL ES Name for the depth buffer
    GLuint depthbuffer = 0;
    
protected:
    virtual void init() override;

protected:
    bool isColorTarget = true;
};
typedef std::shared_ptr<RenderTargetGLES> RenderTargetGLESRef;

}
