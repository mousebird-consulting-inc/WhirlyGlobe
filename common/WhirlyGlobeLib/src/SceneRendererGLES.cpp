/*  SceneRendererGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/13/19.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "SceneRendererGLES.h"
#import "TextureGLES.h"
#import "RenderTargetGLES.h"
#import "BasicDrawableBuilderGLES.h"
#import "BasicDrawableInstanceBuilderGLES.h"
#import "BillboardDrawableBuilderGLES.h"
#import "ScreenSpaceDrawableBuilderGLES.h"
#import "WideVectorDrawableBuilderGLES.h"
#import "ParticleSystemDrawableBuilderGLES.h"
#import "DynamicTextureAtlasGLES.h"
#import "MaplyView.h"
#import "WhirlyKitLog.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

WorkGroupGLES::WorkGroupGLES(GroupType inGroupType)
{
    groupType = inGroupType;
    
    switch (groupType) {
        case Calculation:
            // For calculation we don't really have a render target
            renderTargetContainers.push_back(WorkGroupGLES::makeRenderTargetContainer(nullptr));
            break;
        default:
        case Offscreen:
        case ReduceOps:
        case ScreenRender:
            break;
    }
}

RenderTargetContainerRef WorkGroupGLES::makeRenderTargetContainer(RenderTargetRef renderTarget)
{
    return std::make_shared<RenderTargetContainerGLES>(renderTarget);
}
    
RendererFrameInfoGLES::RendererFrameInfoGLES()
    : glesVersion(0)
{
}
    
SceneRendererGLES::SceneRendererGLES() :
    extraFrameCount(0)
{
    init(); // NOLINT: derived virtual methods not called

    // Calculation shaders
    workGroups.emplace_back(std::make_shared<WorkGroupGLES>(WorkGroup::Calculation));
    // Offscreen target render group
    workGroups.emplace_back(std::make_shared<WorkGroupGLES>(WorkGroup::Offscreen));
    // Middle one for weird stuff
    workGroups.emplace_back(std::make_shared<WorkGroupGLES>(WorkGroup::ReduceOps));
    // Last workgroup is used for on screen rendering
    workGroups.emplace_back(std::make_shared<WorkGroupGLES>(WorkGroup::ScreenRender));

    extraFrameMode = false;
}
    
SceneRenderer::Type SceneRendererGLES::getType() {
    return SceneRenderer::RenderGLES;
}
    
const RenderSetupInfo *SceneRendererGLES::getRenderSetupInfo() const
{
    return &setupInfo;
}

bool SceneRendererGLES::setup(int apiVersion,int sizeX,int sizeY,float inScale)
{
    frameCount = 0;
    framesPerSec = 0.0;
    numDrawables = 0;
    frameCountStart = 0.0;
    frameCountStart = 0.0;
    zBufferMode = zBufferOn;
    clearColor.r = 0;  clearColor.g = 0;  clearColor.b = 0;  clearColor.a = 0;
    perfInterval = -1;
    scene = nullptr;
    scale = inScale;
    theView = nullptr;
    
    // All the animations should work now, except for particle systems
    useViewChanged = true;
    
    extraFrameMode = false;
    
    framebufferWidth = sizeX;
    framebufferHeight = sizeY;
    
    setupInfo.glesVersion = apiVersion;
    
    // We need a texture to draw to in this case
    if (framebufferWidth > 0)
    {
        TextureGLESRef framebufferTexGL = TextureGLESRef(new TextureGLES("Framebuffer Texture"));
        framebufferTexGL->setWidth(framebufferWidth);
        framebufferTexGL->setHeight(framebufferHeight);
        framebufferTexGL->setIsEmptyTexture(true);
        framebufferTexGL->setFormat(TexTypeUnsignedByte);
        framebufferTexGL->createInRenderer(nullptr);
        framebufferTex = framebufferTexGL;
    }
    
    auto defaultTarget = std::make_shared<RenderTargetGLES>(EmptyIdentity);
    defaultTarget->width = sizeX;
    defaultTarget->height = sizeY;
    if (framebufferTex) {
        defaultTarget->setTargetTexture(framebufferTex.get());
        // Note: Should make this optional
        defaultTarget->blendEnable = false;
    } else {
        if (sizeX > 0 && sizeY > 0)
            defaultTarget->init(this,nullptr,EmptyIdentity);
        defaultTarget->blendEnable = true;
    }
    defaultTarget->clearEveryFrame = true;
    renderTargets.push_back(defaultTarget);

    // GL doesn't do anything special for teardown
    teardownInfo = RenderTeardownInfoRef(new RenderTeardownInfo());
    
    return true;
}
    
void SceneRendererGLES::setView(View *newView)
{
    SceneRenderer::setView(newView);
    setupInfo.minZres = newView->calcZbufferRes();
}
    
void SceneRendererGLES::setScene(Scene *newScene)
{
    SceneRenderer::setScene(newScene);
    auto *sceneGL = (SceneGLES *)newScene;
    setupInfo.memManager = sceneGL->getMemManager();
}
    
bool SceneRendererGLES::resize(int sizeX,int sizeY)
{
    // Don't want to deal with it for offscreen rendering
    if (framebufferTex)
        return false;
    
    framebufferWidth = sizeX;
    framebufferHeight = sizeY;
    
    RenderTargetRef defaultTarget = renderTargets.back();
    defaultTarget->width = sizeX;
    defaultTarget->height = sizeY;
    defaultTarget->init(this, nullptr, EmptyIdentity);
    
    // Note: Check this
    return true;
}

SceneRendererGLES::~SceneRendererGLES() = default;

// Keep track of a drawable and the MVP we're supposed to use with it
class DrawableContainer
{
public:
    explicit DrawableContainer(DrawableGLES *draw) :
        drawable(draw),
        mvpMat(Eigen::Matrix4d::Identity()),
        mvMat(Eigen::Matrix4d::Identity()),
        mvNormalMat(Eigen::Matrix4d::Identity())
    {
    }

    DrawableContainer(DrawableGLES *draw,Matrix4d mvpMat,Matrix4d mvMat,Matrix4d mvNormalMat) :
        drawable(draw),
        mvpMat(std::move(mvpMat)),
        mvMat(std::move(mvMat)),
        mvNormalMat(std::move(mvNormalMat))
    {
    }

    DrawableGLES *drawable;
    Matrix4d mvpMat,mvMat,mvNormalMat;
};

// Alpha stuff goes at the end
// Otherwise sort by draw priority
class DrawListSortStruct2
{
public:
    DrawListSortStruct2() = delete;
    DrawListSortStruct2(bool useZBuffer,RendererFrameInfo *frameInfo) : useZBuffer(useZBuffer), frameInfo(frameInfo)
    {
    }
    DrawListSortStruct2(const DrawListSortStruct2 &that) = default;
    DrawListSortStruct2 & operator =(const DrawListSortStruct2 &that)
    {
        if (this != &that)
        {
            useZBuffer= that.useZBuffer;
            frameInfo = that.frameInfo;
        }
        return *this;
    }
    bool operator()(const DrawableContainer &conA, const DrawableContainer &conB) const
    {
        const Drawable *a = conA.drawable;
        const Drawable *b = conB.drawable;

        if (a->getDrawPriority() == b->getDrawPriority())
        {
            if (useZBuffer)
            {
                const bool bufferA = a->getRequestZBuffer();
                const bool bufferB = b->getRequestZBuffer();
                if (bufferA != bufferB)
                    return !bufferA;
            }
        }
        
        return a->getDrawPriority() < b->getDrawPriority();
    }
    
    bool useZBuffer;
    RendererFrameInfo *frameInfo;
};

void SceneRendererGLES::setExtraFrameMode(bool newMode)
{
    extraFrameMode = newMode;
    if (extraFrameMode)
        extraFrameCount = 2;
}
    
bool SceneRendererGLES::hasChanges()
{
    return SceneRenderer::hasChanges();
}

void SceneRendererGLES::render(TimeInterval duration)
{
    if (!scene)
        return;
    
    frameCount++;
        
    theView->animate();
    
    TimeInterval now = scene->getCurrentTime();

    lastDraw = now;
    
    if (perfInterval > 0)
        perfTimer.startTiming("Render Frame");
    
    if (perfInterval > 0)
        perfTimer.startTiming("Render Setup");
    
    //    if (!renderSetup)
    {
        // Turn on blending
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }
    
    // See if we're dealing with a globe or map view
    float overlapMarginX = 0.0;
    if (auto mapView = dynamic_cast<Maply::MapView *>(theView))
    {
        overlapMarginX = (float)scene->getOverlapMargin();
    }
    
    // Get the model and view matrices
    Eigen::Matrix4d modelTrans4d = theView->calcModelMatrix();
    Eigen::Matrix4f modelTrans = Matrix4dToMatrix4f(modelTrans4d);
    Eigen::Matrix4d viewTrans4d = theView->calcViewMatrix();
    Eigen::Matrix4f viewTrans = Matrix4dToMatrix4f(viewTrans4d);
    
    // Set up a projection matrix
    Point2f frameSize(framebufferWidth,framebufferHeight);
    Eigen::Matrix4d projMat4d = theView->calcProjectionMatrix(frameSize,0.0);
    
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
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            break;
        case zBufferOff:
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            break;
        case zBufferOffDefault:
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
            break;
    }
    
    //    if (!renderSetup)
    {
        glEnable(GL_CULL_FACE);
        CheckGLError("SceneRendererES2: glEnable(GL_CULL_FACE)");
    }
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Setup");
    
    if (scene)
    {
        int numDrawables = 0;
        
        RendererFrameInfoGLES baseFrameInfo;
        baseFrameInfo.glesVersion = setupInfo.glesVersion;
        baseFrameInfo.sceneRenderer = this;
        baseFrameInfo.theView = theView;
        baseFrameInfo.viewTrans = viewTrans;
        baseFrameInfo.viewTrans4d = viewTrans4d;
        baseFrameInfo.modelTrans = modelTrans;
        baseFrameInfo.modelTrans4d = modelTrans4d;
        baseFrameInfo.scene = scene;
        baseFrameInfo.frameLen = duration;
        baseFrameInfo.currentTime = scene->getCurrentTime();
        baseFrameInfo.projMat = projMat;
        baseFrameInfo.projMat4d = projMat4d;
        baseFrameInfo.mvpMat = mvpMat;
        Eigen::Matrix4f mvpInvMat = mvpMat.inverse();
        baseFrameInfo.mvpInvMat = mvpInvMat;
        baseFrameInfo.mvpNormalMat = mvpNormalMat4f;
        baseFrameInfo.viewModelNormalMat = modelAndViewNormalMat;
        baseFrameInfo.viewAndModelMat = modelAndViewMat;
        baseFrameInfo.viewAndModelMat4d = modelAndViewMat4d;
        baseFrameInfo.pvMat = Matrix4dToMatrix4f(pvMat);
        baseFrameInfo.pvMat4d = pvMat;
        theView->getOffsetMatrices(baseFrameInfo.offsetMatrices, frameSize, overlapMarginX);
        Point2d screenSize = theView->screenSizeInDisplayCoords(frameSize);
        baseFrameInfo.screenSizeInDisplayCoords = screenSize;
        baseFrameInfo.lights = &lights;
        
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
        baseFrameInfo.heightAboveSurface = (float)theView->heightAboveSurface();
        baseFrameInfo.eyePos = Vector3d(eyeVec4d.x(),eyeVec4d.y(),eyeVec4d.z()) * (1.0+baseFrameInfo.heightAboveSurface);
        
        if (perfInterval > 0)
            perfTimer.startTiming("Scene preprocessing");
        
        // Run the preprocess for the changes.  These modify things the active models need.
        int numPreProcessChanges = scene->preProcessChanges(theView, this, now);
        
        if (perfInterval > 0)
            perfTimer.addCount("Preprocess Changes", numPreProcessChanges);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Scene preprocessing");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Active Model Runs");
        
        // Let the active models to their thing
        // That thing had better not take too long
        auto activeModels = scene->getActiveModels();
        for (const auto &activeModel : activeModels) {
            activeModel->updateForFrame(&baseFrameInfo);
            // Note: We were setting the GL context here.  Do we need to?
        }
        if (perfInterval > 0)
            perfTimer.addCount("Active Models", (int)activeModels.size());
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Active Model Runs");
        
        if (perfInterval > 0)
            perfTimer.addCount("Scene changes", scene->getNumChangeRequests());
        
        if (perfInterval > 0)
            perfTimer.startTiming("Scene processing");
        
        // Merge any outstanding changes into the scenegraph
        scene->processChanges(theView,this,now);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Scene processing");
        
        // Work through the available offset matrices (only 1 if we're not wrapping)
        std::vector<Matrix4d> &offsetMats = baseFrameInfo.offsetMatrices;
        // Turn these drawables in to a vector
        std::vector<DrawableContainer> drawList;
        std::vector<DrawableRef> screenDrawables;
        std::vector<DrawableRef> generatedDrawables;
        std::vector<Matrix4d> mvpMats;
        std::vector<Matrix4d> mvpInvMats;
        std::vector<Matrix4f> mvpMats4f;
        std::vector<Matrix4f> mvpInvMats4f;
        mvpMats.resize(offsetMats.size());
        mvpInvMats.resize(offsetMats.size());
        mvpMats4f.resize(offsetMats.size());
        mvpInvMats4f.resize(offsetMats.size());
        bool calcPassDone = false;
        for (unsigned int off=0;off<offsetMats.size();off++)
        {
            RendererFrameInfo offFrameInfo(baseFrameInfo);  // NOLINT: slicing from GLES frame
            // Tweak with the appropriate offset matrix
            modelAndViewMat4d = viewTrans4d * offsetMats[off] * modelTrans4d;
            pvMat = projMat4d * viewTrans4d * offsetMats[off];
            modelAndViewMat = Matrix4dToMatrix4f(modelAndViewMat4d);
            mvpMats[off] = projMat4d * modelAndViewMat4d;
            mvpInvMats[off] = (Eigen::Matrix4d)mvpMats[off].inverse();
            mvpMats4f[off] = Matrix4dToMatrix4f(mvpMats[off]);
            mvpInvMats4f[off] = Matrix4dToMatrix4f(mvpInvMats[off]);
            modelAndViewNormalMat4d = modelAndViewMat4d.inverse().transpose();
            modelAndViewNormalMat = Matrix4dToMatrix4f(modelAndViewNormalMat4d);
            Matrix4d &thisMvpMat = mvpMats[off];
            offFrameInfo.mvpMat = mvpMats4f[off];
            offFrameInfo.mvpInvMat = mvpInvMats4f[off];
            mvpNormalMat4f = Matrix4dToMatrix4f(mvpMats[off].inverse().transpose());
            offFrameInfo.mvpNormalMat = mvpNormalMat4f;
            offFrameInfo.viewModelNormalMat = modelAndViewNormalMat;
            offFrameInfo.viewAndModelMat4d = modelAndViewMat4d;
            offFrameInfo.viewAndModelMat = modelAndViewMat;
            offFrameInfo.pvMat = Matrix4dToMatrix4f(pvMat);
            offFrameInfo.pvMat4d = pvMat;

            auto rawDrawables = scene->getDrawables();
            for (auto draw : rawDrawables)
            {
                auto *theDrawable = dynamic_cast<DrawableGLES *>(draw);
                if (theDrawable->isOn(&offFrameInfo))
                {
                    const Matrix4d *localMat = theDrawable->getMatrix();
                    if (localMat)
                    {
                        Eigen::Matrix4d newMvpMat = thisMvpMat * (*localMat);
                        Eigen::Matrix4d newMvMat = modelAndViewMat4d * (*localMat);
                        Eigen::Matrix4d newMvNormalMat = newMvMat.inverse().transpose();
                        drawList.emplace_back(theDrawable,newMvpMat,newMvMat,newMvNormalMat);
                    } else
                        drawList.emplace_back(theDrawable,thisMvpMat,modelAndViewMat4d,modelAndViewNormalMat4d);
                }
            }
        }
        
        // Sort the drawables (possibly multiple of the same if we have offset matrices)
        bool sortLinesToEnd = (zBufferMode == zBufferOffDefault);
        std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(sortLinesToEnd,&baseFrameInfo));
        
        if (perfInterval > 0)
            perfTimer.startTiming("Calculation Shaders");
        
        // Run any calculation shaders
        // These should be independent of screen space, so we only run them once and ignore offsets.
        if (!calcPassDone) {
            // But do we have any
            bool haveCalcShader = false;
            for (unsigned int ii=0;ii<drawList.size();ii++)
                if (drawList[ii].drawable->getCalculationProgram() != EmptyIdentity) {
                    haveCalcShader = true;
                    break;
                }
            
            if (haveCalcShader) {
                // Have to set an active framebuffer for our empty fragment shaders to write to
                RenderTargetGLESRef renderTarget = std::dynamic_pointer_cast<RenderTargetGLES>(renderTargets[0]);
                renderTarget->setActiveFramebuffer(this);
                
                glEnable(GL_RASTERIZER_DISCARD);
                
                for (unsigned int ii=0;ii<drawList.size();ii++) {
                    DrawableContainer &drawContain = drawList[ii];
                    SimpleIdentity calcProgID = drawContain.drawable->getCalculationProgram();
                    
                    // Figure out the program to use for drawing
                    if (calcProgID == EmptyIdentity)
                        continue;
                    auto *program = (ProgramGLES *)scene->getProgram(calcProgID);
                    if (program)
                    {
                        glUseProgram(program->getProgram());
                        baseFrameInfo.program = program;
                    }
                    
                    // Tweakers probably not necessary, but who knows
                    drawContain.drawable->runTweakers(&baseFrameInfo);
                    
                    // Run the calculation phase
                    drawContain.drawable->calculate(&baseFrameInfo,scene);
                }
                
                glDisable(GL_RASTERIZER_DISCARD);
            }
            
            calcPassDone = true;
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Calculation Shaders");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Draw Execution");
        
        SimpleIdentity curProgramId = EmptyIdentity;
        
        // Iterate through rendering targets here
        for (const RenderTargetRef &inRenderTarget : renderTargets)
        {
            RenderTargetGLESRef renderTarget = std::dynamic_pointer_cast<RenderTargetGLES>(inRenderTarget);
            
            renderTarget->setActiveFramebuffer(this);
            
            if (renderTarget->clearEveryFrame || renderTarget->clearOnce)
            {
                renderTarget->clearOnce = false;
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                CheckGLError("SceneRendererES2: glClear");
            }
            
            bool depthMaskOn = (zBufferMode == zBufferOn);
            for (unsigned int ii=0;ii<drawList.size();ii++)
            {
                DrawableContainer &drawContain = drawList[ii];
                                
                // For this mode we turn the z buffer off until we get a request to turn it on
                if (zBufferMode == zBufferOffDefault)
                {
                    if (drawContain.drawable->getRequestZBuffer())
                    {
                        glDepthFunc(GL_LESS);
                        depthMaskOn = true;
                    } else {
                        glDepthFunc(GL_ALWAYS);
                    }
                }
                
                // If we're drawing lines or points we don't want to update the z buffer
                if (zBufferMode != zBufferOff)
                {
                    if (drawContain.drawable->getWriteZbuffer())
                        glDepthMask(GL_TRUE);
                    else
                        glDepthMask(GL_FALSE);
                }
                
                // Set up transforms to use right now
                Matrix4f currentMvpMat = Matrix4dToMatrix4f(drawContain.mvpMat);
                Matrix4f currentMvpInvMat = Matrix4dToMatrix4f(drawContain.mvpMat.inverse());
                Matrix4f currentMvMat = Matrix4dToMatrix4f(drawContain.mvMat);
                Matrix4f currentMvNormalMat = Matrix4dToMatrix4f(drawContain.mvNormalMat);
                baseFrameInfo.mvpMat = currentMvpMat;
                baseFrameInfo.mvpInvMat = currentMvpInvMat;
                baseFrameInfo.viewAndModelMat = currentMvMat;
                baseFrameInfo.viewModelNormalMat = currentMvNormalMat;
                
                // Figure out the program to use for drawing
                SimpleIdentity drawProgramId = drawContain.drawable->getProgram();
                if (drawProgramId == EmptyIdentity) {
                    wkLogLevel(Error, "Drawable missing program ID.  Skipping.");
                    continue;
                }
                if (drawProgramId != curProgramId)
                {
                    curProgramId = drawProgramId;
                    auto program = (ProgramGLES *)scene->getProgram(drawProgramId);
                    if (program)
                    {
                        //                    [renderStateOptimizer setUseProgram:program->getProgram()];
                        glUseProgram(program->getProgram());
                        // Assign the lights if we need to
                        if (program->hasLights() && !lights.empty())
                            program->setLights(lights, lightsLastUpdated, &defaultMat, currentMvpMat);
                        // Explicitly turn the lights on
                        program->setUniform(u_numLightsNameID, (int)lights.size());
                        
                        baseFrameInfo.program = program;
                    } else {
                        wkLogLevel(Error, "Missing OpenGL ES Program.");
                        continue;
                    }
                }
                if (drawProgramId == EmptyIdentity || !baseFrameInfo.program)
                    continue;
                
                // Only draw drawables that are active for the current render target
                if (drawContain.drawable->getRenderTarget() != renderTarget->getId())
                    continue;
                
                // Run any tweakers right here
                drawContain.drawable->runTweakers(&baseFrameInfo);
                
                // Draw using the given program
                drawContain.drawable->draw(&baseFrameInfo,scene);
                
                // If we had a local matrix, set the frame info back to the general one
                //            if (localMat)
                //                offFrameInfo.mvpMat = mvpMat;
                
                numDrawables++;
            }
        }
        
        if (perfInterval > 0)
            perfTimer.addCount("Drawables drawn", numDrawables);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Draw Execution");
        
        // Anything generated needs to be cleaned up
        generatedDrawables.clear();
        drawList.clear();
    }
    
    //    if (perfInterval > 0)
    //        perfTimer.startTiming("glFinish");
    
    //    glFlush();
    //    glFinish();
    
    //    if (perfInterval > 0)
    //        perfTimer.stopTiming("glFinish");
    
    if (perfInterval > 0)
        perfTimer.startTiming("Present Renderbuffer");

#ifndef __ANDROID__
    // Explicitly discard the depth buffer
    const GLenum discards[]  = {GL_DEPTH_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER,1,discards);
    CheckGLError("SceneRendererES2: glInvalidateFramebuffer");
#endif
    
    // Subclass with do the presentation
    presentRender();
    
    // Snapshots tend to be platform specific
    snapshotCallback(now);
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Present Renderbuffer");
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Frame");
    
    // Update the frames per sec
    if (perfInterval > 0 && frameCount > perfInterval)
    {
        const TimeInterval newNow = scene->getCurrentTime();
        const TimeInterval howLong =  newNow - frameCountStart;
        framesPerSec = (float)(frameCount / howLong);
        frameCountStart = newNow;
        frameCount = 0;
        
        wkLogLevel(Verbose,"---Rendering Performance---");
        wkLogLevel(Verbose," Frames per sec = %.2f",framesPerSec);
        perfTimer.log();
        perfTimer.clear();
    }
}

RawDataRef SceneRendererGLES::getSnapshotAt(SimpleIdentity renderTargetID, int x, int y, int width, int height)
{
    for (const auto &renderTarget: renderTargets) {
        if (renderTarget->getId() == renderTargetID) {
            if (width <= 0 || height <= 0) {
                return renderTarget->snapshot();
            } else {
                return renderTarget->snapshot(x,y,width,height);
            }
        }
    }

    return RawDataRef();
}


BasicDrawableBuilderRef SceneRendererGLES::makeBasicDrawableBuilder(const std::string &name) const
{
    return std::make_shared<BasicDrawableBuilderGLES>(name,scene);
}

BasicDrawableInstanceBuilderRef SceneRendererGLES::makeBasicDrawableInstanceBuilder(const std::string &name) const
{
    return std::make_shared<BasicDrawableInstanceBuilderGLES>(name,scene);
}

BillboardDrawableBuilderRef SceneRendererGLES::makeBillboardDrawableBuilder(const std::string &name) const
{
    return std::make_shared<BillboardDrawableBuilderGLES>(name,scene);
}

ScreenSpaceDrawableBuilderRef SceneRendererGLES::makeScreenSpaceDrawableBuilder(const std::string &name) const
{
    return std::make_shared<ScreenSpaceDrawableBuilderGLES>(name,scene);
}

ParticleSystemDrawableBuilderRef  SceneRendererGLES::makeParticleSystemDrawableBuilder(const std::string &name) const
{
    return std::make_shared<ParticleSystemDrawableBuilderGLES>(name,scene);
}

WideVectorDrawableBuilderRef SceneRendererGLES::makeWideVectorDrawableBuilder(const std::string &name) const
{
    return std::make_shared<WideVectorDrawableBuilderGLES>(name,this,scene);
}

RenderTargetRef SceneRendererGLES::makeRenderTarget() const
{
    return std::make_shared<RenderTargetGLES>();
}

DynamicTextureRef SceneRendererGLES::makeDynamicTexture(const std::string &name) const
{
    return std::make_shared<DynamicTextureGLES>(name);
}

}


#include <utility>
