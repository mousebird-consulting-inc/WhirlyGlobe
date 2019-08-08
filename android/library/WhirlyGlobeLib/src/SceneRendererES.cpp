/*
 *  SceneRendererES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/20/12.
 *  Copyright 2011-2016 mousebird consulting
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
#import "WhirlyKitLog.h"

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
}
    
bool RenderTarget::init(Scene *scene,SimpleIdentity targetTexID)
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    // Our destination is a texture in this case
    if (targetTexID)
    {
        colorbuffer = 0;
        TextureBase *tex = scene->getTexture(targetTexID);
        if (tex)
        {
            glBindTexture(GL_TEXTURE_2D, tex->getGLId());
            CheckGLError("RenderTarget: glBindTexture");
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->getGLId(), 0);
            CheckGLError("RenderTarget: glFramebufferTexture2D");
        } else
            WHIRLYKIT_LOGE("RenderTarget: No such texture %d",(int)targetTexID);

    } else {
        // Generate our own color buffer
        glGenRenderbuffers(1, &colorbuffer);
        CheckGLError("RenderTarget: glGenRenderbuffers");
        glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
        CheckGLError("RenderTarget: glBindRenderbuffer");
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuffer);
        CheckGLError("RenderTarget: glFramebufferRenderbuffer");
    }
    
//    WHIRLYKIT_LOGD("RenderTarget init: framebuffer = %d colorbuffer = %d width = %d, height = %d, targetTexID = %d",(int)framebuffer,(int)colorbuffer,width,height,(int)targetTexID);
    
    glGenRenderbuffers(1, &depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
    CheckGLError("RenderTarget: glFramebufferRenderbuffer");
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        WHIRLYKIT_LOGD("RenderTarget: Failed to build valid render target: %x",status);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    isSetup = false;
    return true;
}
    
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
    
void RenderTarget::setActiveFramebuffer(WhirlyKit::SceneRendererES *renderer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    CheckGLError("SceneRendererES2: glBindFramebuffer");
    glViewport(0, 0, width, height);
    CheckGLError("SceneRendererES2: glViewport");
    if (colorbuffer)
        glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
    
//    WHIRLYKIT_LOGD("RenderTarget setActive: framebuffer = %d colorbuffer = %d width = %d, height = %d",framebuffer,colorbuffer,width,height);
    
    if (!isSetup)
    {
        // Note: Should allow this to be selected
        // For non-main rendering targets, we want clear
        if (getId())
        {
            glClearColor(0.0,0.0,0.0,0.0);
        } else {
            RGBAColor color = renderer->getClearColor();
            glClearColor(color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
        }
        CheckGLError("SceneRendererES2: glClearColor");
        isSetup = true;
    }
}
    
AddRenderTargetReq::AddRenderTargetReq(SimpleIdentity renderTargetID,int width,int height,SimpleIdentity texID)
    : renderTargetID(renderTargetID), width(width), height(height), texID(texID)
{
}

// Set up a render target
void AddRenderTargetReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    RenderTarget renderTarget(renderTargetID);
    renderTarget.width = width;
    renderTarget.height = height;
    renderTarget.init(scene,texID);
    
    renderer->addRenderTarget(renderTarget);
}
    
RemRenderTargetReq::RemRenderTargetReq(SimpleIdentity targetID)
    : targetID(targetID)
{
}
    
void RemRenderTargetReq::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
    renderer->clearRenderTarget(targetID);
}
    
RendererFrameInfo::RendererFrameInfo()
    : oglVersion(0), sceneRenderer(NULL), theView(NULL), scene(NULL), frameLen(0), currentTime(0),
    heightAboveSurface(0), screenSizeInDisplayCoords(0,0), program(NULL), stateOpt(NULL)
    // Note: Porting
//,lights(NULL)
{
}
    
RendererFrameInfo::RendererFrameInfo(const RendererFrameInfo &that)
{
    *this = that;
}
    
OpenGLStateOptimizer::OpenGLStateOptimizer()
{
    reset();
}

void OpenGLStateOptimizer::reset()
{
    activeTexture = -1;
    depthMask = 0;
    depthTest = -1;
    progId = -1;
    lineWidth = -1.0;
    depthFunc = -1;
}

// Note: using glActiveTexture elsewhere so we can't optimize this
void OpenGLStateOptimizer::setActiveTexture(GLenum newActiveTexture)
{
//    if (newActiveTexture != activeTexture)
//    {
        glActiveTexture(newActiveTexture);
        activeTexture = newActiveTexture;
//    }
}

void OpenGLStateOptimizer::setDepthMask(bool newDepthMask)
{
    if (depthMask == -1 || (bool)depthMask != newDepthMask)
    {
        glDepthMask(newDepthMask);
        depthMask = newDepthMask;
    }
}

void OpenGLStateOptimizer::setEnableDepthTest(bool newEnable)
{
    if (depthTest == -1 || (bool)depthTest != newEnable)
    {
        if (newEnable)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        depthTest = newEnable;
    }
}

void OpenGLStateOptimizer::setDepthFunc(GLenum newDepthFunc)
{
    if (depthFunc == -1 || newDepthFunc != depthFunc)
    {
        glDepthFunc(newDepthFunc);
        depthFunc = newDepthFunc;
    }
}

void OpenGLStateOptimizer::setUseProgram(GLuint newProgId)
{
//    if (progId != newProgId)
//    {
        glUseProgram(newProgId);
        progId = newProgId;
//    }
}

void OpenGLStateOptimizer::setLineWidth(GLfloat newLineWidth)
{
    if (lineWidth != newLineWidth || lineWidth == -1.0)
    {
        if (newLineWidth > 0.0)
        {
            glLineWidth(newLineWidth);
            lineWidth = newLineWidth;
        }
    }
}

SceneRendererES::SceneRendererES(int apiVersion)
{
    frameCount = 0;
    framesPerSec = 0.0;
    numDrawables = 0;
    frameCountStart = 0.0;
    frameCountStart = 0.0;
    zBufferMode = zBufferOn;
    doCulling = false;
    // Note: Debugging
    clearColor.r = 0;  clearColor.g = 0;  clearColor.b = 0;  clearColor.a = 0;
    perfInterval = -1;
    lastFrameRate = 0.0;
    scale = 1.0;
    scene = NULL;
    theView = NULL;
    
    // All the animations should work now, except for particle systems
    useViewChanged = true;

    // No longer really ncessary
    sortAlphaToEnd = false;
    
    // Off by default.  Because duh.
    depthBufferOffForAlpha = false;
    
    extraFrameMode = false;
}
    
void SceneRendererES::setup()
{
    lastDraw = 0;

//    if (view)
//        view->runViewUpdates();
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

void SceneRendererES::clearRenderTarget(SimpleIdentity targetID)
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

// Calculate an acceptable MBR from world coords
Mbr SceneRendererES::calcCurvedMBR(Point3f *corners,WhirlyGlobe::GlobeView *globeView,Eigen::Matrix4d *modelTrans,Point2f frameSize)
{
    Mbr localScreenMbr;
    
    for (unsigned int ii=0;ii<WhirlyKitCullableCorners;ii++)
    {
        Point3d cornerPt = Point3d(corners[ii].x(),corners[ii].y(),corners[ii].z());
        Point2f screenPt = globeView->pointOnScreenFromSphere(cornerPt,modelTrans,frameSize);
        localScreenMbr.addPoint(Point2f(screenPt.x(),screenPt.y()));
    }
    
    return localScreenMbr;
}

void SceneRendererES::mergeDrawableSet(const std::set<DrawableRef,IdentifiableRefSorter> &newDrawables,WhirlyGlobe::GlobeView *globeView,Point2f frameSize,Eigen::Matrix4d *modelTrans,WhirlyKit::RendererFrameInfo *frameInfo,Mbr screenMbr,std::set<DrawableRef> *toDraw,int *drawablesConsidered)
{
    // Grab any drawables that live just at this level
    *drawablesConsidered += newDrawables.size();
    for (std::set<DrawableRef,IdentifiableSorter>::const_iterator it = newDrawables.begin();
         it != newDrawables.end(); ++it)
    {
        DrawableRef draw = *it;
        // Make sure we haven't added it already and it's on
        // Note: We're doing the on check repeatedly
        //       And we're doing the refusal check repeatedly as well, possibly
        if ((toDraw->find(draw) == toDraw->end()) && draw->isOn(frameInfo))
            toDraw->insert(draw);
    }
}

void SceneRendererES::findDrawables(WhirlyKit::Cullable *cullable,WhirlyGlobe::GlobeView *globeView,WhirlyKit::Point2f frameSize,Eigen::Matrix4d *modelTrans,Eigen::Vector3f eyeVec,WhirlyKit::RendererFrameInfo *frameInfo,WhirlyKit::Mbr screenMbr,bool isTopLevel,std::set<WhirlyKit::DrawableRef> *toDraw,int *drawablesConsidered)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    // Check the four corners of the cullable to see if they're pointed away
    // But just for the globe case
    bool inView = false;
    if (coordAdapter->isFlat() || isTopLevel)
    {
        inView = true;
    } else {
        for (unsigned int ii=0;ii<WhirlyKitCullableCornerNorms;ii++)
        {
            Vector3f norm = cullable->cornerNorms[ii];
            if (norm.dot(eyeVec) > 0)
            {
                inView = true;
                break;
            }
        }
    }
    if (doCulling && !inView)
        return;
    
    Mbr localScreenMbr;
    if (globeView)
        localScreenMbr = calcCurvedMBR(&cullable->cornerPoints[0],globeView,modelTrans,frameSize);
    
    // If this doesn't overlap what we're viewing, we're done
    if (doCulling && !screenMbr.overlaps(localScreenMbr))
        return;
    
    // If the footprint of this level on the screen is larger than
    //  the screen area, keep going down (if we can).
    float localScreenArea = localScreenMbr.area();
    float screenArea = screenMbr.area();
    if (isTopLevel || (localScreenArea > screenArea/4 && cullable->hasChildren()))
    {
        // Grab the drawables at this level
        mergeDrawableSet(cullable->getDrawables(),globeView,frameSize,modelTrans,frameInfo,screenMbr,toDraw,drawablesConsidered);
        
        // And recurse downward for the rest
        for (unsigned int ii=0;ii<4;ii++)
        {
            Cullable *child = cullable->getChild(ii);
            if (child)
                findDrawables(child,globeView,frameSize,modelTrans,eyeVec,frameInfo,screenMbr,false,toDraw,drawablesConsidered);
        }
    } else {
        // If not, then just return what we found here
        mergeDrawableSet(cullable->getChildDrawables(),globeView,frameSize,modelTrans,frameInfo,screenMbr,toDraw,drawablesConsidered);
    }
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


