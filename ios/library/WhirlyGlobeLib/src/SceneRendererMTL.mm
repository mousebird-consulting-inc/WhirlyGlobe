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

WorkGroupMTL::WorkGroupMTL(GroupType inGroupType)
{
    groupType = inGroupType;
    
    switch (groupType) {
        case Calculation:
            // For calculation we don't really have a render target
            renderTargetContainers.push_back(makeRenderTargetContainer());
            break;
        case Offscreen:
            break;
        case ReduceOps:
            break;
        case ScreenRender:
            break;
    }
}

WorkGroupMTL::~WorkGroupMTL()
{
}

WorkGroupMTL::RenderTargetContainerMTL::RenderTargetContainerMTL(RenderTargetRef renderTarget)
: RenderTargetContainer(renderTarget)
{
}

WorkGroupMTL::RenderTargetContainerRef WorkGroupMTL::makeRenderTargetContainer()
{
    return RenderTargetContainerRef(new RenderTargetContainerMTL(RenderTargetRef()));
}
    
RendererFrameInfoMTL::RendererFrameInfoMTL()
{
}
    
RendererFrameInfoMTL::RendererFrameInfoMTL(const RendererFrameInfoMTL &that)
: RendererFrameInfo(that)
{
}

SceneRendererMTL::SceneRendererMTL(id<MTLDevice> mtlDevice,id<MTLLibrary> mtlLibrary, float inScale)
: setupInfo(mtlDevice,mtlLibrary)
{
    offscreenBlendEnable = false;
    // Indirect rendering is only on for 13 and later
    if (@available(iOS 13.0, *)) {
        indirectRender = true;
    } else {
        indirectRender = false;
    }
    init();

    // Calculation shaders
    workGroups.push_back(WorkGroupRef(new WorkGroupMTL(WorkGroup::Calculation)));
    // Offscreen target render group
    workGroups.push_back(WorkGroupRef(new WorkGroupMTL(WorkGroup::Offscreen)));
    // Middle one for weird stuff
    workGroups.push_back(WorkGroupRef(new WorkGroupMTL(WorkGroup::ReduceOps)));
    // Last workgroup is used for on screen rendering
    workGroups.push_back(WorkGroupRef(new WorkGroupMTL(WorkGroup::ScreenRender)));

    scale = inScale;
    setupInfo.mtlDevice = mtlDevice;
    setupInfo.uniformBuff = setupInfo.heapManage.allocateBuffer(HeapManagerMTL::Drawable,sizeof(WhirlyKitShader::Uniforms));
    setupInfo.lightingBuff = setupInfo.heapManage.allocateBuffer(HeapManagerMTL::Drawable,sizeof(WhirlyKitShader::Lighting));
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
        
void SceneRendererMTL::setupUniformBuffer(RendererFrameInfoMTL *frameInfo,id<MTLBlitCommandEncoder> bltEncode,CoordSystemDisplayAdapter *coordAdapter,int texLevel,ResourceRefsMTL &resources)
{
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    
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
    
    // Copy this to a buffer and then blit that buffer into place
    // TODO: Try to reuse these
    id<MTLBuffer> buff = [setupInfo.mtlDevice newBufferWithBytes:&uniforms length:sizeof(uniforms) options:MTLResourceStorageModeShared];
    [bltEncode copyFromBuffer:buff sourceOffset:0 toBuffer:sceneRender->setupInfo.uniformBuff->buffer destinationOffset:sceneRender->setupInfo.uniformBuff->offset size:sizeof(uniforms)];
    resources.addEntry(sceneRender->setupInfo.uniformBuff);
}

void SceneRendererMTL::setupLightBuffer(SceneMTL *scene,RendererFrameInfoMTL *frameInfo,id<MTLBlitCommandEncoder> bltEncode,ResourceRefsMTL &resources)
{
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;

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
    
    // Copy this to a buffer and then blit that buffer into place
    // TODO: Try to reuse these
    id<MTLBuffer> buff = [setupInfo.mtlDevice newBufferWithBytes:&lighting length:sizeof(lighting) options:MTLResourceStorageModeShared];
    [bltEncode copyFromBuffer:buff sourceOffset:0 toBuffer:sceneRender->setupInfo.lightingBuff->buffer destinationOffset:sceneRender->setupInfo.lightingBuff->offset size:sizeof(lighting)];
    resources.addEntry(sceneRender->setupInfo.lightingBuff);
}
    
void SceneRendererMTL::setupDrawStateA(WhirlyKitShader::UniformDrawStateA &drawState)
{
    // That was anti-climactic
    bzero(&drawState,sizeof(drawState));
}
    
MTLRenderPipelineDescriptor *SceneRendererMTL::defaultRenderPipelineState(SceneRendererMTL *sceneRender,ProgramMTL *program,RenderTargetMTL *renderTarget)
{
    MTLRenderPipelineDescriptor *renderDesc = [[MTLRenderPipelineDescriptor alloc] init];
    renderDesc.vertexFunction = program->vertFunc;
    renderDesc.fragmentFunction = program->fragFunc;
    
    renderDesc.colorAttachments[0].pixelFormat = renderTarget->getPixelFormat();
    if (renderTarget->getRenderPassDesc().depthAttachment.texture)
        renderDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    
    if (renderTarget->blendEnable) {
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
    
    if (@available(iOS 13.0, *)) {
        renderDesc.supportIndirectCommandBuffers = true;
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

void SceneRendererMTL::updateWorkGroups(RendererFrameInfo *frameInfo)
{
    SceneRenderer::updateWorkGroups(frameInfo);
    
    if (!indirectRender)
        return;
    
    // Build the indirect command buffers if they're available
    if (@available(iOS 13.0, *)) {
        for (auto &workGroup : workGroups) {
            for (auto targetContainer : workGroup->renderTargetContainers) {
                if (targetContainer->drawables.empty() || !targetContainer->modified)
                    continue;
                WorkGroupMTL::RenderTargetContainerMTL *targetContainerMTL = static_cast<WorkGroupMTL::RenderTargetContainerMTL *>(targetContainer.get());
                targetContainerMTL->drawGroups.clear();

                RenderTargetMTLRef renderTarget;
                if (!targetContainer->renderTarget) {
                    // Need some sort of render target even if we're not really rendering
                    renderTarget = std::dynamic_pointer_cast<RenderTargetMTL>(renderTargets.back());
                } else {
                    renderTarget = std::dynamic_pointer_cast<RenderTargetMTL>(targetContainer->renderTarget);
                }

                // Sort the drawables into draw groups by Z buffer usage
                WorkGroupMTL::DrawGroupMTLRef drawGroup;
                bool dgZBufferRead = false, dgZBufferWrite = false;
                for (auto draw : targetContainer->drawables) {
                    DrawableMTL *drawMTL = dynamic_cast<DrawableMTL *>(draw.get());
                    
                    // Sort out what the zbuffer should be
                    bool zBufferWrite = (zBufferMode == zBufferOn);
                    bool zBufferRead = (zBufferMode == zBufferOn);
                    if (renderTarget->getTex() != nil) {
                        // Off screen render targets don't like z buffering
                        zBufferRead = false;
                        zBufferWrite = false;
                    } else {
                        // The drawable itself gets a say
                        zBufferRead = drawMTL->getRequestZBuffer();
                        zBufferWrite = drawMTL->getWriteZbuffer();
                    }

                    // If this isn't compatible with the draw group, create a new one
                    if (!drawGroup || zBufferRead != dgZBufferRead || zBufferWrite != dgZBufferWrite) {
                        // It's not, so we need to make a new draw group
                        drawGroup = WorkGroupMTL::DrawGroupMTLRef(new WorkGroupMTL::DrawGroupMTL());

                        // Depth stencil, which goes in the command encoder later
                        MTLDepthStencilDescriptor *depthDesc = [[MTLDepthStencilDescriptor alloc] init];
                        if (zBufferRead)
                            depthDesc.depthCompareFunction = MTLCompareFunctionLess;
                        else
                            depthDesc.depthCompareFunction = MTLCompareFunctionAlways;
                        depthDesc.depthWriteEnabled = zBufferWrite;
                        
                        drawGroup->depthStencil = [setupInfo.mtlDevice newDepthStencilStateWithDescriptor:depthDesc];
                        
                        targetContainerMTL->drawGroups.push_back(drawGroup);
                        
                        dgZBufferRead = zBufferRead;
                        dgZBufferWrite = zBufferWrite;
                    }
                    drawGroup->drawables.push_back(draw);
                }

                // Command buffer description should be the same
                MTLIndirectCommandBufferDescriptor *cmdBuffDesc = [[MTLIndirectCommandBufferDescriptor alloc] init];
                cmdBuffDesc.commandTypes = MTLIndirectCommandTypeDraw | MTLIndirectCommandTypeDrawIndexed;
                cmdBuffDesc.inheritBuffers = false;
                if (@available(iOS 13.0, *)) {
                    cmdBuffDesc.inheritPipelineState = false;
                }
                // TODO: Should query the drawables to get this maximum number
                cmdBuffDesc.maxVertexBufferBindCount = 12;
                cmdBuffDesc.maxFragmentBufferBindCount = 3;

                // Build up indirect buffers for each draw group
                for (auto drawGroup : targetContainerMTL->drawGroups) {
                    int curCommand = 0;
                    drawGroup->numCommands = drawGroup->drawables.size();
                    drawGroup->indCmdBuff = [setupInfo.mtlDevice newIndirectCommandBufferWithDescriptor:cmdBuffDesc maxCommandCount:drawGroup->numCommands options:MTLResourceStorageModeShared];

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
                            
                            id<MTLIndirectRenderCommand> cmdEncode = [drawGroup->indCmdBuff indirectRenderCommandAtIndex:curCommand++];
                            drawMTL->encodeInirectCalculate(cmdEncode,this,scene,renderTarget.get());
                        }
                    } else {
                        // Work through the drawables
                        for (auto &draw : drawGroup->drawables) {
                            DrawableMTL *drawMTL = dynamic_cast<DrawableMTL *>(draw.get());

                            // Figure out the program to use for drawing
                            SimpleIdentity drawProgramId = drawMTL->getProgram();
                            ProgramMTL *program = (ProgramMTL *)scene->getProgram(drawProgramId);
                            if (!program) {
                                wkLogLevel(Error, "SceneRendererMTL: Drawable without Program");
                                continue;
                            }
                            
                            id<MTLIndirectRenderCommand> cmdEncode = [drawGroup->indCmdBuff indirectRenderCommandAtIndex:curCommand++];

                            // TODO: Handle the offset matrices by encoding twice

                            drawMTL->encodeIndirect(cmdEncode,this,scene,renderTarget.get());
                        }
                    }
                }
                
                targetContainer->modified = false;
            }
        }
    }
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

    RenderTargetMTL *defaultTarget = (RenderTargetMTL *)renderTargets.back().get();
    if (renderPassDesc)
        defaultTarget->setRenderPassDesc(renderPassDesc);
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
            if (targetContainer->drawables.empty())
                continue;
            WorkGroupMTL::RenderTargetContainerMTL *targetContainerMTL = (WorkGroupMTL::RenderTargetContainerMTL *)targetContainer.get();
            
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
            
            // Ask all the drawables to set themselves up.  Mostly memory stuff.
            id<MTLFence> preProcessFence = [mtlDevice newFence];
            id<MTLBlitCommandEncoder> bltEncode = [cmdBuff blitCommandEncoder];
            ResourceRefsMTL resources;
            for (auto &draw : targetContainer->drawables) {
                DrawableMTL *drawMTL = dynamic_cast<DrawableMTL *>(draw.get());
                drawMTL->runTweakers(&baseFrameInfo);
                drawMTL->preProcess(this, cmdBuff, bltEncode, sceneMTL, resources);
            }
            // TODO: Just set this up once and copy it into position
            setupLightBuffer(sceneMTL,&baseFrameInfo,bltEncode,resources);
            [bltEncode updateFence:preProcessFence];
            [bltEncode endEncoding];
                        
            // If we're forcing a mipmap calculation, then we're just going to use this render target once
            // If not, then we run some program over it multiple times
            // TODO: Make the reduce operation more explicit
            int numLevels = renderTarget->numLevels();
            if (renderTarget->mipmapType != RenderTargetMipmapNone)
                numLevels = 1;

            for (unsigned int level=0;level<numLevels;level++) {
                // Regular uniforms are per level, unfortunately
                // Set up and then copied into place
                id<MTLFence> uniformFence = [mtlDevice newFence];
                id<MTLBlitCommandEncoder> bltEncode = [cmdBuff blitCommandEncoder];
                // TODO: Just set this up once and copy it into position
                setupUniformBuffer(&baseFrameInfo,bltEncode,scene->getCoordAdapter(),level,resources);
                [bltEncode updateFence:uniformFence];
                [bltEncode endEncoding];

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
                [cmdEncode waitForFence:preProcessFence beforeStages:MTLRenderStageVertex];
                [cmdEncode waitForFence:uniformFence beforeStages:MTLRenderStageVertex];

                // Wire up all the resources we need to use
                // These are buffers created or used by the various drawables
                resources.use(cmdEncode);

                if (indirectRender) {
                    if (@available(iOS 12.0, *)) {
                        [cmdEncode setCullMode:MTLCullModeFront];
                        for (auto drawGroup : targetContainerMTL->drawGroups) {
                            if (drawGroup->numCommands > 0) {
                                [cmdEncode setDepthStencilState:drawGroup->depthStencil];
                                [cmdEncode executeCommandsInBuffer:drawGroup->indCmdBuff withRange:NSMakeRange(0,drawGroup->numCommands)];
                            }
                        }
                    }
                } else {
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
                            
                            // Run the calculation phase
                            drawMTL->encodeDirectCalculate(&baseFrameInfo,cmdEncode,scene);
                        }
                    } else {
                        // Keep track of state changes for z buffer state
                        bool firstDepthState = true;
                        bool zBufferWrite = (zBufferMode == zBufferOn);
                        bool zBufferRead = (zBufferMode == zBufferOn);

                        bool lastZBufferWrite = zBufferWrite;
                        bool lastZBufferRead = zBufferRead;

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
                                
                                baseFrameInfo.program = program;
                                
                                // Activate the program
                                // TODO: Move this into the drawable
                                program->addResources(&baseFrameInfo, cmdEncode, sceneMTL);
                                                            
                                // "Draw" using the given program
                                drawMTL->encodeDirect(&baseFrameInfo,cmdEncode,scene);
                                
                                // If we had a local matrix, set the frame info back to the general one
                                //            if (localMat)
                                //                offFrameInfo.mvpMat = mvpMat;
                                
                                numDrawables++;
                            }
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
