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
#import "UIColor+Stuff.h"
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
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->getGLId(), 0);
    } else {
        // Generate our own color buffer
        glGenRenderbuffers(1, &colorbuffer);
        CheckGLError("SceneRendererES: glGenRenderbuffers");
        glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
        CheckGLError("SceneRendererES: glBindRenderbuffer");
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuffer);
        CheckGLError("SceneRendererES: glFramebufferRenderbuffer");
    }
    
    glGenRenderbuffers(1, &depthbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
    
//    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
//    if(status != GL_FRAMEBUFFER_COMPLETE) {
//        NSLog(@"Failed to build valid render target: %x", status);
//        return false;
//    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    isSetup = false;
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
    
void RenderTarget::setActiveFramebuffer(WhirlyKitSceneRendererES *renderer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    CheckGLError("SceneRendererES2: glBindFramebuffer");
    glViewport(0, 0, width, height);
    CheckGLError("SceneRendererES2: glViewport");
    if (colorbuffer)
        glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
    
//    if (!isSetup)
    {
        // Note: Should allow this to be selected
        // For non-main rendering targets, we want clear
        if (getId())
            glClearColor(0.0,0.0,0.0,0.0);
        else
            glClearColor(renderer->_clearColor.r / 255.0, renderer->_clearColor.g / 255.0, renderer->_clearColor.b / 255.0, renderer->_clearColor.a / 255.0);
        CheckGLError("SceneRendererES2: glClearColor");
        isSetup = true;
    }
}
    
AddRenderTargetReq::AddRenderTargetReq(SimpleIdentity renderTargetID,int width,int height,SimpleIdentity texID)
    : renderTargetID(renderTargetID), width(width), height(height), texID(texID)
{
}

// Set up a render target
void AddRenderTargetReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    RenderTarget renderTarget(renderTargetID);
    renderTarget.width = width;
    renderTarget.height = height;
    renderTarget.init(scene,texID);
    
    [renderer addRenderTarget:renderTarget];
}
    
RemRenderTargetReq::RemRenderTargetReq(SimpleIdentity targetID)
    : targetID(targetID)
{
}
    
void RemRenderTargetReq::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
    [renderer clearRenderTarget:targetID];
}

}

@implementation WhirlyKitRendererFrameInfo

- (id)initWithFrameInfo:(WhirlyKitRendererFrameInfo *)info
{
    self = [super init];
    
    _oglVersion = info.oglVersion;
    _sceneRenderer = info.sceneRenderer;
    _theView = info.theView;
    _modelTrans = info.modelTrans;
    _modelTrans4d = info.modelTrans4d;
    _viewTrans = info.viewTrans;
    _viewTrans4d = info.viewTrans4d;
    _projMat = info.projMat;
    _projMat4d = info.projMat4d;
    _viewAndModelMat = info.viewAndModelMat;
    _viewAndModelMat4d = info.viewAndModelMat4d;
    _mvpMat = info.mvpMat;
    _mvpNormalMat = info.mvpNormalMat;
    _viewModelNormalMat = info.viewModelNormalMat;
    _pvMat = info.pvMat;
    _pvMat4d = info.pvMat4d;
    _offsetMatrices = info.offsetMatrices;
    _scene = info.scene;
    _frameLen = info.frameLen;
    _currentTime = info.currentTime;
    _eyeVec = info.eyeVec;
    _fullEyeVec = info.fullEyeVec;
    _eyePos = info.eyePos;
    _dispCenter = info.dispCenter;
    _heightAboveSurface = info.heightAboveSurface;
    _screenSizeInDisplayCoords = info.screenSizeInDisplayCoords;
    _program = info.program;
    _lights = info.lights;
    _stateOpt = info.stateOpt;
    
    return self;
}

@end

@implementation WhirlyKitOpenGLStateOptimizer
{
    int activeTexture;
    int depthMask;
    int depthTest;
    int progId;
    int depthFunc;
    GLfloat lineWidth;
}

- (id)init
{
    self = [super init];
    [self reset];
    
    return self;
}

- (void)reset
{
    activeTexture = -1;
    depthMask = 0;
    depthTest = -1;
    progId = -1;
    lineWidth = -1.0;
    depthFunc = -1;
}

// Note: using glActiveTexture elsewhere so we can't optimize this
- (void)setActiveTexture:(GLenum)newActiveTexture
{
//    if (newActiveTexture != activeTexture)
//    {
        glActiveTexture(newActiveTexture);
        activeTexture = newActiveTexture;
//    }
}

- (void)setDepthMask:(bool)newDepthMask
{
    if (depthMask == -1 || (bool)depthMask != newDepthMask)
    {
        glDepthMask(newDepthMask);
        depthMask = newDepthMask;
    }
}

- (void)setEnableDepthTest:(bool)newEnable
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

- (void)setDepthFunc:(GLenum)newDepthFunc
{
    if (depthFunc == -1 || newDepthFunc != depthFunc)
    {
        glDepthFunc(newDepthFunc);
        depthFunc = newDepthFunc;
    }
}

- (void)setUseProgram:(GLuint)newProgId
{
//    if (progId != newProgId)
//    {
        glUseProgram(newProgId);
        progId = newProgId;
//    }
}

- (void)setLineWidth:(GLfloat)newLineWidth
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

@end

@implementation WhirlyKitSceneRendererES
{
    // View state from the last render, for comparison
    Eigen::Matrix4d modelMat,viewMat,projMat;    
}

- (id) initWithOpenGLESVersion:(EAGLRenderingAPI)apiVersion
{
	if ((self = [super init]))
	{
		frameCount = 0;
		_framesPerSec = 0.0;
        _numDrawables = 0;
		frameCountStart = 0.0;
        _zBufferMode = zBufferOn;
        _doCulling = true;
        _clearColor.r = 0.0;  _clearColor.g = 0.0;  _clearColor.b = 0.0;  _clearColor.a = 1.0;
        _perfInterval = -1;
        _scale = [[UIScreen mainScreen] scale];
		
		_context = [[EAGLContext alloc] initWithAPI:apiVersion];
        
        EAGLContext *oldContext = [EAGLContext currentContext];
        if (!_context || ![EAGLContext setCurrentContext:_context])
		{
            return nil;
        }
        
        RenderTarget defaultTarget(EmptyIdentity);
        defaultTarget.init(NULL,EmptyIdentity);
        renderTargets.push_back(defaultTarget);
        
        // All the animations should work now, except for particle systems
        _useViewChanged = true;

        // No longer really ncessary
        _sortAlphaToEnd = false;
        
        // Off by default.  Because duh.
        _depthBufferOffForAlpha = false;
        
        [EAGLContext setCurrentContext:oldContext];        
	}
	
	return self;
}

- (void) dealloc
{
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != _context)
        [EAGLContext setCurrentContext:_context];
    
    for (auto &target : renderTargets)
        target.clear();
    
	if (oldContext != _context)
        [EAGLContext setCurrentContext:oldContext];
	_context = nil;	
}

/// Add the given rendering target and, you know, render to it
- (void) addRenderTarget:(RenderTarget &)newTarget
{
    renderTargets.insert(renderTargets.begin(),newTarget);
}

/// Clear out the given render target
- (void) clearRenderTarget:(SimpleIdentity)targetID
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
- (void)setRenderUntil:(NSTimeInterval)newRenderUntil
{
    renderUntil = std::max(renderUntil,newRenderUntil);
}

- (void)addContinuousRenderRequest:(SimpleIdentity)drawID
{
    contRenderRequests.insert(drawID);
}

- (void)removeContinuousRenderRequest:(SimpleIdentity)drawID
{
    SimpleIDSet::iterator it = contRenderRequests.find(drawID);
    if (it != contRenderRequests.end())
        contRenderRequests.erase(it);
}

- (void)setTriggerDraw
{
    _triggerDraw = true;
}

- (void)setScene:(WhirlyKit::Scene *)newScene
{
    _scene = newScene;
    if (_scene)
    {
        _scene->setRenderer(self);
    }
}

- (void)useContext
{
	if (_context && [EAGLContext currentContext] != _context)
		[EAGLContext setCurrentContext:_context];
}

- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer
{
    if (renderTargets.empty())
        return false;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != _context)
        [EAGLContext setCurrentContext:_context];
    
    RenderTarget &renderTarget = renderTargets[0];
    
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderTarget.colorbuffer);
    CheckGLError("SceneRendererES: glBindRenderbuffer");
	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)layer];
    CheckGLError("SceneRendererES: glBindRenderbuffer");
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_framebufferWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_framebufferHeight);
    renderTarget.width = _framebufferWidth;
    renderTarget.height = _framebufferHeight;
    
	// For this sample, we also need a depth buffer, so we'll create and attach one via another renderbuffer.
	glBindRenderbuffer(GL_RENDERBUFFER, renderTarget.depthbuffer);
    CheckGLError("SceneRendererES: glBindRenderbuffer");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _framebufferWidth, _framebufferHeight);
    CheckGLError("SceneRendererES: glRenderbufferStorage");
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTarget.depthbuffer);
    CheckGLError("SceneRendererES: glFramebufferRenderbuffer");
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (oldContext != _context)
            [EAGLContext setCurrentContext:oldContext];
		return NO;
	}
		
    lastDraw = 0;
	
    if (oldContext != _context)
        [EAGLContext setCurrentContext:oldContext];
    
    // If we've resized, we're looking at different content
    if (_theView)
        [_theView runViewUpdates];
    
	return YES;
}

- (void) setClearColor:(UIColor *)color
{
    _clearColor = [color asRGBAColor];
}

// Calculate an acceptable MBR from world coords
- (Mbr) calcCurvedMBR:(Point3f *)corners view:(WhirlyGlobeView *)globeView modelTrans:(Eigen::Matrix4d *)modelTrans frameSize:(Point2f)frameSize
{
    Mbr localScreenMbr;
    
    for (unsigned int ii=0;ii<WhirlyKitCullableCorners;ii++)
    {
        Point3d cornerPt = Point3d(corners[ii].x(),corners[ii].y(),corners[ii].z());
        CGPoint screenPt = [globeView pointOnScreenFromSphere:cornerPt transform:modelTrans frameSize:frameSize];
        localScreenMbr.addPoint(Point2f(screenPt.x,screenPt.y));
    }
    
    return localScreenMbr;
}

- (void) mergeDrawableSet:(const std::set<DrawableRef,IdentifiableRefSorter> &)newDrawables globeView:(WhirlyGlobeView *)globeView frameSize:(Point2f)frameSize modelTrans:(Eigen::Matrix4d *)modelTrans frameInfo:(WhirlyKitRendererFrameInfo *)frameInfo screenMbr:(Mbr)screenMbr toDraw:(std::set<DrawableRef> *) toDraw considered:(int *)drawablesConsidered
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

- (void) findDrawables:(Cullable *)cullable view:(WhirlyGlobeView *)globeView frameSize:(Point2f)frameSize modelTrans:(Eigen::Matrix4d *)modelTrans eyeVec:(Vector3f)eyeVec frameInfo:(WhirlyKitRendererFrameInfo *)frameInfo screenMbr:(Mbr)screenMbr topLevel:(bool)isTopLevel toDraw:(std::set<DrawableRef> *) toDraw considered:(int *)drawablesConsidered
{
    CoordSystemDisplayAdapter *coordAdapter = _scene->getCoordAdapter();
    
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
    if (_doCulling && !inView)
        return;
    
    Mbr localScreenMbr;
    if (globeView)
        localScreenMbr = [self calcCurvedMBR:&cullable->cornerPoints[0] view:globeView modelTrans:modelTrans frameSize:frameSize];
    
    // If this doesn't overlap what we're viewing, we're done
    if (_doCulling && !screenMbr.overlaps(localScreenMbr))
        return;
    
    // If the footprint of this level on the screen is larger than
    //  the screen area, keep going down (if we can).
    float localScreenArea = localScreenMbr.area();
    float screenArea = screenMbr.area();
    if (isTopLevel || (localScreenArea > screenArea/4 && cullable->hasChildren()))
    {
        // Grab the drawables at this level
        [self mergeDrawableSet:cullable->getDrawables() globeView:globeView frameSize:frameSize modelTrans:modelTrans frameInfo:frameInfo screenMbr:screenMbr toDraw:toDraw considered:drawablesConsidered];
        
        // And recurse downward for the rest
        for (unsigned int ii=0;ii<4;ii++)
        {
            Cullable *child = cullable->getChild(ii);
            if (child)
                [self findDrawables:child view:globeView frameSize:frameSize modelTrans:modelTrans eyeVec:eyeVec frameInfo:frameInfo screenMbr:screenMbr topLevel:false toDraw:toDraw considered:drawablesConsidered];
        }
    } else {
        // If not, then just return what we found here
        [self mergeDrawableSet:cullable->getChildDrawables() globeView:globeView frameSize:frameSize modelTrans:modelTrans frameInfo:frameInfo screenMbr:screenMbr toDraw:toDraw considered:drawablesConsidered];
    }
}

// Check if the view changed from the last frame
- (bool) viewDidChange
{
    if (!_useViewChanged)
        return true;
    
    // First time through
    if (lastDraw == 0.0)
        return true;
    
    // Something wants to be sure we draw on the next frame
    if (_triggerDraw)
    {
        _triggerDraw = false;
        return true;
    }
    
    // Something wants us to draw (probably an animation)
    // We look at the last draw so we can handle jumps in time
    if (lastDraw < renderUntil)
        return true;
    
    Matrix4d newModelMat = [_theView calcModelMatrix];
    Matrix4d newViewMat = [_theView calcViewMatrix];
    Matrix4d newProjMat = [_theView calcProjectionMatrix:Point2f(_framebufferWidth,_framebufferHeight) margin:0.0];
    
    // Should be exactly the same
    if (matrixAisSameAsB(newModelMat,modelMat) && matrixAisSameAsB(newViewMat,viewMat) && matrixAisSameAsB(newProjMat, projMat))
        return false;
    
    modelMat = newModelMat;
    viewMat = newViewMat;
    projMat = newProjMat;
    return true;
}

- (void)forceDrawNextFrame
{
    lastDraw = 0;
}

- (void)render:(NSTimeInterval)duration
{
    return;
}

- (void)processScene
{
    return;
}

@end


