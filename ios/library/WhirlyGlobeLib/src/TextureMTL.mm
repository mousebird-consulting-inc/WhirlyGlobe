/*
 *  TextureMTL.mm
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

#import "TextureMTL.h"
#import "UIImage+Stuff.h"
#import "RawData_NSData.h"

namespace WhirlyKit
{

TextureMTL::TextureMTL(const std::string &name)
    : Texture(name), TextureBaseMTL(name), TextureBase(name)
{
}

TextureMTL::TextureMTL(const std::string &name,RawDataRef texData,bool isPVRTC)
    : Texture(name,texData,isPVRTC), TextureBaseMTL(name), TextureBase(name)
{
}
    
TextureMTL::TextureMTL(const std::string &name,UIImage *inImage,int inWidth,int inHeight)
    : Texture(name), TextureBase(name), TextureBaseMTL(name)
{
    NSData *data = [inImage rawDataScaleWidth:inWidth height:inHeight border:0];
    if (!data)
        return;
    width = inWidth;  height = inHeight;
    
    texData = RawDataRef(new RawNSDataReader(data));
}

bool TextureMTL::createInRenderer(const RenderSetupInfo *inSetupInfo)
{
    if (!texData && !isEmptyTexture)
        return false;
    
    if (mtlID)
        return true;
    
    RenderSetupInfoMTL *setupInfo = (RenderSetupInfoMTL *)inSetupInfo;
    
    MTLPixelFormat pixFormat = MTLPixelFormatR32Uint;
    int bytesPerRow = 0;
    
    // TODO: Missing all the compressed formats
    switch (format)
    {
        case TexTypeUnsignedByte:
            pixFormat = MTLPixelFormatR32Uint;
            bytesPerRow = 4*width;
            break;
        case TexTypeShort565:
            pixFormat = MTLPixelFormatB5G6R5Unorm;
            bytesPerRow = 2*width;
            break;
        case TexTypeShort4444:
            pixFormat = MTLPixelFormatABGR4Unorm;
            bytesPerRow = 2*width;
            break;
        case TexTypeShort5551:
            pixFormat = MTLPixelFormatA1BGR5Unorm;
            bytesPerRow = 2*width;
            break;
        case TexTypeSingleChannel:
            pixFormat = MTLPixelFormatA8Unorm;
            // Nudge up the size a bit
            bytesPerRow = ceil(width / 4.0)*4;
            break;
        case TexTypeDoubleChannel:
            pixFormat = MTLPixelFormatRG8Unorm;
            bytesPerRow = 2*width;
            break;
    }
    
    // Set up the texture and upload the data
    MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixFormat width:width height:height mipmapped:usesMipmaps];
    mtlID = [setupInfo->mtlDevice newTextureWithDescriptor:desc];
    if (mtlID) {
        MTLRegion region = MTLRegionMake2D(0,0,width,height);
        [mtlID replaceRegion:region mipmapLevel:0 withBytes:texData.get() bytesPerRow:bytesPerRow];
    }
    
    texData.reset();
    
    return mtlID != nil;
}

void TextureMTL::destroyInRenderer(const RenderSetupInfo *inSetupInfo)
{
    // Seriously?  <swoon>
    mtlID = nil;
}
    
}
