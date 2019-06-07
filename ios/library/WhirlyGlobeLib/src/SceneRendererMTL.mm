/*
 *  SceneRendererMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "SceneRendererMTL.h"
#import "BasicDrawableBuilderMTL.h"
#import "BasicDrawableInstanceBuilderMTL.h"
#import "BillboardDrawableBuilderMTL.h"
#import "ScreenSpaceDrawableBuilderMTL.h"
#import "ParticleSystemDrawableBuilderMTL.h"
#import "WideVectorDrawableBuilderMTL.h"
#import "RenderTargetMTL.h"
#import "DynamicTextureAtlasMTL.h"
#import "MaplyView.h"
#import "WhirlyKitLog.h"
#import "DefaultShadersMTL.h"

using namespace Eigen;

namespace WhirlyKit
{
    
RendererFrameInfoMTL::RendererFrameInfoMTL()
{
    cmdEncode = nil;
}
    
RendererFrameInfoMTL::RendererFrameInfoMTL(const RendererFrameInfoMTL &that)
: RendererFrameInfo(that)
{
    cmdEncode = that.cmdEncode;
}

SceneRendererMTL::SceneRendererMTL(id<MTLDevice> mtlDevice)
{
    init();
    setupInfo.mtlDevice = mtlDevice;
}
    
SceneRendererMTL::~SceneRendererMTL()
{
}

SceneRendererMTL::Type SceneRendererMTL::getType()
{
    return RenderMetal;
}

const RenderSetupInfo *SceneRendererMTL::getRenderSetupInfo() const
{
    return &setupInfo;
}

void SceneRendererMTL::setView(View *newView)
{
    SceneRenderer::setView(newView);
}

void SceneRendererMTL::setScene(Scene *newScene)
{
    SceneRenderer::setScene(newScene);
}

bool SceneRendererMTL::setup(int sizeX,int sizeY)
{
    // Set up a default render target
    RenderTargetMTLRef defaultTarget = RenderTargetMTLRef(new RenderTargetMTL(EmptyIdentity));
    defaultTarget->width = sizeX;
    defaultTarget->height = sizeY;
    if (framebufferTex) {
        defaultTarget->setTargetTexture(dynamic_cast<TextureBaseMTL *>(framebufferTex));
        // Note: Should make this optional
        defaultTarget->blendEnable = false;
    } else {
        if (sizeX > 0 && sizeY > 0)
            defaultTarget->init(this,NULL,EmptyIdentity);
        defaultTarget->blendEnable = true;
    }
    defaultTarget->clearEveryFrame = true;
    renderTargets.push_back(defaultTarget);
    
    return true;
}

bool SceneRendererMTL::resize(int sizeX,int sizeY)
{
    // Don't want to deal with it for offscreen rendering
    if (framebufferTex)
        return false;
    
    framebufferWidth = sizeX;
    framebufferHeight = sizeY;
    
    RenderTargetRef defaultTarget = renderTargets.back();
    defaultTarget->width = sizeX;
    defaultTarget->height = sizeY;
    defaultTarget->init(this, NULL, EmptyIdentity);
    
    return true;
}
    
    
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
    DrawListSortStruct2(bool useAlpha,bool useZBuffer,RendererFrameInfoMTL *frameInfo) : useAlpha(useAlpha), useZBuffer(useZBuffer), frameInfo(frameInfo)
    {
    }
    DrawListSortStruct2() { }
    DrawListSortStruct2(const DrawListSortStruct2 &that) : useAlpha(that.useAlpha), useZBuffer(that.useZBuffer), frameInfo(that.frameInfo)
    {
    }
    DrawListSortStruct2 & operator = (const DrawListSortStruct2 &that)
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
    RendererFrameInfoMTL *frameInfo;
};
    
static void CopyIntoFloat4x4(simd::float4x4 &dest,Eigen::Matrix4f &src)
{
    for (unsigned int ix=0;ix<4;ix++)
        for (unsigned int iy=0;iy<4;iy++)
            dest.columns[ix][iy] = src(ix*4+iy);
}
    
//static void CopyIntoFloat3(simd::float3 &dest,const Point3d &src)
//{
//    dest[0] = src.x();
//    dest[1] = src.y();
//    dest[2] = src.z();
//}
    
static void CopyIntoFloat3(simd::float3 &dest,const Point3f &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
    dest[2] = src.z();
}
    
static void CopyIntoFloat4(simd::float4 &dest,const Eigen::Vector4f &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
    dest[2] = src.z();
    dest[3] = src.w();
}
    
void SceneRendererMTL::setupUniformBuffer(RendererFrameInfoMTL *frameInfo)
{
    WhirlyKitShader::Uniforms uniforms;
    CopyIntoFloat4x4(uniforms.mvpMatrix,frameInfo->mvpMat);
    CopyIntoFloat4x4(uniforms.mvMatrix,frameInfo->viewAndModelMat);
    CopyIntoFloat4x4(uniforms.mvNormalMatrix,frameInfo->viewModelNormalMat);
    
    [frameInfo->cmdEncode setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:WKSUniformBuffer];
    [frameInfo->cmdEncode setFragmentBytes:&uniforms length:sizeof(uniforms) atIndex:WKSUniformBuffer];
}

void SceneRendererMTL::setupLightBuffer(SceneMTL *scene,id<MTLRenderCommandEncoder> cmdEncode)
{
    WhirlyKitShader::Lighting lighting;
    lighting.numLights = lights.size();
    for (unsigned int ii=0;ii<lighting.numLights;ii++) {
        DirectionalLight &dirLight = lights[ii];
        
        Eigen::Vector3f dir = dirLight.pos.normalized();
        Eigen::Vector3f halfPlane = (dir + Eigen::Vector3f(0,0,1)).normalized();
        
        WhirlyKitShader::Light &light = lighting.lights[ii];
        CopyIntoFloat3(light.direction,dir);
        CopyIntoFloat3(light.halfPlane,halfPlane);
        CopyIntoFloat4(light.ambient,dirLight.getAmbient());
        CopyIntoFloat4(light.diffuse,dirLight.getDiffuse());
        CopyIntoFloat4(light.specular,dirLight.getSpecular());
        light.viewDepend = dirLight.viewDependent ? 0.0f : 1.0f;
    }
    CopyIntoFloat4(lighting.mat.ambient,defaultMat.getAmbient());
    CopyIntoFloat4(lighting.mat.diffuse,defaultMat.getDiffuse());
    CopyIntoFloat4(lighting.mat.specular,defaultMat.getSpecular());
    lighting.mat.specularExponent = defaultMat.getSpecularExponent();
    
    [cmdEncode setVertexBytes:&lighting length:sizeof(lighting) atIndex:WKSLightingBuffer];
}
    
void SceneRendererMTL::setupDrawStateA(WhirlyKitShader::UniformDrawStateA &drawState,RendererFrameInfoMTL *frameInfo)
{
    // This works for now
    bzero(&drawState,sizeof(drawState));
}

void SceneRendererMTL::render(TimeInterval duration,
                              MTLRenderPassDescriptor *renderPassDesc,
                              id<CAMetalDrawable> drawable)
{
    if (!scene)
        return;
    
    frameCount++;
    
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
    {
        // Process the scene even if the window isn't up
        processScene();
        return;
    }
    
    theView->animate();
    
    TimeInterval now = TimeGetCurrent();
    
    // Note: Put this back
//    if (!hasChanges())
//        return;
    
    if (perfInterval > 0)
        perfTimer.startTiming("Render Frame");
    
    if (perfInterval > 0)
        perfTimer.startTiming("Render Setup");

    // See if we're dealing with a globe or map view
    Maply::MapView *mapView = dynamic_cast<Maply::MapView *>(theView);
    float overlapMarginX = 0.0;
    if (mapView) {
        overlapMarginX = scene->getOverlapMargin();
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

    if (perfInterval > 0)
        perfTimer.stopTiming("Render Setup");

    // Note: Make this more general
    auto defaultTarget = renderTargets.back();
    auto clearColor = defaultTarget->clearColor;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0],clearColor[1],clearColor[2],clearColor[3]);

    // Send the command buffer and encoders
    // TODO: Make this general for more than one render target
    id<MTLDevice> mtlDevice = setupInfo.mtlDevice;
    id<MTLCommandQueue> cmdQueue = [mtlDevice newCommandQueue];

    if (scene)
    {
        int numDrawables = 0;
        
        RendererFrameInfoMTL baseFrameInfo;
        baseFrameInfo.sceneRenderer = this;
        baseFrameInfo.theView = theView;
        baseFrameInfo.viewTrans = viewTrans;
        baseFrameInfo.viewTrans4d = viewTrans4d;
        baseFrameInfo.modelTrans = modelTrans;
        baseFrameInfo.modelTrans4d = modelTrans4d;
        baseFrameInfo.scene = scene;
        baseFrameInfo.frameLen = duration;
        baseFrameInfo.currentTime = TimeGetCurrent();
        baseFrameInfo.projMat = projMat;
        baseFrameInfo.projMat4d = projMat4d;
        baseFrameInfo.mvpMat = mvpMat;
        Eigen::Matrix4f mvpInvMat = mvpMat.inverse();
        baseFrameInfo.mvpInvMat = mvpInvMat;
        baseFrameInfo.mvpNormalMat = mvpNormalMat4f;
        baseFrameInfo.viewModelNormalMat = modelAndViewNormalMat;
        baseFrameInfo.viewAndModelMat = modelAndViewMat;
        baseFrameInfo.viewAndModelMat4d = modelAndViewMat4d;
        Matrix4f pvMat4f = Matrix4dToMatrix4f(pvMat);
        baseFrameInfo.pvMat = pvMat4f;
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
        baseFrameInfo.heightAboveSurface = theView->heightAboveSurface();
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
        for (auto activeModel : scene->activeModels) {
            activeModel->updateForFrame(&baseFrameInfo);
        }
        if (perfInterval > 0)
            perfTimer.addCount("Active Models", (int)scene->activeModels.size());
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Active Model Runs");
        
        if (perfInterval > 0)
            perfTimer.addCount("Scene changes", (int)scene->changeRequests.size());
        
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
            RendererFrameInfoMTL offFrameInfo(baseFrameInfo);
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
            Matrix4f pvMat4f = Matrix4dToMatrix4f(pvMat);
            offFrameInfo.pvMat = pvMat4f;
            offFrameInfo.pvMat4d = pvMat;
            
            DrawableRefSet rawDrawables = scene->getDrawables();
            for (DrawableRefSet::iterator it = rawDrawables.begin(); it != rawDrawables.end(); ++it)
            {
                Drawable *theDrawable = it->second.get();
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
        
        // Sort the drawables (possibly multiple of the same if we have offset matrices)
        bool sortLinesToEnd = (zBufferMode == zBufferOffDefault);
        std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(sortAlphaToEnd,sortLinesToEnd,&baseFrameInfo));
        
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
                RenderTargetMTLRef renderTarget = std::dynamic_pointer_cast<RenderTargetMTL>(renderTargets[0]);
                
                for (unsigned int ii=0;ii<drawList.size();ii++) {
                    DrawableContainer &drawContain = drawList[ii];
                    SimpleIdentity calcProgID = drawContain.drawable->getCalculationProgram();
                    
                    // Figure out the program to use for drawing
                    if (calcProgID == EmptyIdentity)
                        continue;
//                    ProgramGLES *program = (ProgramGLES *)scene->getProgram(calcProgID);
//                    if (program)
//                    {
//                        glUseProgram(program->getProgram());
//                        baseFrameInfo.program = program;
//                    }
                    
                    // Tweakers probably not necessary, but who knows
                    drawContain.drawable->runTweakers(&baseFrameInfo);
                    
                    // Run the calculation phase
                    drawContain.drawable->calculate(&baseFrameInfo,scene);
                }
            }
            
            calcPassDone = true;
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Calculation Shaders");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Draw Execution");
        
        // Iterate through rendering targets here
        for (RenderTargetRef inRenderTarget : renderTargets)
        {
            RenderTargetMTLRef renderTarget = std::dynamic_pointer_cast<RenderTargetMTL>(inRenderTarget);
                        
            // Each render target needs its own buffer and command queue
            id<MTLCommandBuffer> cmdBuff = [cmdQueue commandBuffer];
            id<MTLRenderCommandEncoder> cmdEncode = nil;

            if (renderTarget->getId() == EmptyIdentity) {
                cmdEncode = [cmdBuff renderCommandEncoderWithDescriptor:renderPassDesc];
                baseFrameInfo.renderPassDesc = renderPassDesc;
            } else {
                cmdEncode = [cmdBuff renderCommandEncoderWithDescriptor:renderTarget->renderPassDesc];
                baseFrameInfo.renderPassDesc = renderTarget->renderPassDesc;
            }
            
            // Backface culling on by default
            [cmdEncode setCullMode:MTLCullModeFront];

            baseFrameInfo.cmdEncode = cmdEncode;
            setupLightBuffer((SceneMTL *)scene,cmdEncode);

            // Keep track of state changes for z buffer state
            bool firstDepthState = true;
            bool zBufferWrite = (zBufferMode == zBufferOn);
            bool zBufferRead = (zBufferMode == zBufferOn);
            
            bool lastZBufferWrite = zBufferWrite;
            bool lastZBufferRead = zBufferRead;
            
            for (unsigned int ii=0;ii<drawList.size();ii++)
            {
                DrawableContainer &drawContain = drawList[ii];
                
                // The first time we hit an explicitly alpha drawable
                //  turn off the depth buffer
                if (depthBufferOffForAlpha && drawContain.drawable->hasAlpha(&baseFrameInfo))
                    zBufferWrite = false;
                
                // For this mode we turn the z buffer off until we get a request to turn it on
                zBufferRead = drawContain.drawable->getRequestZBuffer();
                
                // If we're drawing lines or points we don't want to update the z buffer
                zBufferWrite = drawContain.drawable->getWriteZbuffer();
                
                // Off screen render targets don't like z buffering
                if (renderTarget->getId() != EmptyIdentity) {
                    zBufferRead = false;
                    zBufferWrite = false;
                }
                
                // TODO: Optimize this a bit
                if (firstDepthState ||
                    (zBufferRead != lastZBufferRead) ||
                    (zBufferWrite != lastZBufferWrite)) {
                    
                    MTLDepthStencilDescriptor *depthDesc = [[MTLDepthStencilDescriptor alloc] init];
                    if (zBufferRead)
                        depthDesc.depthCompareFunction = MTLCompareFunctionLess;
                    else
                        depthDesc.depthCompareFunction = MTLCompareFunctionAlways;
                    depthDesc.depthWriteEnabled = zBufferWrite;
                    
                    lastZBufferRead = zBufferRead;
                    lastZBufferWrite = zBufferWrite;
                    
                    id<MTLDepthStencilState> depthStencil = [mtlDevice newDepthStencilStateWithDescriptor:depthDesc];
                    
                    [cmdEncode setDepthStencilState:depthStencil];
                    firstDepthState = false;
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

                // Note: Try to do this once rather than per drawable
                setupUniformBuffer(&baseFrameInfo);

                // Figure out the program to use for drawing
                SimpleIdentity drawProgramId = drawContain.drawable->getProgram();
                ProgramMTL *program = (ProgramMTL *)scene->getProgram(drawProgramId);
                if (!program)
                    continue;
                
                baseFrameInfo.program = program;
                
                // Only draw drawables that are active for the current render target
                if (drawContain.drawable->getRenderTarget() != renderTarget->getId())
                    continue;
                
                // Activate the program
                program->preRender(&baseFrameInfo,(SceneMTL *)scene);
            
                // Run any tweakers right here
                drawContain.drawable->runTweakers(&baseFrameInfo);
                
                // "Draw" using the given program
                drawContain.drawable->draw(&baseFrameInfo,scene);
                
                // If we had a local matrix, set the frame info back to the general one
                //            if (localMat)
                //                offFrameInfo.mvpMat = mvpMat;
                
                numDrawables++;
            }

            [cmdEncode endEncoding];
            // Main screen has to be committed
            if (renderTarget->getId() == EmptyIdentity)
                [cmdBuff presentDrawable:drawable];
            [cmdBuff commit];
            if (renderTarget->getId() != EmptyIdentity)
                [cmdBuff waitUntilCompleted];
        }
        
        if (perfInterval > 0)
            perfTimer.addCount("Drawables drawn", numDrawables);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Draw Execution");
        
        // Anything generated needs to be cleaned up
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
    
    // Snapshots tend to be platform specific
    snapshotCallback(now);
    
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
        
        wkLogLevel(Verbose,"---Rendering Performance---");
        wkLogLevel(Verbose," Frames per sec = %.2f",framesPerSec);
        perfTimer.log();
        perfTimer.clear();
    }
}
    
void SceneRendererMTL::snapshotCallback(TimeInterval now)
{
    
}

BasicDrawableBuilderRef SceneRendererMTL::makeBasicDrawableBuilder(const std::string &name) const
{
    return BasicDrawableBuilderRef(new BasicDrawableBuilderMTL(name));
}

BasicDrawableInstanceBuilderRef SceneRendererMTL::makeBasicDrawableInstanceBuilder(const std::string &name) const
{
    return BasicDrawableInstanceBuilderRef(new BasicDrawableInstanceBuilderMTL(name));
}

BillboardDrawableBuilderRef SceneRendererMTL::makeBillboardDrawableBuilder(const std::string &name) const
{
    return BillboardDrawableBuilderRef(new BillboardDrawableBuilderMTL(name));
}

ScreenSpaceDrawableBuilderRef SceneRendererMTL::makeScreenSpaceDrawableBuilder(const std::string &name) const
{
    return ScreenSpaceDrawableBuilderRef(new ScreenSpaceDrawableBuilderMTL(name));
}

ParticleSystemDrawableBuilderRef  SceneRendererMTL::makeParticleSystemDrawableBuilder(const std::string &name) const
{
    return ParticleSystemDrawableBuilderRef(new ParticleSystemDrawableBuilderMTL(name));
}

WideVectorDrawableBuilderRef SceneRendererMTL::makeWideVectorDrawableBuilder(const std::string &name) const
{
    return WideVectorDrawableBuilderRef(new WideVectorDrawableBuilderMTL(name));
}

RenderTargetRef SceneRendererMTL::makeRenderTarget() const
{
    return RenderTargetRef(new RenderTargetMTL());
}

DynamicTextureRef SceneRendererMTL::makeDynamicTexture(const std::string &name) const
{
    return DynamicTextureRef(new DynamicTextureMTL(name));
}

    
}
