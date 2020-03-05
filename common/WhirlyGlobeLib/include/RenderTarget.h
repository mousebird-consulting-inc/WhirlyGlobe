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

// If we're doing mipmaps for the render target texture, how they're calculated
typedef enum {RenderTargetMipmapNone,RenderTargetMimpapAverage,RenderTargetMipmapGauss} RenderTargetMipmapType;

/** What and where we're rendering.  This can be a regular framebuffer
 to the screen or to a texture.
 */
class RenderTarget : public Identifiable
{
public:
    RenderTarget();
    RenderTarget(SimpleIdentity newID);
    virtual ~RenderTarget();

    // Initialize in the renderer
    virtual bool init(SceneRenderer *renderer,Scene *scene,SimpleIdentity targetTexID) = 0;

    /// Set up the target texture
    virtual bool setTargetTexture(SceneRenderer *renderer,Scene *scene,SimpleIdentity newTargetTexID) = 0;

    /// Set the clear color
    virtual void setClearColor(const RGBAColor &color) = 0;
    
    /// If we're generating mipmaps for the render target, this is how
    virtual void setMipmap(RenderTargetMipmapType inMipmapType) { mipmapType = inMipmapType; }
    
    /// Calculate the min/max values for a given render target every frame
    virtual void setCalcMinMax(bool newVal) { calcMinMax = newVal; }
    
    // Clear up resources from the render target (not clear the buffer)
    virtual void clear() = 0;
    
    /// If we're tied to a texture, the number of levels in that texture
    virtual int numLevels();
    
    /// Copy the data out of the destination texture and return it
    virtual RawDataRef snapshot() { return RawDataRef(); };

    /// Copy just a subset out of the destination texture and return it
    virtual RawDataRef snapshot(int startX,int startY,int snapWidth,int snapHeight) { return RawDataRef(); };
    
    /// If we've asked for a min/max calculation, this is where we get it
    virtual RawDataRef snapshotMinMax() { return RawDataRef(); }
    
    /// Output framebuffer size
    int width,height;
    /// Set if we've set up background and such
    bool isSetup;

    // Clear color, if we're clearing
    float clearColor[4];
    // Used for non-4 channel RGBA targets
    float clearVal;
    bool clearEveryFrame;
    // Clear on the next frame, then reset this
    bool clearOnce;
    // Control how the blending into a destination works
    bool blendEnable;
    
public:
    RenderTargetMipmapType mipmapType;
    bool calcMinMax;
    virtual void init();
};
typedef std::shared_ptr<RenderTarget> RenderTargetRef;

// Add a new render target
class AddRenderTargetReq : public ChangeRequest
{
public:
    AddRenderTargetReq(SimpleIdentity renderTargetID,
                       int width,int height,
                       SimpleIdentity texID,
                       bool clearEveryFrame,
                       bool blend,
                       const RGBAColor &clearColor,
                       float clearVal,
                       RenderTargetMipmapType mipmapType,
                       bool calcMinMax);
    
    /// Add the render target to the renderer
    void execute(Scene *scene,SceneRenderer *renderer,View *view);
    
protected:
    int width,height;
    SimpleIdentity renderTargetID;
    SimpleIdentity texID;
    bool clearEveryFrame;
    RGBAColor clearColor;
    float clearVal;
    bool blend;
    RenderTargetMipmapType mipmapType;
    bool calcMinMax;
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
