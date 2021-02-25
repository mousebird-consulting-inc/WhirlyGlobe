/*
 *  DynamicTextureAtlasMTL.mm
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

#import "DynamicTextureAtlasMTL.h"

namespace WhirlyKit
{
    
DynamicTextureMTL::DynamicTextureMTL(const std::string &name)
: DynamicTexture(name), TextureBase(name), TextureBaseMTL(name), valid(false), bytesPerRow(0), bytesPerPixel(0)
{
}

void DynamicTextureMTL::setup(int texSize,int cellSize,TextureType inType,bool clearTextures)
{
    DynamicTexture::setup(texSize,cellSize,inType,clearTextures);
    
    switch (inType)
    {
        case TexTypeUnsignedByte:
            bytesPerPixel = 4;
            bytesPerRow = texSize * bytesPerPixel;
            pixFormat = MTLPixelFormatRGBA8Unorm;
            type = inType;
            break;
        case TexTypeSingleChannel:
            bytesPerPixel = 1;
            bytesPerRow = texSize;
            pixFormat = MTLPixelFormatA8Unorm;
            type = inType;
            break;
        case TexTypeShort565:
            bytesPerPixel = 2;
            bytesPerRow = texSize * bytesPerPixel;
            pixFormat = MTLPixelFormatB5G6R5Unorm;
            type = inType;
            break;
        case TexTypeShort4444:
            bytesPerPixel = 2;
            bytesPerRow = texSize * bytesPerPixel;
            pixFormat = MTLPixelFormatABGR4Unorm;
            type = inType;
            break;
        case TexTypeShort5551:
            bytesPerPixel = 2;
            bytesPerRow = texSize * bytesPerPixel;
            pixFormat = MTLPixelFormatBGR5A1Unorm;
            type = inType;
            break;
        default:
            NSLog(@"DynamicTextureMTL: Unrecognized texture type.");
            return;
            break;
    }
    
    valid = true;
}

bool DynamicTextureMTL::createInRenderer(const RenderSetupInfo *inSetupInfo)
{
    if (texBuf.tex)
        return true;
    
    RenderSetupInfoMTL *setupInfo = (RenderSetupInfoMTL *)inSetupInfo;
    
    // Set up an empty texture
    MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixFormat width:texSize height:texSize mipmapped:false];
    texBuf = setupInfo->heapManage.newTextureWithDescriptor(desc,bytesPerRow * texSize);
    if (!this->name.empty())
        texBuf.tex.label = [NSString stringWithFormat:@"%s dynamic texture",this->name.c_str()];
    if (!texBuf.tex) {
        valid = false;
        return false;
    }

    return valid;
}

void DynamicTextureMTL::destroyInRenderer(const RenderSetupInfo *setupInfo,Scene *scene)
{
    texBuf.tex = nil;
}

void DynamicTextureMTL::addTextureData(int startX,int startY,int width,int height,RawDataRef data)
{
    MTLRegion region = MTLRegionMake2D(startX,startY,width,height);
    [texBuf.tex replaceRegion:region mipmapLevel:0 withBytes:data->getRawData() bytesPerRow:width*bytesPerPixel];    
}

void DynamicTextureMTL::clearTextureData(int startX,int startY,int width,int height,ChangeSet &changes,bool mainThreadMerge,unsigned char *emptyData)
{
    if (!clearTextures)
        return;

    // TODO: Implement this with a blit encoder
}
    
}
