/*
 *  RenderTargetMTL.mm
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

#import "RenderTargetMTL.h"

namespace WhirlyKit
{

RenderTargetMTL::RenderTargetMTL()
    : renderPassDesc(nil)
{
}

RenderTargetMTL::RenderTargetMTL(SimpleIdentity newID)
    : RenderTarget(newID), renderPassDesc(nil)
{
}

RenderTargetMTL::~RenderTargetMTL()
{
}

bool RenderTargetMTL::init(SceneRenderer *renderer,Scene *scene,SimpleIdentity targetTexID)
{
    // This is not the screen and so we must set things up
    if (targetTexID != EmptyIdentity)
        setTargetTexture(renderer, scene, targetTexID);
    
    return false;
}

bool RenderTargetMTL::setTargetTexture(SceneRenderer *renderer,Scene *scene,SimpleIdentity newTargetTexID)
{
    TextureBase *tex = scene->getTexture(newTargetTexID);
    if (!tex)
        return false;
    
    setTargetTexture(dynamic_cast<TextureBaseMTL *>(tex));
    
    return true;
}

void RenderTargetMTL::clear()
{
    // TODO: Implement
}

RawDataRef RenderTargetMTL::snapshot()
{
    // TODO: Implement
    return RawDataRef();
}

RawDataRef RenderTargetMTL::snapshot(int startX,int startY,int snapWidth,int snapHeight)
{
    // TODO: Implement
    return RawDataRef();
}

void RenderTargetMTL::setTargetTexture(TextureBaseMTL *inTex)
{
    if (!inTex)
        return;
    
    // We need our own little render pass when we go out to a texture
    TextureBaseMTL *tex = (TextureBaseMTL *)inTex;
    
    renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    renderPassDesc.colorAttachments[0].texture = tex->getMTLID();
    if (this->clearEveryFrame)
        renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
}
    
}
