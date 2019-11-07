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
    clearOnce = true;
}

RenderTargetMTL::RenderTargetMTL(SimpleIdentity newID)
    : RenderTarget(newID), renderPassDesc(nil), pixelFormat(MTLPixelFormatBGRA8Unorm)
{
    clearOnce = true;
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
    tex = nil;
    renderPassDesc = nil;
}
 
// TODO: Move this somewhere else
static size_t calcPixelSize(MTLPixelFormat pixFormat)
{
    int pixSize = 4;
    // TODO: Fix this pixel size hack
    switch (pixFormat) {
        case MTLPixelFormatR16Float:
            pixSize = 2;
            break;
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatBGRA8Unorm:
        case MTLPixelFormatR32Float:
        case MTLPixelFormatRG16Float:
            pixSize = 4;
            break;
        case MTLPixelFormatRG32Float:
            pixSize = 8;
            break;
        case MTLPixelFormatRGBA16Float:
            pixSize = 16;
            break;
        default:
            // TODO: Fill in the rest of these
            pixSize = 4;
    }
    
    return pixSize;
}

RawDataRef RenderTargetMTL::snapshot()
{
    if (!tex)
        return RawDataRef();
    
    MTLRegion region = MTLRegionMake2D(0,0,[tex width],[tex height]);
    int width = [tex width], height = [tex height];
    int pixSize = calcPixelSize([tex pixelFormat]);
    
    NSMutableData *data = [[NSMutableData alloc] initWithLength:width*height*pixSize];
    [tex getBytes:[data mutableBytes] bytesPerRow:width*pixSize fromRegion:region mipmapLevel:0];
    
    return RawDataRef(new RawNSDataReader(data));
}

RawDataRef RenderTargetMTL::snapshot(int startX,int startY,int snapWidth,int snapHeight)
{
    if (!tex)
        return RawDataRef();
    
    MTLRegion region = MTLRegionMake2D(startX,startY,snapWidth,snapHeight);
    int pixSize = calcPixelSize([tex pixelFormat]);
    
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
}

void RenderTargetMTL::setTargetDepthTexture(TextureBaseMTL *inDepthTex)
{
    depthTex = inDepthTex->getMTLID();
    depthPixelFormat = inDepthTex->getMTLID().pixelFormat;
}

    
MTLRenderPassDescriptor *RenderTargetMTL::makeRenderPassDesc()
{
    renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    renderPassDesc.colorAttachments[0].texture = tex;
    if (clearEveryFrame || clearOnce)
        renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    else
        // TODO: This is more work for the renderer.  Narrow down when to do this
        renderPassDesc.colorAttachments[0].loadAction =  MTLLoadActionLoad;
    switch (pixelFormat) {
        case MTLPixelFormatBGRA8Unorm:
            renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
            break;
        case MTLPixelFormatR32Float:
        case MTLPixelFormatRG32Float:
        case MTLPixelFormatRGBA32Float:
        case MTLPixelFormatRG16Float:
        case MTLPixelFormatRGBA16Float:
            renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(clearVal, clearVal, clearVal, clearVal);
            break;
        default:
            NSLog(@"RenderTargetMTL: Unknown Pixel Format.  Not clearing.");
            break;
    }
    if (depthTex) {
        renderPassDesc.depthAttachment.texture = depthTex;
    }
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    clearOnce = false;
    
    return renderPassDesc;
}
    
MTLPixelFormat RenderTargetMTL::getPixelFormat()
{
    return pixelFormat;
}
    
MTLRenderPassDescriptor *RenderTargetMTL::getRenderPassDesc()
{
    if (!renderPassDesc)
        makeRenderPassDesc();
    return renderPassDesc;
}
    
void RenderTargetMTL::setClearColor(const RGBAColor &color)
{
    color.asUnitFloats(clearColor);
    
    if (renderPassDesc)
        renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
}

id<MTLTexture> RenderTargetMTL::getTex()
{
    return tex;
}
    
}
