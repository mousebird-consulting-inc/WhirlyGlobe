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
    sortAlphaToEnd = false;
    depthBufferOffForAlpha = false;
    triggerDraw = true;
    frameCount = 0;
    frameCountStart = 0.0;
    lastDraw = 0.0;
    renderUntil = 0.0;
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
    renderTargets.insert(renderTargets.begin(),newTarget);
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

void SceneRenderer::processScene()
{
    if (!scene)
        return;
    
    scene->processChanges(theView,this,scene->getCurrentTime());
}

bool SceneRenderer::hasChanges()
{
    return scene->hasChanges(scene->getCurrentTime()) || viewDidChange() || !contRenderRequests.empty();
}
    
}
