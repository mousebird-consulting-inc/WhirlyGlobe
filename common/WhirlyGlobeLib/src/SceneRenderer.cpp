/*
 *  SceneRenderer.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
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

#import "SceneRenderer.h"

using namespace Eigen;

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
: sceneRenderer(NULL), theView(NULL), scene(NULL), frameLen(0), currentTime(0),
heightAboveSurface(0), screenSizeInDisplayCoords(0,0), lights(NULL), program(NULL)
{
}

RendererFrameInfo::RendererFrameInfo(const RendererFrameInfo &that)
{
    *this = that;
}

WorkGroup::WorkGroup(GroupType groupType) : groupType(groupType)
{
    switch (groupType) {
        case Calculation:
            // For calculation we don't really have a render target
            renderTargetContainers.push_back(RenderTargetContainerRef(new RenderTargetContainer(RenderTargetRef())));
            break;
        case Offscreen:
            break;
        case ReduceOps:
            break;
        case ScreenRender:
            break;
    }
}

WorkGroup::~WorkGroup()
{
    for (auto &targetCon : renderTargetContainers) {
        for (auto &draw : targetCon->drawables) {
            auto it = draw->workGroupIDs.find(getId());
            if (it != draw->workGroupIDs.end())
                draw->workGroupIDs.erase(it);
        }
    }
}

bool WorkGroup::addDrawable(DrawableRef drawable)
{
    for (auto &renderTargetCon : renderTargetContainers) {
        // If there is no render target set, this is the calculation group
        // Or if there's no render target in the drawable, this is probably the ScreenRender group
        // Or if it actually matches
        if (!renderTargetCon->renderTarget || drawable->getRenderTarget() == EmptyIdentity ||
            renderTargetCon->renderTarget->getId() == drawable->getRenderTarget()) {
            renderTargetCon->drawables.insert(drawable);
            drawable->workGroupIDs.insert(getId());
            return true;
        }
    }
    
    return false;
}

void WorkGroup::removeDrawable(DrawableRef drawable)
{
    for (auto &renderTargetCon : renderTargetContainers) {
        auto it = renderTargetCon->drawables.find(drawable);
        if (it != renderTargetCon->drawables.end()) {
            renderTargetCon->drawables.erase(it);
        }
    }
    
    auto it = drawable->workGroupIDs.find(getId());
    if (it != drawable->workGroupIDs.end()) {
        drawable->workGroupIDs.erase(it);
    }
}

void WorkGroup::addRenderTarget(RenderTargetRef renderTarget)
{
    // See if it's already in here
    for (auto &renderTargetCon : renderTargetContainers)
        if (renderTargetCon->renderTarget && renderTarget->getId() == renderTargetCon->renderTarget->getId()) {
            return;
        }
    
    renderTargetContainers.push_back(RenderTargetContainerRef(new RenderTargetContainer(renderTarget)));
}
    
SceneRenderer::SceneRenderer()
{
}
    
SceneRenderer::~SceneRenderer()
{
}
    
void SceneRenderer::init()
{
    scene = NULL;
    theView = NULL;
    zBufferMode = zBufferOff;
    framebufferWidth = 0;
    framebufferHeight = 0;
    scale = 1.0;
    framesPerSec = 0.0;
    numDrawables = 0;
    perfInterval = 0.0;
    useViewChanged = true;
    triggerDraw = true;
    frameCount = 0;
    frameCountLastChanged = 0;
    frameCountStart = 0.0;
    lastDraw = 0.0;
    renderUntil = 0.0;
    extraFrames = 0;
    clearColor = RGBAColor(0,0,0,0);
    framebufferTex = NULL;

    // Add a simple default light
    DirectionalLight light;
    light.setPos(Vector3f(0.75, 0.5, -1.0));
    light.setViewDependent(true);
    light.setAmbient(Vector4f(0.6, 0.6, 0.6, 1.0));
    light.setDiffuse(Vector4f(0.5, 0.5, 0.5, 1.0));
    light.setSpecular(Vector4f(0, 0, 0, 0));
    addLight(light);
    
    lightsLastUpdated = 0.0;

    // Calculation shaders
    workGroups.push_back(WorkGroupRef(new WorkGroup(WorkGroup::Calculation)));
    // Offscreen target render group
    workGroups.push_back(WorkGroupRef(new WorkGroup(WorkGroup::Offscreen)));
    // Middle one for weird stuff
    workGroups.push_back(WorkGroupRef(new WorkGroup(WorkGroup::ReduceOps)));
    // Last workgroup is used for on screen rendering
    workGroups.push_back(WorkGroupRef(new WorkGroup(WorkGroup::ScreenRender)));
}

Scene *SceneRenderer::getScene()
    { return scene; }

View *SceneRenderer::getView()
    { return theView; }

float SceneRenderer::getScale()
    { return scale; }

void SceneRenderer::setScale(float newScale)
    { scale = newScale; }

void SceneRenderer::setZBufferMode(WhirlyKitSceneRendererZBufferMode inZBufferMode)
    { zBufferMode = inZBufferMode; }

void SceneRenderer::setPerfInterval(int howLong)
    { perfInterval = howLong; }

void SceneRenderer::setUseViewChanged(bool newVal)
    { useViewChanged = newVal; }

void SceneRenderer::setView(View *newView)
    { theView = newView; }
    
void SceneRenderer::addRenderTarget(RenderTargetRef newTarget)
{
    workGroups[WorkGroup::Offscreen]->renderTargetContainers.push_back(WorkGroup::RenderTargetContainerRef(new WorkGroup::RenderTargetContainer(newTarget)));
    renderTargets.insert(renderTargets.begin(),newTarget);
}

void SceneRenderer::addDrawable(DrawableRef newDrawable)
{
    newDrawable->setupForRenderer(getRenderSetupInfo());
    newDrawable->updateRenderer(this);
    
    // This will sort it into the appropriate work group later
    offDrawables.insert(newDrawable);
}

void SceneRenderer::removeDrawable(DrawableRef draw,bool teardown)
{
    // TODO: Can make this simpler
    for (auto &workGroup : workGroups) {
        workGroup->removeDrawable(draw);
    }
    auto it = offDrawables.find(draw);
    if (it != offDrawables.end()) {
        offDrawables.erase(it);
    }
    
    removeContinuousRenderRequest(draw->getId());
    removeExtraFrameRenderRequest(draw->getId());

    if (teardown) {
        // Teardown OpenGL foo
        draw->teardownForRenderer(getRenderSetupInfo(), scene);
    }
}

void SceneRenderer::updateWorkGroups(RendererFrameInfo *frameInfo)
{
    // Look at drawables to move into the active set
    std::vector<DrawableRef> drawsToMoveIn;
    for (auto draw : offDrawables) {
        if (draw->isOn(frameInfo)) {
            bool keep = false;
            // If there's a render target, we need that too
            if (draw->getRenderTarget() != EmptyIdentity) {
                for (auto &renderTarget : renderTargets) {
                    if (draw->getRenderTarget() == renderTarget->getId())
                        keep = true;
                }
            } else
                keep = true;
            if (keep)
                drawsToMoveIn.push_back(draw);
        }
    }
    for (auto draw : drawsToMoveIn) {
        auto it = offDrawables.find(draw);
        if (it != offDrawables.end()) {
            offDrawables.erase(it);
        }

        // If there's a calculation program, it always goes in there
        if (draw->getCalculationProgram() != EmptyIdentity) {
            workGroups[WorkGroup::Calculation]->addDrawable(draw);
        }
        // Sort into offscreen or onscreen buckets
        if (draw->getRenderTarget() != EmptyIdentity) {
            workGroups[WorkGroup::Offscreen]->addDrawable(draw);
        } else {
            workGroups[WorkGroup::ScreenRender]->addDrawable(draw);
        }
    }
    
    // Look for active drawables to move out of the active set
    for (auto workGroup : workGroups) {
        for (auto renderTargetCon : workGroup->renderTargetContainers) {
            std::vector<DrawableRef> drawsToMoveOut;
            for (auto draw : renderTargetCon->drawables) {
                if (!draw->isOn(frameInfo)) {
                    drawsToMoveOut.push_back(draw);
                }
            }
            for (auto draw : drawsToMoveOut) {
                auto it = renderTargetCon->drawables.find(draw);
                if (it != renderTargetCon->drawables.end())
                    renderTargetCon->drawables.erase(it);
                offDrawables.insert(draw);
            }
        }
    }
}

void SceneRenderer::removeRenderTarget(SimpleIdentity targetID)
{
    for (int ii=0;ii<renderTargets.size();ii++)
    {
        RenderTargetRef target = renderTargets[ii];
        if (target->getId() == targetID)
        {
            target->clear();
            renderTargets.erase(renderTargets.begin()+ii);
            break;
        }
    }
    
    for (auto &workGroup : workGroups) {
        int which = 0;
        for (auto &con : workGroup->renderTargetContainers) {
            if (con->renderTarget && con->renderTarget->getId() == targetID) {
                break;
            }
            which++;
        }
        if (which < workGroup->renderTargetContainers.size()) {
            workGroup->renderTargetContainers.erase(workGroup->renderTargetContainers.begin()+which);
            break;
        }
    }
}

void SceneRenderer::defaultTargetInit(RenderTarget *)
    { }

void SceneRenderer::presentRender()
    { }

Point2f SceneRenderer::getFramebufferSize()
{
    return Point2f(framebufferWidth,framebufferHeight);
}

Point2f SceneRenderer::getFramebufferSizeScaled()
{
    return Point2f(framebufferWidth/scale,framebufferHeight/scale);
}
    
void SceneRenderer::setRenderUntil(TimeInterval newRenderUntil)
{
    renderUntil = std::max(renderUntil,newRenderUntil);
}

void SceneRenderer::setTriggerDraw()
{
    triggerDraw = true;
}

void SceneRenderer::addContinuousRenderRequest(SimpleIdentity drawID)
{
    contRenderRequests.insert(drawID);
}

void SceneRenderer::addExtraFrameRenderRequest(SimpleIdentity drawID,int numFrames)
{
    extraFramesPerID[drawID] = numFrames;
}

void SceneRenderer::removeExtraFrameRenderRequest(SimpleIdentity drawID)
{
    auto it = extraFramesPerID.find(drawID);
    if (it != extraFramesPerID.end())
        extraFramesPerID.erase(it);
    
    updateExtraFrames();
}

void SceneRenderer::updateExtraFrames()
{
    extraFrames = 0;
    for (auto it : extraFramesPerID)
        extraFrames = std::max(extraFrames,it.second);
}

void SceneRenderer::removeContinuousRenderRequest(SimpleIdentity drawID)
{
    SimpleIDSet::iterator it = contRenderRequests.find(drawID);
    if (it != contRenderRequests.end())
        contRenderRequests.erase(it);
}


void SceneRenderer::setScene(WhirlyKit::Scene *newScene)
{
    scene = newScene;
    if (scene)
    {
        scene->setRenderer(this);
    }
}

RGBAColor SceneRenderer::getClearColor()
{
    return clearColor;
}

bool SceneRenderer::viewDidChange()
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

void SceneRenderer::forceDrawNextFrame()
{
    triggerDraw = true;
}

void SceneRenderer::forceRenderSetup()
{
    for (auto &renderTarget : renderTargets)
        renderTarget->isSetup = false;
}

/// Add a light to the existing set
void SceneRenderer::addLight(const DirectionalLight &light)
{
    lights.push_back(light);
    if (scene)
        lightsLastUpdated = scene->getCurrentTime();
    triggerDraw = true;
}

/// Replace all the lights at once. nil turns off lighting
void SceneRenderer::replaceLights(const std::vector<DirectionalLight> &newLights)
{
    lights.clear();
    for (auto light : newLights)
        lights.push_back(light);
    
    lightsLastUpdated = scene->getCurrentTime();
    triggerDraw = true;
}

void SceneRenderer::setDefaultMaterial(const Material &mat)
{
    defaultMat = mat;
    lightsLastUpdated = scene->getCurrentTime();
    triggerDraw = true;
}

void SceneRenderer::setClearColor(const RGBAColor &color)
{
    if (renderTargets.empty())
        return;
    
    RenderTargetRef defaultTarget = renderTargets.back();
    defaultTarget->setClearColor(color);
    color.asUnitFloats(defaultTarget->clearColor);
    
    clearColor = color;
    forceRenderSetup();
}

int SceneRenderer::preProcessScene(TimeInterval now)
{
    if (!scene)
        return 0;

    return scene->preProcessChanges(theView, this, now);
}

int SceneRenderer::processScene(TimeInterval now)
{
    if (!scene)
        return 0;
    
    return scene->processChanges(theView,this,now);
}

bool SceneRenderer::hasChanges()
{
    if (scene->hasChanges(scene->getCurrentTime()) || viewDidChange() || !contRenderRequests.empty()) {
        frameCountLastChanged = frameCount;
        return true;
    }
    
    return frameCount - frameCountLastChanged <= extraFrames;
}
    
}
