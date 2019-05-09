/*
 *  RenderTarget.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/19.
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

#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "Scene.h"
#import "PerformanceTimer.h"
#import "Lighting.h"

namespace WhirlyKit
{
class SceneRenderer;

/** What and where we're rendering.  This can be a regular framebuffer
 to the screen or to a texture.
 */
class RenderTarget : public Identifiable
{
public:
    RenderTarget();
    RenderTarget(SimpleIdentity newID);
    
    /// Set up the target texture
    virtual void setTargetTexture(Scene *scene,SimpleIdentity newTargetTexID);
    
    /// Copy the data out of the destination texture and return it
    virtual RawDataRef snapshot();
    
    // Clear color, if we're clearing
    float clearColor[4];
    bool clearEveryFrame;
    // Clear on the next frame, then reset this
    bool clearOnce;
    // Control how the blending into a destination works
    bool blendEnable;
};
typedef std::shared_ptr<RenderTarget> RenderTargetRef;

// Add a new render target
class AddRenderTargetReq : public ChangeRequest
{
public:
    AddRenderTargetReq(SimpleIdentity renderTargetID,int width,int height,SimpleIdentity texID,bool clearEveryFrame,bool blend,const RGBAColor &clearColor);
    
    /// Add the render target to the renderer
    void execute(Scene *scene,SceneRenderer *renderer,View *view);
    
protected:
    int width,height;
    SimpleIdentity renderTargetID;
    SimpleIdentity texID;
    bool clearEveryFrame;
    RGBAColor clearColor;
    bool blend;
};

// Change details about a rendering target.  In this case, just texture.
class ChangeRenderTargetReq : public ChangeRequest
{
public:
    ChangeRenderTargetReq(SimpleIdentity renderTargetID,SimpleIdentity texID);
    
    /// Add the render target to the renderer
    void execute(Scene *scene,SceneRenderer *renderer,View *view);
    
protected:
    SimpleIdentity renderTargetID;
    SimpleIdentity texID;
};

// Request a one time clear on the rendering target.  Happens next frame.
class ClearRenderTargetReq : public ChangeRequest
{
public:
    ClearRenderTargetReq(SimpleIdentity renderTargetID);
    
    /// Add the render target to the renderer
    void execute(Scene *scene,SceneRenderer *renderer,View *view);
    
protected:
    SimpleIdentity renderTargetID;
};

// Remove a render target from the rendering loop
class RemRenderTargetReq : public ChangeRequest
{
public:
    RemRenderTargetReq(SimpleIdentity targetID);
    
    /// Remove the render target from the renderer
    void execute(Scene *scene,SceneRenderer *renderer,View *view);
    
protected:
    SimpleIdentity targetID;
};
    
}
