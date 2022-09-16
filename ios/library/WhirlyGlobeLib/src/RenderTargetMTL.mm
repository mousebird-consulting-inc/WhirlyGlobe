/*
 *  RenderTargetMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
 *  Copyright 2011-2022 mousebird consulting
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
    : pixelFormat(MTLPixelFormatBGRA8Unorm), renderPassDescSetFromOutside(false)
{
    clearOnce = true;
}

RenderTargetMTL::RenderTargetMTL(SimpleIdentity newID)
    : RenderTarget(newID), pixelFormat(MTLPixelFormatBGRA8Unorm), renderPassDescSetFromOutside(false)
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
    TextureBaseRef tex = scene->getTexture(newTargetTexID);
    if (!tex)
        return false;
    
    setTargetTexture(dynamic_cast<TextureBaseMTL *>(tex.get()));
    
    return true;
}

void RenderTargetMTL::clear()
{
    tex = nil;
    renderPassDesc.clear();
    renderPassDescSetFromOutside = false;
}

int RenderTargetMTL::numLevels() const
{
    return renderPassDesc.size();
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
    const int width = [tex width];
    const int height = [tex height];
    const int pixSize = calcPixelSize([tex pixelFormat]);
    
    if (NSMutableData *data = [[NSMutableData alloc] initWithLength:width*height*pixSize])
    if (auto *bytes = [data mutableBytes])
    {
        [tex getBytes:bytes bytesPerRow:width*pixSize fromRegion:region mipmapLevel:0];
        return std::make_shared<RawNSDataReader>(data);
    }
    return RawDataRef();
}

RawDataRef RenderTargetMTL::snapshot(int startX,int startY,int snapWidth,int snapHeight)
{
    if (!tex)
        return RawDataRef();
    
    MTLRegion region = MTLRegionMake2D(startX,startY,snapWidth,snapHeight);
    int pixSize = calcPixelSize([tex pixelFormat]);
    
    NSMutableData *data = [[NSMutableData alloc] initWithLength:snapWidth*snapHeight*pixSize];
#if !TARGET_OS_SIMULATOR
    [tex getBytes:[data mutableBytes] bytesPerRow:width*pixSize fromRegion:region mipmapLevel:0];
#endif
    
    return RawDataRef(new RawNSDataReader(data));
}

RawDataRef RenderTargetMTL::snapshotMinMax()
{
    if (!minMaxOutTex)
        return RawDataRef();
    
    MTLRegion region = MTLRegionMake2D(0,0,2,1);
    int pixSize = calcPixelSize([minMaxOutTex pixelFormat]);
    
    NSMutableData *data = [[NSMutableData alloc] initWithLength:2*1*pixSize];
#if !TARGET_OS_SIMULATOR
    [minMaxOutTex getBytes:[data mutableBytes] bytesPerRow:2*pixSize fromRegion:region mipmapLevel:0];
#endif
        
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

    
void RenderTargetMTL::makeRenderPassDesc()
{
    if (!renderPassDesc.empty() && renderPassDescSetFromOutside)
        return;
    
    renderPassDesc.clear();
    
    // TODO: Only regenerate these if something has changed
    
    int numLevels = 1;
    if (tex) {
        numLevels = tex.mipmapLevelCount;
    }
    
    for (unsigned int ii=0;ii<numLevels;ii++) {
        MTLRenderPassDescriptor *rpd = [[MTLRenderPassDescriptor alloc] init];
        rpd.colorAttachments[0].texture = tex;
        rpd.colorAttachments[0].level = ii;
        if (clearEveryFrame || clearOnce)
            rpd.colorAttachments[0].loadAction = MTLLoadActionClear;
        else
            // TODO: This is more work for the renderer.  Narrow down when to do this
            rpd.colorAttachments[0].loadAction =  MTLLoadActionLoad;
        switch (pixelFormat) {
            // four-component formats
            case MTLPixelFormatBGRA8Unorm:
            case MTLPixelFormatRGBA8Unorm:
            case MTLPixelFormatRGBA8Unorm_sRGB:
            case MTLPixelFormatRGBA8Snorm:
            case MTLPixelFormatRGBA8Uint:
            case MTLPixelFormatRGBA8Sint:
            case MTLPixelFormatBGRA8Unorm_sRGB:
            case MTLPixelFormatRGBA16Unorm:
            case MTLPixelFormatRGBA16Snorm:
            case MTLPixelFormatRGBA16Uint:
            case MTLPixelFormatRGBA16Sint:
            case MTLPixelFormatBGRA10_XR:
            case MTLPixelFormatBGRA10_XR_sRGB:
            case MTLPixelFormatRGBA16Float:
            case MTLPixelFormatRGBA32Float:
            case MTLPixelFormatRGBA32Uint:
            case MTLPixelFormatA1BGR5Unorm:
            case MTLPixelFormatABGR4Unorm:
            case MTLPixelFormatBGR5A1Unorm:
                rpd.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
                break;

            // one-, two-, and three-component formats
            case MTLPixelFormatA8Unorm:
                
            case MTLPixelFormatR8Unorm:
            case MTLPixelFormatR8Snorm:
            case MTLPixelFormatR8Uint:
            case MTLPixelFormatR8Sint:

            case MTLPixelFormatR16Uint:
            case MTLPixelFormatR16Sint:
            case MTLPixelFormatR16Snorm:
            case MTLPixelFormatR16Unorm:
            case MTLPixelFormatR16Float:
            case MTLPixelFormatRG16Float:
            case MTLPixelFormatRG16Unorm:

            case MTLPixelFormatR32Uint:
            case MTLPixelFormatR32Sint:
            case MTLPixelFormatR32Float:

            case MTLPixelFormatRG8Unorm:
            case MTLPixelFormatRG8Unorm_sRGB:
            case MTLPixelFormatRG8Snorm:
            case MTLPixelFormatRG8Uint:
            case MTLPixelFormatRG8Sint:
            case MTLPixelFormatRG16Snorm:
            case MTLPixelFormatRG16Uint:
            case MTLPixelFormatRG16Sint:
            case MTLPixelFormatRG32Float:
            case MTLPixelFormatRG32Uint:
            case MTLPixelFormatRG32Sint:
                
            case MTLPixelFormatB5G6R5Unorm:
            case MTLPixelFormatRGB10A2Unorm:
            case MTLPixelFormatRGB10A2Uint:
            case MTLPixelFormatRG11B10Float:
            case MTLPixelFormatRGB9E5Float:
            case MTLPixelFormatBGR10A2Unorm:
                rpd.colorAttachments[0].clearColor = MTLClearColorMake(clearVal, clearVal, clearVal, clearVal);
                break;
            default:
                NSLog(@"RenderTargetMTL: Unknown Pixel Format.  Not clearing.");
                break;
        }
        if (depthTex) {
            rpd.depthAttachment.texture = depthTex;
        }
        rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        renderPassDesc.push_back(rpd);
    }

    clearOnce = false;
}
    
MTLPixelFormat RenderTargetMTL::getPixelFormat()
{
    return pixelFormat;
}

void RenderTargetMTL::setRenderPassDesc(MTLRenderPassDescriptor *inRenderPassDesc)
{
    renderPassDesc.clear();
    renderPassDesc.push_back(inRenderPassDesc);
    renderPassDescSetFromOutside = true;
}
    
MTLRenderPassDescriptor *RenderTargetMTL::getRenderPassDesc(int level)
{
    if (renderPassDesc.empty())
        makeRenderPassDesc();
    if (level < 0)
        level = 0;
    return renderPassDesc[level];
}

/// Encodes any post processing commands
void RenderTargetMTL::addPostProcessing(id<MTLDevice> mtlDevice,id<MTLCommandBuffer> cmdBuff)
{
    switch (mipmapType) {
        case RenderTargetMipmapNone:
            break;
        case RenderTargetMimpapAverage:
        {
            id <MTLBlitCommandEncoder> encoder = [cmdBuff blitCommandEncoder];
            [encoder generateMipmapsForTexture:tex];
            [encoder endEncoding];
            break;
        }
        case RenderTargetMipmapGauss:
        {
            if (!mipmapKernel) {
                MPSImageGaussianPyramid *blurKernel = [[MPSImageGaussianPyramid alloc] initWithDevice:mtlDevice];
                mipmapKernel = blurKernel;
            }
            if (mipmapKernel) {
                [mipmapKernel encodeToCommandBuffer:cmdBuff inPlaceTexture:&tex fallbackCopyAllocator:nil];
            }
            break;
        }
    }
    
    // Calculate the min/max every frame
    if (@available(iOS 11.0, *)) {
        if (calcMinMax) {
            if (!minMaxOutTex) {
                minMaxKernel = [[MPSImageStatisticsMinAndMax alloc] initWithDevice:mtlDevice];

                MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:[tex pixelFormat] width:2 height:1 mipmapped:false];
                desc.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
                minMaxOutTex = [mtlDevice newTextureWithDescriptor:desc];
            }
            
            [minMaxKernel encodeToCommandBuffer:cmdBuff sourceTexture:tex destinationTexture:minMaxOutTex];
        }
    }
}


void RenderTargetMTL::setClearColor(const RGBAColor &color)
{
    color.asUnitFloats(clearColor);
    
    for (auto rpd : renderPassDesc)
        rpd.colorAttachments[0].clearColor = MTLClearColorMake(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
}

id<MTLTexture> RenderTargetMTL::getTex()
{
    return tex;
}
    
}
