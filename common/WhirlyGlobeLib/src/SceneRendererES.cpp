/*
 *  SceneRendererES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/20/12.
 *  Copyright 2011-2017 mousebird consulting
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

#import "SceneRendererES.h"
#import "GLUtils.h"
#import "SelectionManager.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
// Compare two matrices float by float
// The default comparison seems to have an epsilon and the cwise version isn't getting picked up
bool matrixAisSameAsB(Matrix4d &a,Matrix4d &b)
{
    double *floatsA = a.data();
    double *floatsB = b.data();
    
    for (unsigned int ii=0;ii<16;ii++)
        if (floatsA[ii] != floatsB[ii])
            return false;
    
    return true;
}
    
RenderTarget::RenderTarget()
    : framebuffer(0), colorbuffer(0), depthbuffer(0), width(0), height(0), isSetup(false)
{
    init();
}
    
RenderTarget::RenderTarget(SimpleIdentity newID) : Identifiable(newID)
{
    init();
}
    
void RenderTarget::init()
{
    framebuffer = 0;
    colorbuffer = 0;
    depthbuffer = 0;
    width = 0;
    height = 0;
    isSetup = false;
    clearColor[0] = 0.0;  clearColor[1] = 0.0;  clearColor[2] = 0.0;  clearColor[3] = 0.0;
    clearEveryFrame = true;
    clearOnce = false;
}
    
bool RenderTarget::init(Scene *scene,SimpleIdentity targetTexID)
{
    if (framebuffer == 0)
        glGenFramebuffers(1, &framebuffer);
    
    // Our destination is a texture in this case
    if (targetTexID)
    {
        colorbuffer = 0;
        setTargetTexture(scene,targetTexID);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Generate our own color buffer
        if (colorbuffer == 0)
            glGenRenderbuffers(1, &colorbuffer);
        CheckGLError("RenderTarget: glGenRenderbuffers");
        glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
        CheckGLError("RenderTarget: glBindRenderbuffer");
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuffer);
        CheckGLError("RenderTarget: glFramebufferRenderbuffer");

        if (depthbuffer == 0)
            glGenRenderbuffers(1, &depthbuffer);
        CheckGLError("RenderTarget: glGenRenderbuffers");
        glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
        CheckGLError("RenderTarget: glBindRenderbuffer");
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        CheckGLError("RenderTarget: glRenderbufferStorage");
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
        CheckGLError("RenderTarget: glFramebufferRenderbuffer");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        CheckGLError("RenderTarget: glBindFramebuffer");
    }
    
//    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
//    if(status != GL_FRAMEBUFFER_COMPLETE) {
//        NSLog(@"Failed to build valid render target: %x", status);
//        return false;
//    }
    
    
    isSetup = false;
    return true;
}
    
void RenderTarget::setTargetTexture(Scene *scene,SimpleIdentity targetTexID)
{
    TextureBase *tex = scene->getTexture(targetTexID);
    if (tex)
        setTargetTexture(tex);
}

void RenderTarget::setTargetTexture(TextureBase *tex)
{
    if (framebuffer == 0) {
        glGenFramebuffers(1, &framebuffer);
        colorbuffer = 0;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->getGLId(), 0);
    CheckGLError("RenderTarget: glFramebufferTexture2D");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
    
#if 0
NSData *RenderTarget::snapshot()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    CheckGLError("SceneRendererES2: glBindFramebuffer");
    glViewport(0, 0, width, height);
    CheckGLError("SceneRendererES2: glViewport");

    // Note: We're just assuming this format from the texture.  Should check
    int len = width * height * sizeof(GLubyte) * 4;
    GLubyte* pixels = (GLubyte*) malloc(len);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    NSData *data = [[NSData alloc] initWithBytesNoCopy:pixels length:len];
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return data;
}
#endif

bool RenderTarget::initFromState(int inWidth,int inHeight)
{
    width = inWidth;
    height = inHeight;
    GLint iVal;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&iVal);    framebuffer = iVal;
    glGetIntegerv(GL_RENDERBUFFER_BINDING,&iVal);   colorbuffer = iVal;
    
//    WHIRLYKIT_LOGD("RenderTarget initFromState: framebuffer = %d colorbuffer = %d width = %d, height = %d",framebuffer,colorbuffer,width,height);
    
    return true;
}

void RenderTarget::clear()
{
    if (colorbuffer)
        glDeleteRenderbuffers(1,&colorbuffer);
    if (depthbuffer)
        glDeleteRenderbuffers(1,&depthbuffer);
    if (framebuffer)
        glDeleteFramebuffers(1,&framebuffer);
}
    
void RenderTarget::setActiveFramebuffer(SceneRendererES *renderer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    CheckGLError("RenderTarget::setActiveFramebuffer: glBindFramebuffer");
    glViewport(0, 0, width, height);
    CheckGLError("RenderTarget::setActiveFramebuffer: glViewport");
    if (colorbuffer) {
        glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
        CheckGLError("RenderTarget::setActiveFramebuffer: glBindRenderbuffer");
    }
    
    // Note: Have to run this all the time for some reason
//    if (!isSetup)
    {
        if (blendEnable)
        {
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
        } else {
            glDisable(GL_BLEND);
        }
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        
        CheckGLError("RenderTarget::setActiveFramebuffer: glClearColor");
        isSetup = true;
    }
}
    
AddRenderTargetReq::AddRenderTargetReq(SimpleIdentity renderTargetID,int width,int height,SimpleIdentity texID,bool clearEveryFrame,bool blend,const RGBAColor &clearColor)
    : renderTargetID(renderTargetID), width(width), height(height), texID(texID), clearEveryFrame(clearEveryFrame), blend(blend), clearColor(clearColor)
{
}

// Set up a render target
void AddRenderTargetReq::execute(Scene *scene,SceneRendererES *renderer,View *view)
{
    RenderTarget renderTarget(renderTargetID);
    renderTarget.width = width;
    renderTarget.height = height;
    renderTarget.clearEveryFrame = clearEveryFrame;
    renderTarget.clearColor[0] = clearColor.r;
    renderTarget.clearColor[1] = clearColor.g;
    renderTarget.clearColor[2] = clearColor.b;
    renderTarget.clearColor[3] = clearColor.a;
    renderTarget.blendEnable = blend;
    renderTarget.init(scene,texID);
    
    renderer->addRenderTarget(renderTarget);
}
    
ChangeRenderTargetReq::ChangeRenderTargetReq(SimpleIdentity renderTargetID,SimpleIdentity texID)
    : renderTargetID(renderTargetID), texID(texID)
{
}
    
void ChangeRenderTargetReq::execute(Scene *scene,SceneRendererES *renderer,View *view)
{
    for (RenderTarget &renderTarget : renderer->renderTargets)
    {
        if (renderTarget.getId() == renderTargetID) {
            renderTarget.setTargetTexture(scene,texID);
            break;
        }
    }
}
    
RemRenderTargetReq::RemRenderTargetReq(SimpleIdentity targetID)
    : targetID(targetID)
{
}
    
void RemRenderTargetReq::execute(Scene *scene,SceneRendererES *renderer,View *view)
{
    renderer->removeRenderTarget(targetID);
}

ClearRenderTargetReq::ClearRenderTargetReq(SimpleIdentity targetID)
: renderTargetID(targetID)
{
}

void ClearRenderTargetReq::execute(Scene *scene,SceneRendererES *renderer,View *view)
{
    for (RenderTarget &renderTarget : renderer->renderTargets)
    {
        if (renderTarget.getId() == renderTargetID) {
            renderTarget.clearOnce = true;
            break;
        }
    }
}

RendererFrameInfo::RendererFrameInfo()
    : glesVersion(0), sceneRenderer(NULL), theView(NULL), scene(NULL), frameLen(0), currentTime(0),
    heightAboveSurface(0), screenSizeInDisplayCoords(0,0), program(NULL)
{
}
    
RendererFrameInfo::RendererFrameInfo(const RendererFrameInfo &that)
{
    *this = that;
}
    
SceneRendererES::SceneRendererES()
{
}

bool SceneRendererES::setup(int apiVersion,int sizeX,int sizeY)
{
    glesVersion = apiVersion;
    frameCount = 0;
    framesPerSec = 0.0;
    numDrawables = 0;
    frameCountStart = 0.0;
    frameCountStart = 0.0;
    zBufferMode = zBufferOn;
    clearColor.r = 0;  clearColor.g = 0;  clearColor.b = 0;  clearColor.a = 0;
    perfInterval = -1;
    scale = DeviceScreenScale();
    scene = NULL;
    theView = NULL;
    
    // All the animations should work now, except for particle systems
    useViewChanged = true;

    // No longer really ncessary
    sortAlphaToEnd = false;
    
    // Off by default.  Because duh.
    depthBufferOffForAlpha = false;
    
    extraFrameMode = false;
    
    framebufferWidth = sizeX;
    framebufferHeight = sizeY;
    
    // We need a texture to draw to in this case
    if (framebufferWidth > 0)
    {
        framebufferTex = new Texture("Framebuffer Texture");
        framebufferTex->setWidth(framebufferWidth);
        framebufferTex->setHeight(framebufferHeight);
        framebufferTex->setIsEmptyTexture(true);
        framebufferTex->setFormat(GL_UNSIGNED_BYTE);
        framebufferTex->createInGL(NULL);
    }
    
    RenderTarget defaultTarget(EmptyIdentity);
    defaultTarget.width = sizeX;
    defaultTarget.height = sizeY;
    if (framebufferTex) {
        defaultTarget.setTargetTexture(framebufferTex);
        // Note: Should make this optional
        defaultTarget.blendEnable = false;
    } else {
        defaultTarget.init(NULL,EmptyIdentity);
        defaultTarget.blendEnable = true;
    }
    defaultTarget.clearEveryFrame = true;
    renderTargets.push_back(defaultTarget);
    
    return true;
}
    
bool SceneRendererES::resize(int sizeX,int sizeY)
{
    // Don't want to deal with it for offscreen rendering
    if (framebufferTex)
        return false;
    
    framebufferWidth = sizeX;
    framebufferHeight = sizeY;
    
    auto defaultTarget = renderTargets.back();
    defaultTarget.init(NULL, EmptyIdentity);
    
    // Note: Check this
    return true;
}

SceneRendererES::~SceneRendererES()
{
    // Note: Porting
//    EAGLContext *oldContext = [EAGLContext currentContext];
//    if (oldContext != context)
//        [EAGLContext setCurrentContext:context];

   for (auto &target : renderTargets)
        target.clear();
        
	
    // Note: Porting
//	if (oldContext != context)
//        [EAGLContext setCurrentContext:oldContext];
//	context = NULL;
}

void SceneRendererES::addRenderTarget(RenderTarget &newTarget)
{
    renderTargets.insert(renderTargets.begin(),newTarget);
}

void SceneRendererES::removeRenderTarget(SimpleIdentity targetID)
{
    for (int ii=0;ii<renderTargets.size();ii++)
    {
        RenderTarget &target = renderTargets[ii];
        if (target.getId() == targetID)
        {
            target.clear();
            renderTargets.erase(renderTargets.begin()+ii);
            break;
        }
    }
}

// We'll take the maximum requested time
void SceneRendererES::setRenderUntil(TimeInterval newRenderUntil)
{
    renderUntil = std::max(renderUntil,newRenderUntil);
}

void SceneRendererES::setTriggerDraw()
{
    triggerDraw = true;
}
    
void SceneRendererES::addContinuousRenderRequest(SimpleIdentity drawID)
{
    contRenderRequests.insert(drawID);
}

void SceneRendererES::removeContinuousRenderRequest(SimpleIdentity drawID)
{
    SimpleIDSet::iterator it = contRenderRequests.find(drawID);
    if (it != contRenderRequests.end())
        contRenderRequests.erase(it);
}


void SceneRendererES::setScene(WhirlyKit::Scene *newScene)
{
    scene = newScene;
    if (scene)
    {
        scene->setRenderer(this);
    }
}

void SceneRendererES::setClearColor(const RGBAColor &color)
{
    clearColor = color;
}

RGBAColor SceneRendererES::getClearColor()
{
    return clearColor;
}

// Check if the view changed from the last frame
bool SceneRendererES::viewDidChange()
{
    if (!useViewChanged)
        return true;
    
    // First time through
    if (lastDraw == 0.0)
        return true;
    
    // Something wants to be sure we draw on the next frame
    if (triggerDraw)
    {
        triggerDraw = false;
        return true;
    }
    
    // Something wants us to draw (probably an animation)
    // We look at the last draw so we can handle jumps in time
    if (lastDraw < renderUntil)
        return true;
    
    Matrix4d newModelMat = theView->calcModelMatrix();
    Matrix4d newViewMat = theView->calcViewMatrix();
    Matrix4d newProjMat = theView->calcProjectionMatrix(Point2f(framebufferWidth,framebufferHeight),0.0);
    
    // Should be exactly the same
    if (matrixAisSameAsB(newModelMat,modelMat) && matrixAisSameAsB(newViewMat,viewMat) && matrixAisSameAsB(newProjMat, projMat))
        return false;
    
    modelMat = newModelMat;
    viewMat = newViewMat;
    projMat = newProjMat;
    return true;
}

void SceneRendererES::forceDrawNextFrame()
{
    triggerDraw = true;
}

}


