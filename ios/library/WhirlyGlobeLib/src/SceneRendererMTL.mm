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
#import "RawData_NSData.h"
#import "RenderTargetMTL.h"

using namespace Eigen;

namespace WhirlyKit
{
    
RendererFrameInfoMTL::RendererFrameInfoMTL()
{
}
    
RendererFrameInfoMTL::RendererFrameInfoMTL(const RendererFrameInfoMTL &that)
: RendererFrameInfo(that)
{
}

SceneRendererMTL::SceneRendererMTL(id<MTLDevice> mtlDevice, float inScale)
{
    offscreenBlendEnable = false;
    init();
    scale = inScale;
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

bool SceneRendererMTL::setup(int sizeX,int sizeY,bool offscreen)
{
    // Set up a default render target
    RenderTargetMTLRef defaultTarget = RenderTargetMTLRef(new RenderTargetMTL(EmptyIdentity));
    defaultTarget->width = sizeX;
    defaultTarget->height = sizeY;
    defaultTarget->clearEveryFrame = true;
    if (offscreen) {
        framebufferWidth = sizeX;
        framebufferHeight = sizeY;
        
        // Create the texture we'll use right here
        TextureMTLRef fbTexMTL = TextureMTLRef(new TextureMTL("Framebuffer Texture"));
        fbTexMTL->setWidth(sizeX);
        fbTexMTL->setHeight(sizeY);
        fbTexMTL->setIsEmptyTexture(true);
        fbTexMTL->setFormat(TexTypeUnsignedByte);
        fbTexMTL->createInRenderer(&setupInfo);
        framebufferTex = fbTexMTL;
        
        // And one for depth
        TextureMTLRef depthTexMTL = TextureMTLRef(new TextureMTL("Framebuffer Depth Texture"));
        depthTexMTL->setWidth(sizeX);
        depthTexMTL->setHeight(sizeY);
        depthTexMTL->setIsEmptyTexture(true);
        depthTexMTL->setFormat(TexTypeDepthFloat32);
        depthTexMTL->createInRenderer(&setupInfo);

        // Note: Should make this optional
        defaultTarget->blendEnable = offscreenBlendEnable;
        defaultTarget->setTargetTexture(fbTexMTL.get());
        defaultTarget->setTargetDepthTexture(depthTexMTL.get());
    } else {
        if (sizeX > 0 && sizeY > 0)
            defaultTarget->init(this,NULL,EmptyIdentity);
        defaultTarget->blendEnable = true;
    }
    renderTargets.push_back(defaultTarget);
    
    workGroups[WorkGroup::ScreenRender]->addRenderTarget(defaultTarget);
    
    return true;
}
    
void SceneRendererMTL::setClearColor(const RGBAColor &color)
{
    if (renderTargets.empty())
        return;
    
    auto defaultTarget = renderTargets.back();
    defaultTarget->setClearColor(color);
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
    
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,Eigen::Matrix4f &src)
{
    for (unsigned int ix=0;ix<4;ix++)
        for (unsigned int iy=0;iy<4;iy++)
            dest.columns[ix][iy] = src(ix*4+iy);
}
    
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,Eigen::Matrix4d &src)
{
    for (unsigned int ix=0;ix<4;ix++)
        for (unsigned int iy=0;iy<4;iy++)
            dest.columns[ix][iy] = src(ix*4+iy);
}
    
void CopyIntoMtlFloat3(simd::float3 &dest,const Point3d &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
    dest[2] = src.z();
}
    
void CopyIntoMtlFloat3(simd::float3 &dest,const Point3f &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
    dest[2] = src.z();
}
    
void CopyIntoMtlFloat2(simd::float2 &dest,const Point2f &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
}

void CopyIntoMtlFloat4(simd::float4 &dest,const Eigen::Vector4f &src)
{
    dest[0] = src.x();
    dest[1] = src.y();
    dest[2] = src.z();
    dest[3] = src.w();
}
    
void CopyIntoMtlFloat4(simd::float4 &dest,const float vals[4])
{
    dest[0] = vals[0];
    dest[1] = vals[1];
    dest[2] = vals[2];
    dest[3] = vals[3];
}
    
void SceneRendererMTL::setupUniformBuffer(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,CoordSystemDisplayAdapter *coordAdapter,int texLevel)
{
    WhirlyKitShader::Uniforms uniforms;
    bzero(&uniforms,sizeof(uniforms));
    CopyIntoMtlFloat4x4(uniforms.mvpMatrix,frameInfo->mvpMat);
    CopyIntoMtlFloat4x4(uniforms.mvpInvMatrix,frameInfo->mvpInvMat);
    CopyIntoMtlFloat4x4(uniforms.mvMatrix,frameInfo->viewAndModelMat);
    CopyIntoMtlFloat4x4(uniforms.mvNormalMatrix,frameInfo->viewModelNormalMat);
    CopyIntoMtlFloat3(uniforms.eyePos,frameInfo->eyePos);
    Point2f pixDispSize(frameInfo->screenSizeInDisplayCoords.x()/frameInfo->sceneRenderer->framebufferWidth,
                        frameInfo->screenSizeInDisplayCoords.y()/frameInfo->sceneRenderer->framebufferHeight);
    CopyIntoMtlFloat2(uniforms.pixDispSize,pixDispSize);
    Point2f frameSize(frameInfo->sceneRenderer->framebufferWidth,frameInfo->sceneRenderer->framebufferHeight);
    CopyIntoMtlFloat2(uniforms.frameSize, frameSize);
    uniforms.outputTexLevel = texLevel;
    uniforms.globeMode = !coordAdapter->isFlat();
    uniforms.frameCount = frameCount;
    
    [cmdEncode setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:WKSUniformBuffer];
    [cmdEncode setFragmentBytes:&uniforms length:sizeof(uniforms) atIndex:WKSUniformBuffer];
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
        CopyIntoMtlFloat3(light.direction,dir);
        CopyIntoMtlFloat3(light.halfPlane,halfPlane);
        CopyIntoMtlFloat4(light.ambient,dirLight.getAmbient());
        CopyIntoMtlFloat4(light.diffuse,dirLight.getDiffuse());
        CopyIntoMtlFloat4(light.specular,dirLight.getSpecular());
        light.viewDepend = dirLight.viewDependent ? 0.0f : 1.0f;
    }
    CopyIntoMtlFloat4(lighting.mat.ambient,defaultMat.getAmbient());
    CopyIntoMtlFloat4(lighting.mat.diffuse,defaultMat.getDiffuse());
    CopyIntoMtlFloat4(lighting.mat.specular,defaultMat.getSpecular());
    lighting.mat.specularExponent = defaultMat.getSpecularExponent();
    
    [cmdEncode setVertexBytes:&lighting length:sizeof(lighting) atIndex:WKSLightingBuffer];
}
    
void SceneRendererMTL::setupDrawStateA(WhirlyKitShader::UniformDrawStateA &drawState,RendererFrameInfoMTL *frameInfo)
{
    // That was anti-climactic
    bzero(&drawState,sizeof(drawState));
}
    
MTLRenderPipelineDescriptor *SceneRendererMTL::defaultRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo)
{
    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
    
    MTLRenderPipelineDescriptor *renderDesc = [[MTLRenderPipelineDescriptor alloc] init];
    renderDesc.vertexFunction = program->vertFunc;
    renderDesc.fragmentFunction = program->fragFunc;
    
    renderDesc.colorAttachments[0].pixelFormat = frameInfo->renderTarget->getPixelFormat();
    if (frameInfo->renderPassDesc.depthAttachment.texture)
        renderDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    
    if (frameInfo->renderTarget->blendEnable) {
        renderDesc.colorAttachments[0].blendingEnabled = true;
        renderDesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        renderDesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        renderDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        renderDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        renderDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        renderDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    } else {
        renderDesc.colorAttachments[0].blendingEnabled = false;
    }
    
    return renderDesc;
}
    
void SceneRendererMTL::addSnapshotDelegate(NSObject<WhirlyKitSnapshot> *newDelegate)
{
    snapshotDelegates.push_back(newDelegate);
}

void SceneRendererMTL::removeSnapshotDelegate(NSObject<WhirlyKitSnapshot> *oldDelegate)
{
    snapshotDelegates.erase(std::remove(snapshotDelegates.begin(), snapshotDelegates.end(), oldDelegate), snapshotDelegates.end());
}

void SceneRendererMTL::render(TimeInterval duration,
                              MTLRenderPassDescriptor *renderPassDesc,
                              id<SceneRendererMTLDrawableGetter> drawGetter)
{
    if (!scene)
        return;
    SceneMTL *sceneMTL = (SceneMTL *)scene;
    
    frameCount++;
    
    TimeInterval now = scene->getCurrentTime();

    if (framebufferWidth <= 0 || framebufferHeight <= 0)
    {
        // Process the scene even if the window isn't up
        processScene(now);
        return;
    }
    
    lastDraw = now;
    
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
    id<MTLDevice> mtlDevice = setupInfo.mtlDevice;
    id<MTLCommandQueue> cmdQueue = [mtlDevice newCommandQueue];

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
    Matrix4f pvMat4f = Matrix4dToMatrix4f(pvMat);
    baseFrameInfo.pvMat = pvMat4f;
    baseFrameInfo.pvMat4d = pvMat;
    theView->getOffsetMatrices(baseFrameInfo.offsetMatrices, frameSize, overlapMarginX);
    Point2d screenSize = theView->screenSizeInDisplayCoords(frameSize);
    baseFrameInfo.screenSizeInDisplayCoords = screenSize;
    baseFrameInfo.lights = &lights;
    baseFrameInfo.renderTarget = NULL;

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
    int numPreProcessChanges = preProcessScene(now);;
    
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
    processScene(now);
    
    // Update our work groups accordingly
    updateWorkGroups(&baseFrameInfo);
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Scene processing");
    
    // Work through the available offset matrices (only 1 if we're not wrapping)
    std::vector<Matrix4d> &offsetMats = baseFrameInfo.offsetMatrices;
    std::vector<RendererFrameInfoMTL> offFrameInfos;
    // Turn these drawables in to a vector
    std::vector<Matrix4d> mvpMats;
    std::vector<Matrix4d> mvpInvMats;
    std::vector<Matrix4f> mvpMats4f;
    std::vector<Matrix4f> mvpInvMats4f;
    mvpMats.resize(offsetMats.size());
    mvpInvMats.resize(offsetMats.size());
    mvpMats4f.resize(offsetMats.size());
    mvpInvMats4f.resize(offsetMats.size());
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
        offFrameInfos.push_back(offFrameInfo);
    }

    // Workgroups force us to draw things in order
    for (auto &workGroup : workGroups) {
        if (perfInterval > 0)
            perfTimer.startTiming("Work Group: " + workGroup->name);

        for (auto &targetContainer : workGroup->renderTargetContainers) {
            RenderTargetMTLRef renderTarget;
            if (!targetContainer->renderTarget) {
                // Need some sort of render target even if we're not really rendering
                renderTarget = std::dynamic_pointer_cast<RenderTargetMTL>(renderTargets.back());
            } else {
                renderTarget = std::dynamic_pointer_cast<RenderTargetMTL>(targetContainer->renderTarget);
            }

            // Render pass descriptor might change from frame to frame if we're clearing sporadically
            renderTarget->makeRenderPassDesc();
            baseFrameInfo.renderTarget = renderTarget.get();

            // Each render target needs its own buffer and command queue
            id<MTLCommandBuffer> cmdBuff = [cmdQueue commandBuffer];

            // Some of the drawables want memory copied before they draw
            // TODO: Check with them before creating the encoder
//            id<MTLBlitCommandEncoder> bltEncode = [cmdBuff blitCommandEncoder];
//            for (auto &draw : targetContainer->drawables) {
//                DrawableMTL *drawMTL = dynamic_cast<DrawableMTL *>(draw.get());
//                drawMTL->blitMemory(&baseFrameInfo,bltEncode,scene);
//            }
//            [bltEncode endEncoding];
            
            // If we're forcing a mipmap calculation, then we're just going to use this render target once
            // If not, then we run some program over it multiple times
            // TODO: Make the reduce operation more explicit
            int numLevels = renderTarget->numLevels();
            if (renderTarget->mipmapType != RenderTargetMipmapNone)
                numLevels = 1;

            for (unsigned int level=0;level<numLevels;level++) {

                // Set up the encoder
                id<MTLRenderCommandEncoder> cmdEncode = nil;
                if (renderTarget->getTex() == nil) {
                    // This happens if the dev wants an instantaneous render
                    if (!renderPassDesc)
                        renderPassDesc = renderTarget->getRenderPassDesc(level);
                    
                    baseFrameInfo.renderPassDesc = renderPassDesc;
                } else {
                    baseFrameInfo.renderPassDesc = renderTarget->getRenderPassDesc(level);
                }
                cmdEncode = [cmdBuff renderCommandEncoderWithDescriptor:baseFrameInfo.renderPassDesc];

                // Just run the calculation portion
                if (workGroup->groupType == WorkGroup::Calculation) {
                    // Work through the drawables
                    for (auto &draw : targetContainer->drawables) {
                        DrawableMTL *drawMTL = dynamic_cast<DrawableMTL *>(draw.get());
                        SimpleIdentity calcProgID = drawMTL->getCalculationProgram();
                        
                        // Figure out the program to use for drawing
                        if (calcProgID == EmptyIdentity)
                            continue;

                        ProgramMTL *calcProgram = (ProgramMTL *)scene->getProgram(calcProgID);
                        if (!calcProgram) {
                            NSLog(@"Invalid calculation program for drawable.  Skipping.");
                            continue;
                        }
                        baseFrameInfo.program = calcProgram;
                        
                        // Tweakers probably not necessary, but who knows
                        draw->runTweakers(&baseFrameInfo);
                        
                        // Regular uniforms
                        // TODO: Can we do this just once?
                        setupUniformBuffer(&baseFrameInfo,cmdEncode,scene->getCoordAdapter(),level);

                        // Per program uniforms need to be set up
                        BasicDrawableMTL::encodeUniBlocks(&baseFrameInfo, calcProgram->uniBlocks, cmdEncode);

                        // Run the calculation phase
                        drawMTL->calculate(&baseFrameInfo,cmdEncode,scene);
                    }
                } else {
                    // Keep track of state changes for z buffer state
                    bool firstDepthState = true;
                    bool zBufferWrite = (zBufferMode == zBufferOn);
                    bool zBufferRead = (zBufferMode == zBufferOn);

                    bool lastZBufferWrite = zBufferWrite;
                    bool lastZBufferRead = zBufferRead;

                    // TODO: Set this up once and reuse the buffer
                    setupLightBuffer(sceneMTL,cmdEncode);

                    // Backface culling on by default
                    // Note: Would like to not set this every time
                    [cmdEncode setCullMode:MTLCullModeFront];
                    
                    // Work through the drawables
                    for (auto &draw : targetContainer->drawables) {
                        DrawableMTL *drawMTL = dynamic_cast<DrawableMTL *>(draw.get());

                        // Figure out the program to use for drawing
                        SimpleIdentity drawProgramId = drawMTL->getProgram();
                        ProgramMTL *program = (ProgramMTL *)scene->getProgram(drawProgramId);
                        if (!program) {
                            wkLogLevel(Error, "SceneRendererMTL: Drawable without Program");
                            continue;
                        }

                        // For a reduce operation, we want to draw into the first level of the render
                        //  target texture and then run the reduce over the rest of those levels
                        if (level > 0 && program->getReduceMode() == Program::None)
                            continue;

                        // For this mode we turn the z buffer off until we get a request to turn it on
                        zBufferRead = drawMTL->getRequestZBuffer();
                        
                        // If we're drawing lines or points we don't want to update the z buffer
                        zBufferWrite = drawMTL->getWriteZbuffer();
                        
                        // Off screen render targets don't like z buffering
                        if (renderTarget->getTex() != nil) {
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
                        
                        for (unsigned int off=0;off<offFrameInfos.size();off++) {
                            // Set up transforms to use right now
                            Matrix4d &thisMvpMat = mvpMats[off];
                            const Matrix4d *localMat = drawMTL->getMatrix();
                            if (localMat)
                            {
                                Eigen::Matrix4d newMvpMat = projMat4d * viewTrans4d * offsetMats[off] * modelTrans4d * (*localMat);
                                Eigen::Matrix4d newMvMat = viewTrans4d * offsetMats[off] * modelTrans4d * (*localMat);
                                Eigen::Matrix4d newMvNormalMat = newMvMat.inverse().transpose();

                                baseFrameInfo.mvpMat = Matrix4dToMatrix4f(newMvpMat);
                                baseFrameInfo.mvpInvMat = Matrix4dToMatrix4f(newMvpMat.inverse());
                                baseFrameInfo.viewAndModelMat = Matrix4dToMatrix4f(newMvMat);
                                baseFrameInfo.viewModelNormalMat = Matrix4dToMatrix4f(newMvNormalMat);
                            } else {
                                baseFrameInfo.mvpMat = Matrix4dToMatrix4f(thisMvpMat);
                                baseFrameInfo.mvpInvMat = Matrix4dToMatrix4f(thisMvpMat.inverse());
                                baseFrameInfo.viewAndModelMat = Matrix4dToMatrix4f(modelAndViewMat4d);
                                baseFrameInfo.viewModelNormalMat = Matrix4dToMatrix4f(modelAndViewNormalMat4d);
                            }

                            // TODO: Try to do this once rather than per drawable
                            setupUniformBuffer(&baseFrameInfo,cmdEncode,scene->getCoordAdapter(),level);
                            
                            baseFrameInfo.program = program;
                            
                            // Activate the program
                            program->addResources(&baseFrameInfo, cmdEncode, sceneMTL);
                        
                            // Run any tweakers right here
                            drawMTL->runTweakers(&baseFrameInfo);
                            
                            // "Draw" using the given program
                            drawMTL->draw(&baseFrameInfo,cmdEncode,scene);
                            
                            // If we had a local matrix, set the frame info back to the general one
                            //            if (localMat)
                            //                offFrameInfo.mvpMat = mvpMat;
                            
                            numDrawables++;
                        }
                    }
                }
                
                [cmdEncode endEncoding];
                
                
            }
            
            // Some render targets like to do extra work on their images
            renderTarget->addPostProcessing(mtlDevice,cmdBuff);

            // Main screen has to be committed
            if (drawGetter != nil && workGroup->groupType == WorkGroup::ScreenRender) {
                id<CAMetalDrawable> drawable = [drawGetter getDrawable];
                [cmdBuff presentDrawable:drawable];
            }
            
            // This particular target may want a snapshot
            // TODO: Sort these into the render targets
            if (!snapshotDelegates.empty()) {
                // TODO: Still have a race condition here.  If the renderer is shutdown before this
                //       is called, we'll get a crash
                [cmdBuff addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        // Look for the snapshot delegate that wants this render target
                        for (auto snapshotDelegate : snapshotDelegates) {
                            if (![snapshotDelegate needSnapshot:now])
                                continue;
                            
                            if (renderTarget->getId() != [snapshotDelegate renderTargetID]) {
                                continue;
                            }
                            
                            [snapshotDelegate snapshotData:nil];
                        }
                    });
                }];
            }

            [cmdBuff commit];
            
            // This happens for offline rendering and we want to wait until the render finishes to return it
            if (!drawGetter)
                [cmdBuff waitUntilCompleted];
        }
                
        if (perfInterval > 0)
            perfTimer.stopTiming("Work Group: " + workGroup->name);
    }
        
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
    
    sceneMTL->endOfFrameBufferClear();
}

RenderTargetMTLRef SceneRendererMTL::getRenderTarget(SimpleIdentity renderTargetID)
{
    RenderTargetMTLRef renderTarget;
    
    if (renderTargetID == EmptyIdentity) {
        renderTarget = std::dynamic_pointer_cast<RenderTargetMTL>(renderTargets.back());
    } else {
        for (auto target : renderTargets) {
            if (target->getId() == renderTargetID) {
                renderTarget = std::dynamic_pointer_cast<RenderTargetMTL>(target);
                break;
            }
        }
    }
    
    return renderTarget;
}

RawDataRef SceneRendererMTL::getSnapshot(SimpleIdentity renderTargetID)
{
    RenderTargetMTLRef renderTarget = getRenderTarget(renderTargetID);
    if (!renderTarget)
        return nil;
    
    return renderTarget->snapshot();
}

RawDataRef SceneRendererMTL::getSnapshotAt(SimpleIdentity renderTargetID,int x,int y)
{
    RenderTargetMTLRef renderTarget = getRenderTarget(renderTargetID);
    if (!renderTarget)
        return nil;

    return renderTarget->snapshot(x, y, 1, 1);
}

RawDataRef SceneRendererMTL::getSnapshotMinMax(SimpleIdentity renderTargetID)
{
    RenderTargetMTLRef renderTarget = getRenderTarget(renderTargetID);
    if (!renderTarget)
        return nil;

    return renderTarget->snapshotMinMax();
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
