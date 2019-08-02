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
#import "RawData_NSData.h"

namespace WhirlyKit
{

RenderTargetMTL::RenderTargetMTL()
    : renderPassDesc(nil), pixelFormat(MTLPixelFormatBGRA8Unorm)
{
}

RenderTargetMTL::RenderTargetMTL(SimpleIdentity newID)
    : RenderTarget(newID), renderPassDesc(nil), pixelFormat(MTLPixelFormatBGRA8Unorm)
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
    // Render pass descriptor handles this
}

RawDataRef RenderTargetMTL::snapshot()
{
    if (!tex)
        return RawDataRef();
    
    MTLRegion region = MTLRegionMake2D(0,0,[tex width],[tex height]);
    int width = [tex width], height = [tex height];
    int pixSize = 4;
    MTLPixelFormat pixFormat = [tex pixelFormat];
    // TODO: Fix this pixel size hack
    if (pixFormat != MTLPixelFormatRGBA8Unorm && pixFormat != MTLPixelFormatBGRA8Unorm) {
        pixSize = 2;
    }
    
    NSMutableData *data = [[NSMutableData alloc] initWithLength:width*height*pixSize];
    [tex getBytes:[data mutableBytes] bytesPerRow:width*pixSize fromRegion:region mipmapLevel:0];
    
    return RawDataRef(new RawNSDataReader(data));
}

RawDataRef RenderTargetMTL::snapshot(int startX,int startY,int snapWidth,int snapHeight)
{
    if (!tex)
        return RawDataRef();
    
    MTLRegion region = MTLRegionMake2D(startX,startY,snapWidth,snapHeight);
    int pixSize = 4;
    MTLPixelFormat pixFormat = [tex pixelFormat];
    // TODO: Fix this pixel size hack
    if (pixFormat != MTLPixelFormatRGBA8Unorm || pixFormat != MTLPixelFormatBGRA8Unorm) {
        pixSize = 2;
    }
    
    NSMutableData *data = [[NSMutableData alloc] initWithCapacity:snapWidth*snapHeight*pixSize];
    [tex getBytes:[data mutableBytes] bytesPerRow:snapWidth*pixSize fromRegion:region mipmapLevel:0];
    
    return RawDataRef(new RawNSDataReader(data));
}

void RenderTargetMTL::setTargetTexture(TextureBaseMTL *inTex)
{
    if (!inTex)
        return;
    
    // We need our own little render pass when we go out to a texture
    TextureBaseMTL *theTex = (TextureBaseMTL *)inTex;
    tex = theTex->getMTLID();
    
    pixelFormat = inTex->getMTLID().pixelFormat;
    renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    renderPassDesc.colorAttachments[0].texture = tex;
    if (this->clearEveryFrame)
        renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
}
    
void RenderTargetMTL::setClearColor(const RGBAColor &color)
{
    color.asUnitFloats(clearColor);
    
    if (renderPassDesc)
        renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
}

    
}
