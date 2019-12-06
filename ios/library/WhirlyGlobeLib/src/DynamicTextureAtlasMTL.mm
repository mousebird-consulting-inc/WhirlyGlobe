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
: DynamicTexture(name), TextureBase(name), TextureBaseMTL(name), valid(false), bytesPerRow(0)
{
}

void DynamicTextureMTL::setup(int texSize,int cellSize,TextureType inType,bool clearTextures)
{
    DynamicTexture::setup(texSize,cellSize,inType,clearTextures);
    
    switch (inType)
    {
        case TexTypeUnsignedByte:
            bytesPerRow = texSize * 4;
            pixFormat = MTLPixelFormatRGBA8Unorm;
            type = inType;
            break;
        case TexTypeSingleChannel:
            bytesPerRow = texSize;
            pixFormat = MTLPixelFormatA8Unorm;
            type = inType;
            break;
        case TexTypeShort565:
            bytesPerRow = texSize * 2;
            pixFormat = MTLPixelFormatB5G6R5Unorm;
            type = inType;
            break;
        case TexTypeShort4444:
            bytesPerRow = texSize * 2;
            pixFormat = MTLPixelFormatABGR4Unorm;
            type = inType;
            break;
        case TexTypeShort5551:
            bytesPerRow = texSize * 2;
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
    if (mtlID)
        return true;
    
    if (type != TexTypeUnsignedByte && type != TexTypeSingleChannel)
        return false;
    RenderSetupInfoMTL *setupInfo = (RenderSetupInfoMTL *)inSetupInfo;
    
    // Set up an empty texture
    MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixFormat width:texSize height:texSize mipmapped:false];
    mtlID = [setupInfo->mtlDevice newTextureWithDescriptor:desc];
    if (!this->name.empty())
        mtlID.label = [NSString stringWithFormat:@"%s dynamic texture",this->name.c_str()];
    if (!mtlID) {
        valid = false;
        return false;
    }

    return valid;
}

void DynamicTextureMTL::destroyInRenderer(const RenderSetupInfo *setupInfo,Scene *scene)
{
    mtlID = nil;
}

void DynamicTextureMTL::addTextureData(int startX,int startY,int width,int height,RawDataRef data)
{
    MTLRegion region = MTLRegionMake2D(startX,startY,width,height);
    [mtlID replaceRegion:region mipmapLevel:0 withBytes:data->getRawData() bytesPerRow:width*4];    
}

void DynamicTextureMTL::clearTextureData(int startX,int startY,int width,int height,ChangeSet &changes,bool mainThreadMerge,unsigned char *emptyData)
{
    if (!clearTextures)
        return;

    // TODO: Implement this with a blit encoder
}
    
}
