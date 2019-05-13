/*
 *  ESRenderer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
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

#import "UtilsGLES.h"

#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "Scene.h"
#import "PerformanceTimer.h"
#import "Lighting.h"
#import "SceneRenderer.h"
#import "ProgramGLES.h"
#import "MemManagerGLES.h"

namespace WhirlyKit
{
class SceneRendererGLES;

/** Renderer Frame Info.
 Data about the current frame, passed around by the renderer.
 */
class RendererFrameInfoGLES : public RendererFrameInfo
{
public:
    RendererFrameInfoGLES();
    RendererFrameInfoGLES(const RendererFrameInfoGLES &that);
    
    /// Renderer version (e.g. OpenGL ES 1 vs 2)
    int glesVersion;
};

/// Base class for the scene renderer.
/// It's subclassed for the specific version of OpenGL ES
class SceneRendererGLES : public SceneRenderer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    SceneRendererGLES();
    virtual ~SceneRendererGLES();
    
    /// Called right after the constructor
    virtual bool setup(int apiVersion,int sizeX,int sizeY);
    
    /// Resize framebuffer because something changed
    virtual bool resize(int sizeX,int sizeY);
    
    /// Called before we present the render buffer.  Can do snapshot logic here.
    virtual void snapshotCallback();
    
    /// The next time through we'll redo the render setup.
    /// We might need this if the view has switched away and then back.
    void forceRenderSetup();
    
    /// If set, we'll draw one more frame than needed after updates stop
    virtual void setExtraFrameMode(bool newMode);
    
    /// Draw stuff (the whole point!)
    void render(TimeInterval period);
    
public:
    // Possible post-target creation init
    virtual void defaultTargetInit(RenderTarget *) { };
    
    // Presentation, if required
    virtual void presentRender() { };
    
    // OpenGL Version
    int glesVersion;
    
    // If set we draw one extra frame after updates stop
    bool extraFrameMode;
    bool extraFrameDrawn;
};
    
typedef std::shared_ptr<SceneRendererGLES> SceneRendererGLESRef;

}
