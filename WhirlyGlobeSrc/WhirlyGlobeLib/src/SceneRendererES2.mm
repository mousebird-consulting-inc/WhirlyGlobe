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
    DrawListSortStruct2(bool useAlpha,bool useZBuffer,WhirlyKitRendererFrameInfo *frameInfo) : useAlpha(useAlpha), useZBuffer(useZBuffer), frameInfo(frameInfo)
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
    WhirlyKitRendererFrameInfo * __unsafe_unretained frameInfo;
};
    
}

@implementation WhirlyKitFrameMessage
@end

@implementation WhirlyKitSceneRendererES2
{
    NSMutableArray *lights;
    CFTimeInterval lightsLastUpdated;
    WhirlyKitMaterial *defaultMat;
    dispatch_queue_t contextQueue;
    dispatch_semaphore_t frameRenderingSemaphore;
    bool renderSetup;
    WhirlyKitOpenGLStateOptimizer *renderStateOptimizer;
    std::set<__weak NSObject<WhirlyKitFrameBoundaryObserver> *> frameObservers;
}

- (id) init
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

    self = [super initWithOpenGLESVersion:kEAGLRenderingAPIOpenGLES2];
    lights = [NSMutableArray array];
    
    // Add a simple default light
    WhirlyKitDirectionalLight *light = [[WhirlyKitDirectionalLight alloc] init];
    [light setPos:Vector3f(0.75, 0.5, -1.0)];
    light.viewDependent = true;
    light.ambient = Vector4f(0.6, 0.6, 0.6, 1.0);
    light.diffuse = Vector4f(0.5, 0.5, 0.5, 1.0);
    light.specular = Vector4f(0, 0, 0, 0);
    [self addLight:light];

    // And a basic material
    [self setDefaultMaterial:[[WhirlyKitMaterial alloc] init]];

    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    
    frameRenderingSemaphore = dispatch_semaphore_create(1);
    contextQueue = dispatch_queue_create("rendering queue",DISPATCH_QUEUE_SERIAL);
    
    renderSetup = false;

    // Note: Try to turn this back on at some point
    _dispatchRendering = false;

    return self;
}

- (void) dealloc
{
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 60000
    dispatch_release(contextQueue);
#endif
}

- (void)forceRenderSetup
{
    renderSetup = false;
}

// When the scene is set, we'll compile our shaders
- (void)setScene:(WhirlyKit::Scene *)inScene
{
    [super setScene:inScene];
    super.scene = inScene;

    if (!super.scene)
        return;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != super.context)
        [EAGLContext setCurrentContext:super.context];
    
    SetupDefaultShaders(super.scene);
    
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();

    if (oldContext != super.context)
        [EAGLContext setCurrentContext:oldContext];
}

/// Add a light to the existing set
- (void)addLight:(WhirlyKitDirectionalLight *)light
{
    if (!lights)
        lights = [NSMutableArray array];
    [lights addObject:light];
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    super.triggerDraw = true;
}

/// Replace all the lights at once. nil turns off lighting
- (void)replaceLights:(NSArray *)inLights
{
    lights = [NSMutableArray arrayWithArray:inLights];
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    super.triggerDraw = true;
}

- (void)setDefaultMaterial:(WhirlyKitMaterial *)mat
{
    defaultMat = mat;
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    super.triggerDraw = true;
}

- (void) setClearColor:(UIColor *)color
{
    _clearColor = [color asRGBAColor];
    renderSetup = false;
}

- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer
{
    renderSetup = false;
    bool ret = [super resizeFromLayer:layer];
    
    return ret;
}

- (void)addFrameObserver:(NSObject<WhirlyKitFrameBoundaryObserver> *)observer
{
    @synchronized(self)
    {
        frameObservers.insert(observer);
    }
}

- (void)removeFrameObserver:(NSObject<WhirlyKitFrameBoundaryObserver> *)observer
{
    @synchronized(self)
    {
        auto it = frameObservers.find(observer);
        if (it != frameObservers.end())
            frameObservers.erase(it);
    }
}

// Make the screen a bit bigger for testing
static const float ScreenOverlap = 0.1;

- (void) render:(CFTimeInterval)duration
{
    // Let anyone who cares know the frame draw is starting
    WhirlyKitFrameMessage *frameMsg = [[WhirlyKitFrameMessage alloc] init];
    frameMsg.frameStart = CFAbsoluteTimeGetCurrent();
    frameMsg.frameInterval = duration;
    frameMsg.renderer = self;
    @synchronized(self)
    {
        for (auto it : frameObservers)
        {
            [it frameStart:frameMsg];
        }
    }

    if (_dispatchRendering)
    {
        if (dispatch_semaphore_wait(frameRenderingSemaphore, DISPATCH_TIME_NOW) != 0)
            return;
        
        dispatch_async(contextQueue,
                       ^{
                           [self renderAsync];
                           dispatch_semaphore_signal(frameRenderingSemaphore);
                       });
    } else
        [self renderAsync];
}

- (void) renderAsync
{
    Scene *scene = super.scene;
    
    if (!scene)
        return;
    
    frameCount++;
    
    if (super.framebufferWidth <= 0 || super.framebufferHeight <= 0)
        return;

    if (!renderStateOptimizer)
        renderStateOptimizer = [[WhirlyKitOpenGLStateOptimizer alloc] init];

	[super.theView animate];

    // Decide if we even need to draw
    if (!scene->hasChanges() && ![self viewDidChange])
        return;
    
    NSTimeInterval perfInterval = super.perfInterval;
    
    lastDraw = CFAbsoluteTimeGetCurrent();
        
    if (perfInterval > 0)
        perfTimer.startTiming("Render Frame");
    	
    if (perfInterval > 0)
        perfTimer.startTiming("Render Setup");
    
    EAGLContext *context = super.context;
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
    if ([super.theView isKindOfClass:[WhirlyGlobeView class]])
        globeView = (WhirlyGlobeView *)super.theView;

    GLint framebufferWidth = super.framebufferWidth;
    GLint framebufferHeight = super.framebufferHeight;
    if (!renderSetup)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        CheckGLError("SceneRendererES2: glBindFramebuffer");
        glViewport(0, 0, framebufferWidth,framebufferHeight);
        CheckGLError("SceneRendererES2: glViewport");
    }

    // Get the model and view matrices
    Eigen::Matrix4d modelTrans4d = [super.theView calcModelMatrix];
    Eigen::Matrix4f modelTrans = Matrix4dToMatrix4f(modelTrans4d);
    Eigen::Matrix4d viewTrans4d = [super.theView calcViewMatrix];
    Eigen::Matrix4f viewTrans = Matrix4dToMatrix4f(viewTrans4d);
    
    // Set up a projection matrix
    Point2f frameSize(framebufferWidth,framebufferHeight);
    Eigen::Matrix4d projMat4d = [super.theView calcProjectionMatrix:frameSize margin:0.0];
    
    Eigen::Matrix4f projMat = Matrix4dToMatrix4f(projMat4d);
    Eigen::Matrix4f modelAndViewMat = viewTrans * modelTrans;
    Eigen::Matrix4f mvpMat = projMat * (modelAndViewMat);
    Eigen::Matrix4f modelAndViewNormalMat = modelAndViewMat.inverse().transpose();

    switch (super.zBufferMode)
    {
        case zBufferOn:
            [renderStateOptimizer setDepthMask:GL_TRUE];
            [renderStateOptimizer setEnableDepthTest:true];
            [renderStateOptimizer setDepthFunc:GL_LESS];
            break;
        case zBufferOff:
            [renderStateOptimizer setDepthMask:GL_FALSE];
            [renderStateOptimizer setEnableDepthTest:false];
            break;
        case zBufferOffDefault:
            [renderStateOptimizer setDepthMask:GL_TRUE];
            [renderStateOptimizer setEnableDepthTest:true];
            [renderStateOptimizer setDepthFunc:GL_ALWAYS];
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
        
        WhirlyKitRendererFrameInfo *baseFrameInfo = [[WhirlyKitRendererFrameInfo alloc] init];
        baseFrameInfo.oglVersion = kEAGLRenderingAPIOpenGLES2;
        baseFrameInfo.sceneRenderer = self;
        baseFrameInfo.theView = super.theView;
        baseFrameInfo.viewTrans = viewTrans;
        baseFrameInfo.modelTrans = modelTrans;
        baseFrameInfo.scene = scene;
//        baseFrameInfo.frameLen = duration;
        baseFrameInfo.currentTime = CFAbsoluteTimeGetCurrent();
        baseFrameInfo.projMat = projMat;
        baseFrameInfo.mvpMat = mvpMat;
        baseFrameInfo.viewModelNormalMat = modelAndViewNormalMat;
        baseFrameInfo.viewAndModelMat = modelAndViewMat;
        [super.theView getOffsetMatrices:baseFrameInfo.offsetMatrices frameBuffer:frameSize];
        baseFrameInfo.lights = lights;
        baseFrameInfo.stateOpt = renderStateOptimizer;
		
        if (perfInterval > 0)
            perfTimer.startTiming("Scene processing");
        
        // Let the active models to their thing
        // That thing had better not take too long
        for (NSObject<WhirlyKitActiveModel> *activeModel in scene->activeModels)
            [activeModel updateForFrame:baseFrameInfo];
        
        if (perfInterval > 0)
            perfTimer.addCount("Scene changes", (int)scene->changeRequests.size());
        
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
        baseFrameInfo.eyeVec = eyeVec3;
        Eigen::Matrix4f fullTransInv = modelAndViewMat.inverse();
        Vector4f fullEyeVec4 = fullTransInv * Vector4f(0,0,1,0);
        Vector3f fullEyeVec3(fullEyeVec4.x(),fullEyeVec4.y(),fullEyeVec4.z());
        baseFrameInfo.fullEyeVec = -fullEyeVec3;
        baseFrameInfo.heightAboveSurface = 0.0;
        // Note: Should deal with map view as well
        if (globeView)
            baseFrameInfo.heightAboveSurface = globeView.heightAboveSurface;
		      
        // Work through the available offset matrices (only 1 if we're not wrapping)
        std::vector<Matrix4d> &offsetMats = baseFrameInfo.offsetMatrices;
        std::vector<DrawableRef> screenDrawables;
        for (unsigned int off=0;off<offsetMats.size();off++)
        {
            WhirlyKitRendererFrameInfo *offFrameInfo = [[WhirlyKitRendererFrameInfo alloc] initWithFrameInfo:baseFrameInfo];
            // Tweak with the appropriate offset matrix
            modelAndViewMat = viewTrans * modelTrans * Matrix4dToMatrix4f(offsetMats[off]);
            mvpMat = projMat * (modelAndViewMat);
            modelAndViewNormalMat = modelAndViewMat.inverse().transpose();
            offFrameInfo.mvpMat = mvpMat;
            offFrameInfo.viewModelNormalMat = modelAndViewNormalMat;
            offFrameInfo.viewAndModelMat = modelAndViewMat;

            // Turn these drawables in to a vector
            std::vector<Drawable *> drawList;
            
            // If we're looking at a globe, run the culling
            int drawablesConsidered = 0;
            int cullTreeCount = 0;
            if (self.doCulling)
            {
                std::set<DrawableRef> toDraw;
                CullTree *cullTree = scene->getCullTree();
                // Recursively search for the drawables that overlap the screen
                Mbr screenMbr;
                // Stretch the screen MBR a little for safety
                screenMbr.addPoint(Point2f(-ScreenOverlap*framebufferWidth,-ScreenOverlap*framebufferHeight));
                screenMbr.addPoint(Point2f((1+ScreenOverlap)*framebufferWidth,(1+ScreenOverlap)*framebufferHeight));
                [self findDrawables:cullTree->getTopCullable() view:globeView frameSize:Point2f(framebufferWidth,framebufferHeight) modelTrans:&modelTrans4d eyeVec:eyeVec3 frameInfo:offFrameInfo screenMbr:screenMbr topLevel:true toDraw:&toDraw considered:&drawablesConsidered];
                
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
                cullTreeCount = cullTree->getCount();
            } else {
                DrawableRefSet rawDrawables = scene->getDrawables();
                for (DrawableRefSet::iterator it = rawDrawables.begin(); it != rawDrawables.end(); ++it)
                {
                    if ((*it)->isOn(offFrameInfo))
                    drawList.push_back(it->get());
                }
            }
            
            
            if (perfInterval > 0)
            perfTimer.stopTiming("Culling");
            
            if (perfInterval > 0)
            perfTimer.startTiming("Generators - generate");

            // Run the generators only once, they have to be aware of multiple offset matrices
            std::vector<DrawableRef> generatedDrawables,screenDrawables;
            if (off == offsetMats.size()-1)
            {
                // Now ask our generators to make their drawables
                // Note: Not doing any culling here
                //       And we should reuse these Drawables
                const GeneratorSet *generators = scene->getGenerators();
                for (GeneratorSet::iterator it = generators->begin();
                     it != generators->end(); ++it)
                    (*it)->generateDrawables(baseFrameInfo, generatedDrawables, screenDrawables);
                
                // Add the generated drawables and sort them all together
                for (unsigned int ii=0;ii<generatedDrawables.size();ii++)
                {
                    Drawable *theDrawable = generatedDrawables[ii].get();
                    if (theDrawable)
                        drawList.push_back(theDrawable);
                }
                bool sortLinesToEnd = (super.zBufferMode == zBufferOffDefault);
                std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(super.sortAlphaToEnd,sortLinesToEnd,baseFrameInfo));
            }
            
            if (perfInterval > 0)
            {
                perfTimer.addCount("Drawables considered", drawablesConsidered);
                perfTimer.addCount("Cullables", cullTreeCount);
            }
            
            if (perfInterval > 0)
                perfTimer.stopTiming("Generators - generate");
            
            if (perfInterval > 0)
                perfTimer.startTiming("Draw Execution");
            
            SimpleIdentity curProgramId = EmptyIdentity;
        		
            bool depthMaskOn = (super.zBufferMode == zBufferOn);
            for (unsigned int ii=0;ii<drawList.size();ii++)
            {
                Drawable *drawable = drawList[ii];
                
                // The first time we hit an explicitly alpha drawable
                //  turn off the depth buffer
                if (super.depthBufferOffForAlpha && !(super.zBufferMode == zBufferOffDefault))
                {
                    if (depthMaskOn && super.depthBufferOffForAlpha && drawable->hasAlpha(baseFrameInfo))
                    {
                        depthMaskOn = false;
                        [renderStateOptimizer setEnableDepthTest:false];
                    }
                }
                
                // For this mode we turn the z buffer off until we get a request to turn it on
                if (super.zBufferMode == zBufferOffDefault)
                {
                    if (drawable->getRequestZBuffer())
                    {
                        [renderStateOptimizer setDepthFunc:GL_LESS];
                        depthMaskOn = true;
                    } else {
                        [renderStateOptimizer setDepthFunc:GL_ALWAYS];
                    }
                }

                // If we're drawing lines or points we don't want to update the z buffer
                if (super.zBufferMode != zBufferOff)
                {
                    if (drawable->getWriteZbuffer())
                        [renderStateOptimizer setDepthMask:GL_TRUE];
                    else
                        [renderStateOptimizer setDepthMask:GL_FALSE];
                }
                
                // If it has a local transform, apply that
                const Matrix4d *localMat = drawable->getMatrix();
                if (localMat)
                {
                    Eigen::Matrix4d newMvpMat = projMat4d * (viewTrans4d * (modelTrans4d * (*localMat)));
                    Eigen::Matrix4f newMvpMat4f = Matrix4dToMatrix4f(newMvpMat);
                    offFrameInfo.mvpMat = newMvpMat4f;
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
                            program->setLights(lights, lightsLastUpdated, defaultMat, offFrameInfo.mvpMat);
                        // Explicitly turn the lights on
                        program->setUniform(kWKOGLNumLights, (int)[lights count]);

                        offFrameInfo.program = program;
                    }
                }
                if (drawProgramId == EmptyIdentity)
                    continue;
                
                // Draw using the given program
                drawable->draw(offFrameInfo,scene);
                
                // If we had a local matrix, set the frame info back to the general one
                if (localMat)
                    offFrameInfo.mvpMat = mvpMat;
                
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
                
                [renderStateOptimizer setEnableDepthTest:false];
                // Sort by draw priority (and alpha, I guess)
                for (unsigned int ii=0;ii<screenDrawables.size();ii++)
                {
                    Drawable *theDrawable = screenDrawables[ii].get();
                    if (theDrawable)
                        drawList.push_back(theDrawable);
                    else
                        NSLog(@"Bad drawable coming from generator.");
                }
                std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(false,false,baseFrameInfo));

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
                baseFrameInfo.mvpMat = orthoMat;
                // Turn off lights
                baseFrameInfo.lights = nil;
                
                for (unsigned int ii=0;ii<drawList.size();ii++)
                {
                    Drawable *drawable = drawList[ii];
                    
                    if (drawable->isOn(baseFrameInfo))
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
                                baseFrameInfo.program = program;
                            }
                        }

                        drawable->draw(baseFrameInfo,scene);
                        numDrawables++;
                    }
                }
                
                screenDrawables.clear();
                drawList.clear();
            }
        }
    }
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Generators - Draw 2D");
    
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

    // The user wants help with a screen snapshot
    if (_snapshotDelegate)
    {
        // Courtesy: https://developer.apple.com/library/ios/qa/qa1704/_index.html
        NSInteger dataLength = framebufferWidth * framebufferHeight * 4;
        GLubyte *data = (GLubyte*)malloc(dataLength * sizeof(GLubyte));
        
        // Read pixel data from the framebuffer
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
        
        // Create a CGImage with the pixel data
        // If your OpenGL ES content is opaque, use kCGImageAlphaNoneSkipLast to ignore the alpha channel
        // otherwise, use kCGImageAlphaPremultipliedLast
        CGDataProviderRef ref = CGDataProviderCreateWithData(NULL, data, dataLength, NULL);
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        CGImageRef iref = CGImageCreate(framebufferWidth, framebufferHeight, 8, 32, framebufferWidth * 4, colorspace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast,
                                        ref, NULL, true, kCGRenderingIntentDefault);
        
        // OpenGL ES measures data in PIXELS
        // Create a graphics context with the target size measured in POINTS
        NSInteger widthInPoints, heightInPoints;
        if (NULL != UIGraphicsBeginImageContextWithOptions) {
            // On iOS 4 and later, use UIGraphicsBeginImageContextWithOptions to take the scale into consideration
            // Set the scale parameter to your OpenGL ES view's contentScaleFactor
            // so that you get a high-resolution snapshot when its value is greater than 1.0
            CGFloat scale = self.scale;
            widthInPoints = framebufferWidth / scale;
            heightInPoints = framebufferHeight / scale;
            UIGraphicsBeginImageContextWithOptions(CGSizeMake(widthInPoints, heightInPoints), NO, scale);
        }
        else {
            // On iOS prior to 4, fall back to use UIGraphicsBeginImageContext
            widthInPoints = framebufferWidth;
            heightInPoints = framebufferHeight;
            UIGraphicsBeginImageContext(CGSizeMake(widthInPoints, heightInPoints));
        }
        
        CGContextRef cgcontext = UIGraphicsGetCurrentContext();
        
        // UIKit coordinate system is upside down to GL/Quartz coordinate system
        // Flip the CGImage by rendering it to the flipped bitmap context
        // The size of the destination area is measured in POINTS
        CGContextSetBlendMode(cgcontext, kCGBlendModeCopy);
        CGContextDrawImage(cgcontext, CGRectMake(0.0, 0.0, widthInPoints, heightInPoints), iref);
        
        // Retrieve the UIImage from the current context
        UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
        
        UIGraphicsEndImageContext();
        
        // Clean up
        free(data);
        CFRelease(ref);
        CFRelease(colorspace);
        CGImageRelease(iref);
        
        [_snapshotDelegate snapshot:image];
        
        _snapshotDelegate = nil;
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
