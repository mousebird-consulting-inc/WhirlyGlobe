/*
 *  SceneRendererES2.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
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

#import "Platform.h"
#import "SceneRendererES2.h"
// Note: Porting
//#import "UIColor+Stuff.h"
#import "GLUtils.h"
#import "DefaultShaderPrograms.h"
#import "MaplyView.h"
// Note: Porting
//#import "UIImage+Stuff.h"
//#import "NSDictionary+Stuff.h"
//#import "NSString+Stuff.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

// Keep track of a drawable and the MVP we're supposed to use with it
class DrawableContainer
{
public:
    DrawableContainer(Drawable *draw) : drawable(draw) { mvpMat = mvpMat.Identity(); mvMat = mvMat.Identity();  mvNormalMat = mvNormalMat.Identity(); }
    DrawableContainer(Drawable *draw,Matrix4d mvpMat,Matrix4d mvMat,Matrix4d mvNormalMat) : drawable(draw), mvpMat(mvpMat), mvMat(mvMat), mvNormalMat(mvNormalMat) { }
    
    Drawable *drawable;
    Matrix4d mvpMat,mvMat,mvNormalMat;
};

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
    bool operator()(const DrawableContainer &conA, const DrawableContainer &conB)
    {
        Drawable *a = conA.drawable;
        Drawable *b = conB.drawable;
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

SceneRendererES2::SceneRendererES2()
//: SceneRendererES(kEAGLRenderingAPIOpenGLES2), renderStateOptimizer(NULL), renderSetup(false)
// Note: Porting
: SceneRendererES(2), renderStateOptimizer(NULL), renderSetup(false), extraFrameDrawn(false)
{
    // Add a simple default light
    WhirlyKitDirectionalLight *light = new WhirlyKitDirectionalLight();
    light->setPos(Vector3f(0.75,0.5, -1.0));
    light->setViewDependent(true);
    light->setAmbient(Vector4f(0.6,0.6,0.6,1.0));
    light->setDiffuse(Vector4f(0.5,0.5,0.5,1.0));
    light->setSpecular(Vector4f(0,0,0,0));
    lightsLastUpdated = 0;
    addLight(light);

   // And a basic material
    setDefaultMaterial(new WhirlyKitMaterial());

    lightsLastUpdated = TimeGetCurrent();
    
    // Note: Porting
//    frameRenderingSemaphore = dispatch_semaphore_create(1);
//    contextQueue = dispatch_queue_create("rendering queue",DISPATCH_QUEUE_SERIAL);
    
    renderSetup = false;
}

SceneRendererES2::~SceneRendererES2()
{
#if !__ANDROID__
#if __IPHONE_OS_VERSION_MIN_REQUIRED < 60000
    dispatch_release(contextQueue);
#endif
#endif
    if (renderStateOptimizer)
        delete renderStateOptimizer;
    renderStateOptimizer = NULL;
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
    
    SetupDefaultShaders(scene);
    
    lightsLastUpdated = TimeGetCurrent();
}

/// Add a light to the existing set
void SceneRendererES2::addLight(const WhirlyKitDirectionalLight *light)
{
    lights.push_back(*light);
    lightsLastUpdated = TimeGetCurrent();
    triggerDraw = true;
}

/// Replace all the lights at once. nil turns off lighting
void SceneRendererES2::replaceLights(const std::vector<WhirlyKitDirectionalLight> &newLights)
{
    lights.clear();
    for (auto light : newLights)
        lights.push_back(light);
        
    lightsLastUpdated = TimeGetCurrent();
    triggerDraw = true;
}

void SceneRendererES2::setDefaultMaterial(WhirlyKitMaterial *mat)
{
    defaultMat = mat;
    lightsLastUpdated = TimeGetCurrent();
    triggerDraw = true;
}

void SceneRendererES2::setClearColor(const RGBAColor &color)
{
    clearColor = color;
    renderSetup = false;
}

void SceneRendererES2::processScene()
{
    if (!scene)
        return;
    
    scene->processChanges(theView,this);
}

// Make the screen a bit bigger for testing
static const float ScreenOverlap = 0.1;

void SceneRendererES2::render()
{
    if (!scene || !theView)
        return;
    
    frameCount++;
    
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
        return;

    if (!renderStateOptimizer)
        renderStateOptimizer = new OpenGLStateOptimizer();

	theView->animate();

    // Decide if we even need to draw
    if (!scene->hasChanges() && !viewDidChange())
    {
        if (!extraFrameMode)
            return;
        if (extraFrameDrawn)
            return;
        extraFrameDrawn = true;
    } else
        extraFrameDrawn = false;
    
    lastDraw = TimeGetCurrent();
        
    if (perfInterval > 0)
        perfTimer.startTiming("Render Frame");
    	
    if (perfInterval > 0)
        perfTimer.startTiming("Render Setup");
    
    if (!renderSetup)
    {
        // Turn on blending
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }

    // See if we're dealing with a globe or map view
    WhirlyGlobe::GlobeView *globeView = dynamic_cast<WhirlyGlobe::GlobeView *>(theView);
    Maply::MapView *mapView = dynamic_cast<Maply::MapView *>(theView);

    if (!renderSetup)
    {
        if (defaultFramebuffer)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
            CheckGLError("SceneRendererES2: glBindFramebuffer");
        }
        glViewport(0, 0, framebufferWidth,framebufferHeight);
        CheckGLError("SceneRendererES2: glViewport");
    }

    // Get the model and view matrices
    Eigen::Matrix4d modelTrans4d = theView->calcModelMatrix();
    Eigen::Matrix4f modelTrans = Matrix4dToMatrix4f(modelTrans4d);
    Eigen::Matrix4d viewTrans4d = theView->calcViewMatrix();
    Eigen::Matrix4f viewTrans = Matrix4dToMatrix4f(viewTrans4d);
    
    // Set up a projection matrix
    Eigen::Matrix4d projMat4d = theView->calcProjectionMatrix(Point2f(framebufferWidth,framebufferHeight),0.0);
    
    Eigen::Matrix4f projMat = Matrix4dToMatrix4f(projMat4d);
    Eigen::Matrix4f modelAndViewMat = viewTrans * modelTrans;
    Eigen::Matrix4d modelAndViewMat4d = viewTrans4d * modelTrans4d;
    Eigen::Matrix4d pvMat = projMat4d * viewTrans4d;
    Eigen::Matrix4f mvpMat = projMat * (modelAndViewMat);
    Eigen::Matrix4f mvpNormalMat4f = mvpMat.inverse().transpose();
    Eigen::Matrix4d modelAndViewNormalMat4d = modelAndViewMat4d.inverse().transpose();
    Eigen::Matrix4f modelAndViewNormalMat = Matrix4dToMatrix4f(modelAndViewNormalMat4d);

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
        glClearColor(clearColor.r / 255.0, clearColor.g / 255.0, clearColor.b / 255.0, clearColor.a / 255.0);
        CheckGLError("SceneRendererES2: glClearColor");
    }
//    if (defaultFramebuffer)
//    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CheckGLError("SceneRendererES2: glClear");
//    }

    if (!renderSetup)
    {
        glEnable(GL_CULL_FACE);
        CheckGLError("SceneRendererES2: glEnable(GL_CULL_FACE)");
    }
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Setup");
    
	if (scene)
	{
        Point2f frameSize(framebufferWidth,framebufferHeight);
		int numDrawables = 0;
        
        SimpleIdentity defaultTriShader = scene->getProgramIDBySceneName(kSceneDefaultTriShader);
        SimpleIdentity defaultLineShader = scene->getProgramIDBySceneName(kSceneDefaultLineShader);
        if ((defaultTriShader == EmptyIdentity) || (defaultLineShader == EmptyIdentity))
        {
            // Note: Porting
//            NSLog(@"SceneRendererES2: No valid triangle or line shader.  Giving up.");
            return;
        }
        
        WhirlyKit::RendererFrameInfo baseFrameInfo;
        // Note: Porting
//        baseFrameInfo.oglVersion = kEAGLRenderingAPIOpenGLES2;
        baseFrameInfo.oglVersion = 2;
        baseFrameInfo.sceneRenderer = this;
        baseFrameInfo.theView = theView;
        baseFrameInfo.viewTrans = viewTrans;
        baseFrameInfo.viewTrans4d = viewTrans4d;
        baseFrameInfo.modelTrans = modelTrans;
        baseFrameInfo.modelTrans4d = modelTrans4d;
        baseFrameInfo.scene = scene;
//        frameInfo.frameLen = duration;
        baseFrameInfo.currentTime = TimeGetCurrent();
        baseFrameInfo.projMat = projMat;
        baseFrameInfo.projMat4d = projMat4d;
        baseFrameInfo.mvpMat = mvpMat;
        baseFrameInfo.mvpNormalMat = mvpNormalMat4f;
        baseFrameInfo.viewModelNormalMat = modelAndViewNormalMat;
        baseFrameInfo.viewAndModelMat = modelAndViewMat;
        baseFrameInfo.viewAndModelMat4d = modelAndViewMat4d;
        theView->getOffsetMatrices(baseFrameInfo.offsetMatrices, frameSize);
        Point2d screenSize = theView->screenSizeInDisplayCoords(frameSize);
        baseFrameInfo.screenSizeInDisplayCoords = screenSize;
        baseFrameInfo.lights = &lights;
        baseFrameInfo.stateOpt = renderStateOptimizer;
		
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
        Vector4d eyeVec4d = modelTrans4d.inverse() * Vector4d(0,0,1,0.0);
        baseFrameInfo.heightAboveSurface = 0.0;
        // Note: Should deal with map view as well
        if (globeView)
            baseFrameInfo.heightAboveSurface = globeView->heightAboveSurface();
        baseFrameInfo.eyePos = Vector3d(eyeVec4d.x(),eyeVec4d.y(),eyeVec4d.z()) * (1.0+baseFrameInfo.heightAboveSurface);

        if (perfInterval > 0)
            perfTimer.startTiming("Scene processing");
        
        // Note: Porting
        // Let the active models to their thing
        // That thing had better not take too long
//        for (NSObject<WhirlyKitActiveModel> *activeModel in scene->activeModels)
//        {
//            [activeModel updateForFrame:baseFrameInfo];
//            // Sometimes this gets reset
//            [EAGLContext setCurrentContext:context];
//        }
        
        if (perfInterval > 0)
            perfTimer.addCount("Scene changes", (int)scene->changeRequests.size());
        
		// Merge any outstanding changes into the scenegraph
		// Or skip it if we don't acquire the lock
		scene->processChanges(theView,this);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Scene processing");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Culling");
		
        // Note: Should deal with map view as well
        if (globeView)
            baseFrameInfo.heightAboveSurface = globeView->heightAboveSurface();
		
		
        // Calculate a good center point for the generated drawables
        Point2f screenPt(frameSize.x()/2.0, frameSize.y()/2.0);
        if (globeView)
        {
            Point3d hit;
            if (globeView->pointOnSphereFromScreen(screenPt, &modelAndViewMat4d, frameSize, &hit, true))
                baseFrameInfo.dispCenter = hit;
            else
                baseFrameInfo.dispCenter = Point3d(0,0,0);
        } else {
            Point3d hit;
            if (mapView->pointOnPlaneFromScreen(screenPt,&modelAndViewMat4d,frameSize,&hit,false))
                baseFrameInfo.dispCenter = hit;
            else
                baseFrameInfo.dispCenter = Point3d(0,0,0);
        }
		
        // Work through the available offset matrices (only 1 if we're not wrapping)
        std::vector<Matrix4d> &offsetMats = baseFrameInfo.offsetMatrices;
        // Turn these drawables in to a vector
        std::vector<DrawableContainer> drawList;
        std::vector<DrawableRef> screenDrawables;
        std::vector<DrawableRef> generatedDrawables;
        std::vector<Matrix4d> mvpMats;
        std::vector<Matrix4f> mvpMats4f;
        mvpMats.resize(offsetMats.size());
        mvpMats4f.resize(offsetMats.size());
        for (unsigned int off=0;off<offsetMats.size();off++)
        {
            WhirlyKit::RendererFrameInfo offFrameInfo(baseFrameInfo);
            // Tweak with the appropriate offset matrix
            modelAndViewMat4d = viewTrans4d * offsetMats[off] * modelTrans4d;
            pvMat = projMat4d * viewTrans4d * offsetMats[off];
            modelAndViewMat = Matrix4dToMatrix4f(modelAndViewMat4d);
            mvpMats[off] = projMat4d * modelAndViewMat4d;
            mvpMats4f[off] = Matrix4dToMatrix4f(mvpMats[off]);
            modelAndViewNormalMat4d = modelAndViewMat4d.inverse().transpose();
            modelAndViewNormalMat = Matrix4dToMatrix4f(modelAndViewNormalMat4d);
            Matrix4d &thisMvpMat = mvpMats[off];
            offFrameInfo.mvpMat = mvpMats4f[off];
            mvpNormalMat4f = Matrix4dToMatrix4f(mvpMats[off].inverse().transpose());
            offFrameInfo.mvpNormalMat = mvpNormalMat4f;
            offFrameInfo.viewModelNormalMat = modelAndViewNormalMat;
            offFrameInfo.viewAndModelMat4d = modelAndViewMat4d;
            offFrameInfo.viewAndModelMat = modelAndViewMat;
            Matrix4f pvMat4f = Matrix4dToMatrix4f(pvMat);
            offFrameInfo.pvMat = pvMat4f;
            offFrameInfo.pvMat4d = pvMat;
            
            // If we're looking at a globe, run the culling
            int drawablesConsidered = 0;
            int cullTreeCount = 0;
            if (doCulling)
            {
                std::set<DrawableRef> toDraw;
                CullTree *cullTree = scene->getCullTree();
                // Recursively search for the drawables that overlap the screen
                Mbr screenMbr;
                // Stretch the screen MBR a little for safety
                screenMbr.addPoint(Point2f(-ScreenOverlap*framebufferWidth,-ScreenOverlap*framebufferHeight));
                screenMbr.addPoint(Point2f((1+ScreenOverlap)*framebufferWidth,(1+ScreenOverlap)*framebufferHeight));
                findDrawables(cullTree->getTopCullable(),globeView,frameSize,&modelTrans4d,eyeVec3,&offFrameInfo,screenMbr,true,&toDraw,&drawablesConsidered);
                
                //		drawList.reserve(toDraw.size());
                for (std::set<DrawableRef>::iterator it = toDraw.begin();
                     it != toDraw.end(); ++it)
                {
                    Drawable *theDrawable = it->get();
                    if (theDrawable)
                    {
                        const Matrix4d *localMat = theDrawable->getMatrix();
                        if (localMat)
                        {
                            Eigen::Matrix4d newMvpMat = projMat4d * viewTrans4d * offsetMats[off] * modelTrans4d * (*localMat);
                            Eigen::Matrix4d newMvMat = viewTrans4d * offsetMats[off] * modelTrans4d * (*localMat);
                            Eigen::Matrix4d newMvNormalMat = newMvMat.inverse().transpose();
                            drawList.push_back(DrawableContainer(theDrawable,newMvpMat,newMvMat,newMvNormalMat));
                        } else
                            drawList.push_back(DrawableContainer(theDrawable,thisMvpMat,modelAndViewMat4d,modelAndViewNormalMat4d));
                    } else
                        fprintf(stderr,"Bad drawable coming from cull tree.");
                }
                cullTreeCount = cullTree->getCount();
            } else {
                DrawableRefSet rawDrawables = scene->getDrawables();
                for (DrawableRefSet::iterator it = rawDrawables.begin(); it != rawDrawables.end(); ++it)
                {
                    Drawable *theDrawable = it->get();
                    if (theDrawable->isOn(&offFrameInfo))
                    {
                        const Matrix4d *localMat = theDrawable->getMatrix();
                        if (localMat)
                        {
                            Eigen::Matrix4d newMvpMat = projMat4d * viewTrans4d * offsetMats[off] * modelTrans4d * (*localMat);
                            Eigen::Matrix4d newMvMat = viewTrans4d * offsetMats[off] * modelTrans4d * (*localMat);
                            Eigen::Matrix4d newMvNormalMat = newMvMat.inverse().transpose();
                            drawList.push_back(DrawableContainer(theDrawable,newMvpMat,newMvMat,newMvNormalMat));
                        } else
                            drawList.push_back(DrawableContainer(theDrawable,thisMvpMat,modelAndViewMat4d,modelAndViewNormalMat4d));
                    }
                }
            }
            
            
            if (perfInterval > 0)
                perfTimer.stopTiming("Culling");
            
            if (perfInterval > 0)
                perfTimer.startTiming("Generators - generate");

            // Run the generators only once, they have to be aware of multiple offset matrices
            if (off == offsetMats.size()-1)
            {
                // Now ask our generators to make their drawables
                // Note: Not doing any culling here
                //       And we should reuse these Drawables
                const GeneratorSet *generators = scene->getGenerators();
                for (GeneratorSet::iterator it = generators->begin();
                     it != generators->end(); ++it)
                    (*it)->generateDrawables(&baseFrameInfo, generatedDrawables, screenDrawables);
                
                // Add the generated drawables and sort them all together
                for (unsigned int ii=0;ii<generatedDrawables.size();ii++)
                {
                    Drawable *theDrawable = generatedDrawables[ii].get();
                    if (theDrawable)
                        drawList.push_back(DrawableContainer(theDrawable,thisMvpMat,modelAndViewMat4d,modelAndViewNormalMat4d));
                }
                bool sortLinesToEnd = (zBufferMode == zBufferOffDefault);
                std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(sortAlphaToEnd,sortLinesToEnd,&baseFrameInfo));
            }
            
            if (perfInterval > 0)
            {
                perfTimer.addCount("Drawables considered", drawablesConsidered);
                perfTimer.addCount("Cullables", cullTreeCount);
            }
            
            if (perfInterval > 0)
                perfTimer.stopTiming("Generators - generate");
        }
        
        if (perfInterval > 0)
            perfTimer.startTiming("Draw Execution");
        
        SimpleIdentity curProgramId = EmptyIdentity;
		
        bool depthMaskOn = (zBufferMode == zBufferOn);
        for (unsigned int ii=0;ii<drawList.size();ii++)
        {
            DrawableContainer &drawContain = drawList[ii];
            
            // The first time we hit an explicitly alpha drawable
            //  turn off the depth buffer
            if (depthBufferOffForAlpha && !(zBufferMode == zBufferOffDefault))
            {
                if (depthMaskOn && depthBufferOffForAlpha && drawContain.drawable->hasAlpha(&baseFrameInfo))
                {
                    depthMaskOn = false;
                    renderStateOptimizer->setEnableDepthTest(false);
                }
            }
            
            // For this mode we turn the z buffer off until we get a request to turn it on
            if (zBufferMode == zBufferOffDefault)
            {
                if (drawContain.drawable->getRequestZBuffer())
                {
                    renderStateOptimizer->setDepthFunc(GL_LESS);
                    depthMaskOn = true;
                } else {
                    renderStateOptimizer->setDepthFunc(GL_ALWAYS);
                }
            }

            // If we're drawing lines or points we don't want to update the z buffer
            if (zBufferMode != zBufferOff)
            {
                if (drawContain.drawable->getWriteZbuffer())
                    renderStateOptimizer->setDepthMask(GL_TRUE);
                else
                    renderStateOptimizer->setDepthMask(GL_FALSE);
            }
            
            // Set up transforms to use right now
            Matrix4f currentMvpMat = Matrix4dToMatrix4f(drawContain.mvpMat);
            Matrix4f currentMvMat = Matrix4dToMatrix4f(drawContain.mvMat);
            Matrix4f currentMvNormalMat = Matrix4dToMatrix4f(drawContain.mvNormalMat);
            baseFrameInfo.mvpMat = currentMvpMat;
            baseFrameInfo.viewAndModelMat = currentMvMat;
            baseFrameInfo.viewModelNormalMat = currentMvNormalMat;
            
            // Figure out the program to use for drawing
            SimpleIdentity drawProgramId = drawContain.drawable->getProgram();
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
                    // Note: Porting
                    if (program->hasLights() && (lights.size() > 0))
                        program->setLights(lights, lightsLastUpdated, defaultMat, baseFrameInfo.mvpMat);
                    // Explicitly turn the lights on
                    program->setUniform(kWKOGLNumLights, (int)lights.size());

                    baseFrameInfo.program = program;
                }
            }
            if (drawProgramId == EmptyIdentity)
                continue;
            
            // Run any tweakers right here
            drawContain.drawable->runTweakers(&baseFrameInfo);
                        
            // Draw using the given program
            drawContain.drawable->draw(&baseFrameInfo,scene);
            
            // If we had a local matrix, set the frame info back to the general one
//            if (localMat)
//                frameInfo.mvpMat = mvpMat;
            
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
                else {
                    // Note: Porting
//                    NSLog(@"Bad drawable coming from generator.");
                }
            }
            std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(false,false,&baseFrameInfo));

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
            baseFrameInfo.lights->clear();
            
            for (unsigned int ii=0;ii<drawList.size();ii++)
            {
                DrawableContainer &drawContain = drawList[ii];
                
                if (drawContain.drawable->isOn(&baseFrameInfo))
                {
                    // Figure out the program to use for drawing
                    SimpleIdentity drawProgramId = drawContain.drawable->getProgram();
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

                    drawContain.drawable->draw(&baseFrameInfo,scene);
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

    // Note: Porting.  This seems to bother Android
//    glFlush();
//    glFinish();
    
//    if (perfInterval > 0)
//        perfTimer.stopTiming("glFinish");
    
    if (perfInterval > 0)
        perfTimer.startTiming("Present Renderbuffer");
    
    // Explicitly discard the depth buffer
    // Note: Porting
//    const GLenum discards[]  = {GL_DEPTH_ATTACHMENT};
//    glDiscardFramebufferEXT(GL_FRAMEBUFFER,1,discards);
//    CheckGLError("SceneRendererES2: glDiscardFramebufferEXT");

    if (!renderSetup)
    {
        if (colorRenderbuffer)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
            CheckGLError("SceneRendererES2: glBindRenderbuffer");
        }
    }

    if (perfInterval > 0)
        perfTimer.stopTiming("Present Renderbuffer");
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Frame");
    
	// Update the frames per sec
	if (perfInterval > 0 && frameCount > perfInterval)
	{
        TimeInterval now = TimeGetCurrent();
		TimeInterval howLong =  now - frameCountStart;;
		framesPerSec = frameCount / howLong;
		frameCountStart = now;
		frameCount = 0;
        
        // Note: Porting
        perfTimer.report("---Rendering Performance---");
        char fpsReport[1024];
        sprintf(fpsReport,"Frames per sec = %.2f",framesPerSec);
        perfTimer.report(fpsReport);
        perfTimer.log();
        perfTimer.clear();
	}
        
    renderSetup = true;
}
