/*
 *  SceneRendererES2.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
 *  Copyright 2011-2013 mousebird consulting
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

#import "SceneRendererES2.h"
#import "UIColor+Stuff.h"
#import "GLUtils.h"
#import "DefaultShaderPrograms.h"
#import "UIImage+Stuff.h"
#import "NSDictionary+Stuff.h"
#import "NSString+Stuff.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

// Alpha stuff goes at the end
// Otherwise sort by draw priority
class DrawListSortStruct2
{
public:
    DrawListSortStruct2(bool useAlpha,bool useZBuffer,WhirlyKit::RendererFrameInfo *frameInfo) : useAlpha(useAlpha), useZBuffer(useZBuffer), frameInfo(frameInfo)
    {
    }
    DrawListSortStruct2() { }
    DrawListSortStruct2(const DrawListSortStruct &that) : useAlpha(that.useAlpha), useZBuffer(that.useZBuffer), frameInfo(that.frameInfo)
    {
    }
    DrawListSortStruct2 & operator = (const DrawListSortStruct &that)
    {
        useAlpha = that.useAlpha;
        useZBuffer= that.useZBuffer;
        frameInfo = that.frameInfo;
        return *this;
    }
    bool operator()(Drawable *a,Drawable *b)
    {
        // We may or may not sort all alpha containing drawables to the end
        if (useAlpha)
            if (a->hasAlpha(frameInfo) != b->hasAlpha(frameInfo))
                return !a->hasAlpha(frameInfo);
 
        if (a->getDrawPriority() == b->getDrawPriority())
        {
            if (useZBuffer)
            {
                bool bufferA = a->getRequestZBuffer();
                bool bufferB = b->getRequestZBuffer();
                if (bufferA != bufferB)
                    return !bufferA;
            }
        }
                
        return a->getDrawPriority() < b->getDrawPriority();
    }
    
    bool useAlpha,useZBuffer;
    WhirlyKit::RendererFrameInfo *frameInfo;
};
    
}

@implementation WhirlyKitFrameMessage
@end

SceneRendererES2::SceneRendererES2()
: SceneRendererES(kEAGLRenderingAPIOpenGLES2), renderStateOptimizer(NULL), renderSetup(false)
{
    // We do this to pull in the categories without the -ObjC flag.
    // It's dumb, but it works
    static bool dummyInit = false;
    if (!dummyInit)
    {
        UIImageDummyFunc();
        NSDictionaryDummyFunc();
        UIColorDummyFunc();
        NSStringDummyFunc();
        dummyInit = true;
    }

    lights = [NSMutableArray array];
    
    // Add a simple default light
    WhirlyKitDirectionalLight *light = [[WhirlyKitDirectionalLight alloc] init];
    [light setPos:Vector3f(0.75, 0.5, -1.0)];
    light.viewDependent = true;
    light.ambient = Vector4f(0.6, 0.6, 0.6, 1.0);
    light.diffuse = Vector4f(0.5, 0.5, 0.5, 1.0);
    light.specular = Vector4f(0, 0, 0, 0);
    lightsLastUpdated = 0;
    addLight(light);

    // And a basic material
    setDefaultMaterial([[WhirlyKitMaterial alloc] init]);

    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    
    frameRenderingSemaphore = dispatch_semaphore_create(1);
    contextQueue = dispatch_queue_create("rendering queue",DISPATCH_QUEUE_SERIAL);
    
    renderSetup = false;

    // Note: Try to turn this back on at some point
    _dispatchRendering = false;
}

SceneRendererES2::~SceneRendererES2()
{
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 60000
    dispatch_release(contextQueue);
#endif
    if (renderStateOptimizer)
        delete renderStateOptimizer;
    renderStateOptimizer = nil;
}

void SceneRendererES2::forceRenderSetup()
{
    renderSetup = false;
}

// When the scene is set, we'll compile our shaders
void SceneRendererES2::setScene(WhirlyKit::Scene *inScene)
{
    SceneRendererES::setScene(inScene);
    scene = inScene;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];
    
    SetupDefaultShaders(scene);
    
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();

    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
}

/// Add a light to the existing set
void SceneRendererES2::addLight(WhirlyKitDirectionalLight *light)
{
    if (!lights)
        lights = [NSMutableArray array];
    [lights addObject:light];
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    triggerDraw = true;
}

/// Replace all the lights at once. nil turns off lighting
void SceneRendererES2::replaceLights(NSArray *inLights)
{
    lights = [NSMutableArray arrayWithArray:inLights];
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    triggerDraw = true;
}

void SceneRendererES2::setDefaultMaterial(WhirlyKitMaterial *mat)
{
    defaultMat = mat;
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    triggerDraw = true;
}

void SceneRendererES2::setClearColor(UIColor *color)
{
    _clearColor = [color asRGBAColor];
    renderSetup = false;
}

BOOL SceneRendererES2::resizeFromLayer(CAEAGLLayer *layer)
{
    renderSetup = false;
    bool ret = SceneRendererES::resizeFromLayer(layer);
    
    return ret;
}

// Make the screen a bit bigger for testing
static const float ScreenOverlap = 0.1;

void SceneRendererES2::render(CFTimeInterval duration)
{
    // Let anyone who cares know the frame draw is starting
    WhirlyKitFrameMessage *frameMsg = [[WhirlyKitFrameMessage alloc] init];
    frameMsg.frameStart = CFAbsoluteTimeGetCurrent();
    frameMsg.frameInterval = duration;
    frameMsg.renderer = this;
    [[NSNotificationCenter defaultCenter] postNotificationName:kWKFrameMessage object:frameMsg];

    if (_dispatchRendering)
    {
        if (dispatch_semaphore_wait(frameRenderingSemaphore, DISPATCH_TIME_NOW) != 0)
            return;
        
        dispatch_async(contextQueue,
                       ^{
                           renderAsync();
                           dispatch_semaphore_signal(frameRenderingSemaphore);
                       });
    } else
        renderAsync();
}

void SceneRendererES2::renderAsync()
{
    if (!scene)
        return;
    
    frameCount++;
    
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
        return;

    if (!renderStateOptimizer)
        renderStateOptimizer = new OpenGLStateOptimizer();

	[theView animate];

    // Decide if we even need to draw
    if (!scene->hasChanges() && !viewDidChange())
        return;
        
    lastDraw = CFAbsoluteTimeGetCurrent();
        
    if (perfInterval > 0)
        perfTimer.startTiming("Render Frame");
    	
    if (perfInterval > 0)
        perfTimer.startTiming("Render Setup");
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];
    CheckGLError("SceneRendererES2: setCurrentContext");
    
    if (!renderSetup)
    {
        // Turn on blending
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }

    // See if we're dealing with a globe view
    WhirlyGlobeView *globeView = nil;
    if ([theView isKindOfClass:[WhirlyGlobeView class]])
        globeView = (WhirlyGlobeView *)theView;

    GLint framebufferWidth = framebufferWidth;
    GLint framebufferHeight = framebufferHeight;
    if (!renderSetup)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        CheckGLError("SceneRendererES2: glBindFramebuffer");
        glViewport(0, 0, framebufferWidth,framebufferHeight);
        CheckGLError("SceneRendererES2: glViewport");
    }

    // Get the model and view matrices
    Eigen::Matrix4d modelTrans4d = [theView calcModelMatrix];
    Eigen::Matrix4f modelTrans = Matrix4dToMatrix4f(modelTrans4d);
    Eigen::Matrix4d viewTrans4d = [theView calcViewMatrix];
    Eigen::Matrix4f viewTrans = Matrix4dToMatrix4f(viewTrans4d);
    
    // Set up a projection matrix
    Eigen::Matrix4d projMat4d = [theView calcProjectionMatrix:Point2f(framebufferWidth,framebufferHeight) margin:0.0];
    
    Eigen::Matrix4f projMat = Matrix4dToMatrix4f(projMat4d);
    Eigen::Matrix4f modelAndViewMat = viewTrans * modelTrans;
    Eigen::Matrix4f mvpMat = projMat * (modelAndViewMat);
    Eigen::Matrix4f modelAndViewNormalMat = modelAndViewMat.inverse().transpose();

    switch (zBufferMode)
    {
        case zBufferOn:
            renderStateOptimizer->setDepthMask(GL_TRUE);
            renderStateOptimizer->setEnableDepthTest(true);
            renderStateOptimizer->setDepthFunc(GL_LESS);
            break;
        case zBufferOff:
            renderStateOptimizer->setDepthMask(GL_FALSE);
            renderStateOptimizer->setEnableDepthTest(false);
            break;
        case zBufferOffDefault:
            renderStateOptimizer->setDepthMask(GL_TRUE);
            renderStateOptimizer->setEnableDepthTest(true);
            renderStateOptimizer->setDepthFunc(GL_ALWAYS);
            break;
    }
    
    if (!renderSetup)
    {
        // Note: What happens if they change this?
        glClearColor(_clearColor.r / 255.0, _clearColor.g / 255.0, _clearColor.b / 255.0, _clearColor.a / 255.0);
        CheckGLError("SceneRendererES2: glClearColor");
    }
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CheckGLError("SceneRendererES2: glClear");

    if (!renderSetup)
    {
        glEnable(GL_CULL_FACE);
        CheckGLError("SceneRendererES2: glEnable(GL_CULL_FACE)");
    }
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Setup");
    
	if (scene)
	{
		int numDrawables = 0;
        
        SimpleIdentity defaultTriShader = scene->getProgramIDBySceneName(kSceneDefaultTriShader);
        SimpleIdentity defaultLineShader = scene->getProgramIDBySceneName(kSceneDefaultLineShader);
        if ((defaultTriShader == EmptyIdentity) || (defaultLineShader == EmptyIdentity))
        {
            NSLog(@"SceneRendererES2: No valid triangle or line shader.  Giving up.");
            return;
        }
        
        WhirlyKit::RendererFrameInfo frameInfo;
        frameInfo.oglVersion = kEAGLRenderingAPIOpenGLES2;
        frameInfo.sceneRenderer = this;
        frameInfo.theView = theView;
        frameInfo.modelTrans = modelTrans;
        frameInfo.scene = scene;
//        frameInfo.frameLen = duration;
        frameInfo.currentTime = CFAbsoluteTimeGetCurrent();
        frameInfo.projMat = projMat;
        frameInfo.mvpMat = mvpMat;
        frameInfo.viewModelNormalMat = modelAndViewNormalMat;
        frameInfo.viewAndModelMat = modelAndViewMat;
        frameInfo.lights = lights;
        frameInfo.stateOpt = renderStateOptimizer;
		
        if (perfInterval > 0)
            perfTimer.startTiming("Scene processing");
        
        // Let the active models to their thing
        // That thing had better not take too long
        for (NSObject<WhirlyKitActiveModel> *activeModel in scene->activeModels)
            [activeModel updateForFrame:frameInfo];
        
        if (perfInterval > 0)
            perfTimer.addCount("Scene changes", scene->changeRequests.size());
        
		// Merge any outstanding changes into the scenegraph
		// Or skip it if we don't acquire the lock
		scene->processChanges(super.theView,self);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Scene processing");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Culling");
		
		// We need a reverse of the eye vector in model space
		// We'll use this to determine what's pointed away
		Eigen::Matrix4f modelTransInv = modelTrans.inverse();
		Vector4f eyeVec4 = modelTransInv * Vector4f(0,0,1,0);
		Vector3f eyeVec3(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
        frameInfo.eyeVec = eyeVec3;
        Eigen::Matrix4f fullTransInv = modelAndViewMat.inverse();
        Vector4f fullEyeVec4 = fullTransInv * Vector4f(0,0,1,0);
        Vector3f fullEyeVec3(fullEyeVec4.x(),fullEyeVec4.y(),fullEyeVec4.z());
        frameInfo.fullEyeVec = -fullEyeVec3;
        frameInfo.heightAboveSurface = 0.0;
        // Note: Should deal with map view as well
        if (globeView)
            frameInfo.heightAboveSurface = globeView.heightAboveSurface;
		
        // If we're looking at a globe, run the culling
        std::set<DrawableRef> toDraw;
        int drawablesConsidered = 0;
        CullTree *cullTree = scene->getCullTree();
        // Recursively search for the drawables that overlap the screen
        Mbr screenMbr;
        // Stretch the screen MBR a little for safety
        screenMbr.addPoint(Point2f(-ScreenOverlap*framebufferWidth,-ScreenOverlap*framebufferHeight));
        screenMbr.addPoint(Point2f((1+ScreenOverlap)*framebufferWidth,(1+ScreenOverlap)*framebufferHeight));
        findDrawables(cullTree->getTopCullable(),globeView,Point2f(framebufferWidth,framebufferHeight),&modelTrans4d,eyeVec3 ,frameInfo,screenMbr,true,&toDraw,&drawablesConsidered);
        
        // Turn these drawables in to a vector
		std::vector<Drawable *> drawList;
        //		drawList.reserve(toDraw.size());
		for (std::set<DrawableRef>::iterator it = toDraw.begin();
			 it != toDraw.end(); ++it)
        {
            Drawable *theDrawable = it->get();
            if (theDrawable)
                drawList.push_back(theDrawable);
            else
                NSLog(@"Bad drawable coming from cull tree.");
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Culling");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Generators - generate");
        
        // Now ask our generators to make their drawables
        // Note: Not doing any culling here
        //       And we should reuse these Drawables
        std::vector<DrawableRef> generatedDrawables,screenDrawables;
        const GeneratorSet *generators = scene->getGenerators();
        for (GeneratorSet::iterator it = generators->begin();
             it != generators->end(); ++it)
            (*it)->generateDrawables(&frameInfo, generatedDrawables, screenDrawables);
        
        // Add the generated drawables and sort them all together
        for (unsigned int ii=0;ii<generatedDrawables.size();ii++)
        {
            Drawable *theDrawable = generatedDrawables[ii].get();
            if (theDrawable)
                drawList.push_back(theDrawable);
        }
        bool sortLinesToEnd = (zBufferMode == zBufferOffDefault);
        std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(super.sortAlphaToEnd,sortLinesToEnd,frameInfo));
        
        if (perfInterval > 0)
        {
            perfTimer.addCount("Drawables considered", drawablesConsidered);
            perfTimer.addCount("Cullables", cullTree->getCount());
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Generators - generate");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Draw Execution");
        
        SimpleIdentity curProgramId = EmptyIdentity;
		
        bool depthMaskOn = (zBufferMode == zBufferOn);
		for (unsigned int ii=0;ii<drawList.size();ii++)
		{
			Drawable *drawable = drawList[ii];
            
            // The first time we hit an explicitly alpha drawable
            //  turn off the depth buffer
            if (depthBufferOffForAlpha && !(zBufferMode == zBufferOffDefault))
            {
                if (depthMaskOn && depthBufferOffForAlpha && drawable->hasAlpha(frameInfo))
                {
                    depthMaskOn = false;
                    renderStateOptimizer->setEnableDepthTest(false);
                }
            }
            
            // For this mode we turn the z buffer off until we get a request to turn it on
            if (zBufferMode == zBufferOffDefault)
            {
                if (drawable->getRequestZBuffer())
                {
                    renderStateOptimizer->setDepthFunc(GL_LESS);
                    depthMaskOn = true;
                } else {
                    renderStateOptimizer->setDepthFunc(GL_ALWAYS);
                }
            }

            // If we're drawing lines or points we don't want to update the z buffer
            if (super.zBufferMode != zBufferOff)
            {
                if (drawable->getWriteZbuffer())
                    renderStateOptimizer->setDepthMask(GL_TRUE);
                else
                    renderStateOptimizer->setDepthMask(GL_FALSE);
            }
            
            // If it has a local transform, apply that
            const Matrix4d *localMat = drawable->getMatrix();
            if (localMat)
            {
                Eigen::Matrix4d newMvpMat = projMat4d * (viewTrans4d * (modelTrans4d * (*localMat)));
                Eigen::Matrix4f newMvpMat4f = Matrix4dToMatrix4f(newMvpMat);
                frameInfo.mvpMat = newMvpMat4f;
            }
            
            // Figure out the program to use for drawing
            SimpleIdentity drawProgramId = drawable->getProgram();
            if (drawProgramId == EmptyIdentity)
                drawProgramId = defaultTriShader;
            if (drawProgramId != curProgramId)
            {
                curProgramId = drawProgramId;
                OpenGLES2Program *program = scene->getProgram(drawProgramId);
                if (program)
                {
//                    [renderStateOptimizer setUseProgram:program->getProgram()];
                    glUseProgram(program->getProgram());
                    // Assign the lights if we need to
                    if (program->hasLights() && ([lights count] > 0))
                        program->setLights(lights, lightsLastUpdated, defaultMat, frameInfo.mvpMat);
                    // Explicitly turn the lights on
                    program->setUniform(kWKOGLNumLights, (int)[lights count]);

                    frameInfo.program = program;
                }
            }
            if (drawProgramId == EmptyIdentity)
                continue;
            
            // Draw using the given program
            drawable->draw(frameInfo,scene);
            
            // If we had a local matrix, set the frame info back to the general one
            if (localMat)
                frameInfo.mvpMat = mvpMat;
            
            numDrawables++;
            if (perfInterval > 0)
            {
                // Note: Need a better way to track buffer ID growth
//                BasicDrawable *basicDraw = dynamic_cast<BasicDrawable *>(drawable);
//                if (basicDraw)
//                    perfTimer.addCount("Buffer IDs", basicDraw->getPointBuffer());
            }
		}
        
        if (perfInterval > 0)
            perfTimer.addCount("Drawables drawn", numDrawables);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Draw Execution");
        
        // Anything generated needs to be cleaned up
        generatedDrawables.clear();
        drawList.clear();
        
        if (perfInterval > 0)
            perfTimer.startTiming("Generators - Draw 2D");
        
        // Now for the 2D display
        if (!screenDrawables.empty())
        {
            curProgramId = EmptyIdentity;
            
            renderStateOptimizer->setEnableDepthTest(false);
            // Sort by draw priority (and alpha, I guess)
            for (unsigned int ii=0;ii<screenDrawables.size();ii++)
            {
                Drawable *theDrawable = screenDrawables[ii].get();
                if (theDrawable)
                    drawList.push_back(theDrawable);
                else
                    NSLog(@"Bad drawable coming from generator.");
            }
            std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(false,false,frameInfo));

            // Build an orthographic projection
            // We flip the vertical axis and spread the window out (0,0)->(width,height)
            Eigen::Matrix4f orthoMat = Matrix4f::Identity();
            Vector3f delta(framebufferWidth,-framebufferHeight,2.0);
            orthoMat(0,0) = 2.0f / delta.x();
            orthoMat(0,3) = -(framebufferWidth) / delta.x();
            orthoMat(1,1) = 2.0f / delta.y();
            orthoMat(1,3) = -framebufferHeight / delta.y();
            orthoMat(2,2) = -2.0f / delta.z();
            orthoMat(2,3) = 0.0f;
            frameInfo.mvpMat = orthoMat;
            // Turn off lights
            frameInfo.lights = nil;
            
            for (unsigned int ii=0;ii<drawList.size();ii++)
            {
                Drawable *drawable = drawList[ii];
                
                if (drawable->isOn(frameInfo))
                {
                    // Figure out the program to use for drawing
                    SimpleIdentity drawProgramId = drawable->getProgram();
                    if (drawProgramId == EmptyIdentity)
                        drawProgramId = defaultTriShader;
                    if (drawProgramId != curProgramId)
                    {
                        curProgramId = drawProgramId;
                        OpenGLES2Program *program = scene->getProgram(drawProgramId);
                        if (program)
                        {
//                            [renderStateOptimizer setUseProgram:program->getProgram()];
                            glUseProgram(program->getProgram());
                            // Explicitly turn the lights off
                            program->setUniform(kWKOGLNumLights, 0);
                            frameInfo.program = program;
                        }
                    }

                    drawable->draw(frameInfo,scene);
                    numDrawables++;
                }
            }
            
            screenDrawables.clear();
            drawList.clear();
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Generators - Draw 2D");
    }
    
//    if (perfInterval > 0)
//        perfTimer.startTiming("glFinish");
    
//    glFlush();
//    glFinish();
    
//    if (perfInterval > 0)
//        perfTimer.stopTiming("glFinish");
    
    if (perfInterval > 0)
        perfTimer.startTiming("Present Renderbuffer");
    
    // Explicitly discard the depth buffer
    const GLenum discards[]  = {GL_DEPTH_ATTACHMENT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER,1,discards);
    CheckGLError("SceneRendererES2: glDiscardFramebufferEXT");

    if (!renderSetup)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        CheckGLError("SceneRendererES2: glBindRenderbuffer");
    }

    [context presentRenderbuffer:GL_RENDERBUFFER];
    CheckGLError("SceneRendererES2: presentRenderbuffer");

    if (perfInterval > 0)
        perfTimer.stopTiming("Present Renderbuffer");
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Frame");
    
	// Update the frames per sec
	if (super.perfInterval > 0 && frameCount > perfInterval)
	{
        CFTimeInterval now = CFAbsoluteTimeGetCurrent();
		NSTimeInterval howLong =  now - frameCountStart;;
		super.framesPerSec = frameCount / howLong;
		frameCountStart = now;
		frameCount = 0;
        
        NSLog(@"---Rendering Performance---");
        NSLog(@" Frames per sec = %.2f",super.framesPerSec);
        perfTimer.log();
        perfTimer.clear();
	}
    
    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
    
    renderSetup = true;
}

@end
