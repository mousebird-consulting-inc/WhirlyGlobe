/*
 *  SceneRenderer.h
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

#import "RenderTarget.h"

namespace WhirlyKit
{
class SceneRendererES;

/** What and where we're rendering.  This can be a regular framebuffer
 to the screen or to a texture.
 */
class RenderTargetGLES : public RenderTarget
{
public:
    RenderTarget();
    RenderTarget(SimpleIdentity newID);
    void init();
    
    // Set up the render target
    bool init(SceneRendererES *renderer,Scene *scene,SimpleIdentity targetTexID);
    
    // Pull in framebuffer info from the current OpenGL State
    bool initFromState(int inWidth,int inHeight);
    
    // Clear up resources from the render target
    void clear();
    
    /// Make this framebuffer active
    void setActiveFramebuffer(SceneRendererES *renderer);
    
    /// Set up the target texture
    void setTargetTexture(Scene *scene,SimpleIdentity newTargetTexID);
    
    /// Set the GL texture directly
    void setTargetTexture(TextureBase *tex);
    
    /// Copy the data out of the destination texture and return it
    RawDataRef snapshot();
    
    /// OpenGL ES Name for the frame buffer
    GLuint framebuffer;
    /// OpenGL ES Name for the color buffer
    GLuint colorbuffer;
    /// OpenGL ES Name for the depth buffer
    GLuint depthbuffer;
    /// Output framebuffer size fo glViewport
    int width,height;
    /// Set if we've set up background and suchs
    bool isSetup;
    
};
typedef std::shared_ptr<RenderTargetGLES> RenderTargetGLESRef;

}
