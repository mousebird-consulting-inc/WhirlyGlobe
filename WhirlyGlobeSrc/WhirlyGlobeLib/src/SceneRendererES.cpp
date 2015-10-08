/*
 *  SceneRendererES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/20/12.
 *  Copyright 2011-2015 mousebird consulting
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
// Note: Porting
//#import "UIColor+Stuff.h"
#import "GLUtils.h"
// Note: Porting
//#import "SelectionManager.h"

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
    scale = 1.0;
    scene = NULL;
    theView = NULL;
    defaultFramebuffer = colorRenderbuffer = depthRenderbuffer = 0;
    
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
    // Create default framebuffer object.
    glGenFramebuffers(1, &defaultFramebuffer);
    CheckGLError("SceneRendererES: glGenFramebuffers");
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    CheckGLError("SceneRendererES: glBindFramebuffer");
    
    // Create color render buffer and allocate backing store.
    glGenRenderbuffers(1, &colorRenderbuffer);
    CheckGLError("SceneRendererES: glGenRenderbuffers");
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    CheckGLError("SceneRendererES: glBindRenderbuffer");
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
    CheckGLError("SceneRendererES: glFramebufferRenderbuffer");
    
    // Allocate depth buffer
    glGenRenderbuffers(1, &depthRenderbuffer);
    CheckGLError("SceneRendererES: glGenRenderbuffers");
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    CheckGLError("SceneRendererES: glBindRenderbuffer");
}

SceneRendererES::~SceneRendererES()
{
    // Note: Porting
//    EAGLContext *oldContext = [EAGLContext currentContext];
//    if (oldContext != context)
//        [EAGLContext setCurrentContext:context];
    
	if (defaultFramebuffer)
	{
		glDeleteFramebuffers(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}
	
	if (colorRenderbuffer)
	{
		glDeleteRenderbuffers(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}
	
	if (depthRenderbuffer)
	{
		glDeleteRenderbuffers(1, &depthRenderbuffer	);
		depthRenderbuffer = 0;
	}
	
    // Note: Porting
//	if (oldContext != context)
//        [EAGLContext setCurrentContext:oldContext];
//	context = NULL;
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


