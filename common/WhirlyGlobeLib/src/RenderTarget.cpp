/*
 *  RenderTarget.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/19.
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

#import "RenderTarget.h"
#import "SceneRenderer.h"

namespace WhirlyKit
{
    
RenderTarget::RenderTarget()
{
    init();
}
    
RenderTarget::RenderTarget(SimpleIdentity newID)
: Identifiable(newID)
{
    init();
}
    
RenderTarget::~RenderTarget()
{
}
    
void RenderTarget::init()
{
    width = 0;
    height = 0;
    isSetup = false;
    clearEveryFrame = false;
    clearOnce = false;
    blendEnable = false;
    clearColor[0] = 0.0; clearColor[1] = 0.0; clearColor[2] = 0.0; clearColor[3] = 0.0;
    clearVal = 0.0;
    calcMinMax = false;
    mipmapType = RenderTargetMipmapNone;
}

int RenderTarget::numLevels()
{
    return 0;
}

AddRenderTargetReq::AddRenderTargetReq(SimpleIdentity renderTargetID,int width,int height,SimpleIdentity texID,bool clearEveryFrame,bool blend,const RGBAColor &clearColor, float clearVal, RenderTargetMipmapType mipmapType, bool calcMinMax)
: renderTargetID(renderTargetID), width(width), height(height), texID(texID), clearEveryFrame(clearEveryFrame), blend(blend), clearColor(clearColor), clearVal(clearVal), mipmapType(mipmapType), calcMinMax(calcMinMax)
{
}

// Set up a render target
void AddRenderTargetReq::execute(Scene *scene,SceneRenderer *renderer,View *view)
{
    RenderTargetRef renderTarget = RenderTargetRef(renderer->makeRenderTarget());
    renderTarget->setId(renderTargetID);
    renderTarget->width = width;
    renderTarget->height = height;
    renderTarget->clearEveryFrame = clearEveryFrame;
    renderTarget->clearColor[0] = clearColor.r / 255.0;
    renderTarget->clearColor[1] = clearColor.g / 255.0;
    renderTarget->clearColor[2] = clearColor.b / 255.0;
    renderTarget->clearColor[3] = clearColor.a / 255.0;
    renderTarget->clearVal = clearVal;
    renderTarget->blendEnable = blend;
    renderTarget->setMipmap(mipmapType);
    renderTarget->calcMinMax = calcMinMax;
    renderTarget->init(renderer,scene,texID);
    
    renderer->addRenderTarget(renderTarget);
}

ChangeRenderTargetReq::ChangeRenderTargetReq(SimpleIdentity renderTargetID,SimpleIdentity texID)
: renderTargetID(renderTargetID), texID(texID)
{
}

void ChangeRenderTargetReq::execute(Scene *scene,SceneRenderer *renderer,View *view)
{
    for (RenderTargetRef renderTarget : renderer->renderTargets)
    {
        if (renderTarget->getId() == renderTargetID) {
            renderTarget->setTargetTexture(renderer,scene,texID);
            break;
        }
    }
}

ClearRenderTargetReq::ClearRenderTargetReq(SimpleIdentity targetID)
: renderTargetID(targetID)
{
}

void ClearRenderTargetReq::execute(Scene *scene,SceneRenderer *renderer,View *view)
{
    for (RenderTargetRef renderTarget : renderer->renderTargets)
    {
        if (renderTarget->getId() == renderTargetID) {
            renderTarget->clearOnce = true;
            break;
        }
    }
}

RemRenderTargetReq::RemRenderTargetReq(SimpleIdentity targetID)
: targetID(targetID)
{
}

void RemRenderTargetReq::execute(Scene *scene,SceneRenderer *renderer,View *view)
{
    renderer->removeRenderTarget(targetID);
}

}
