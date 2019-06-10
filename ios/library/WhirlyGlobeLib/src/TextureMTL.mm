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
#import <Accelerate/Accelerate.h>

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
    
RawDataRef TextureMTL::convertData()
{
    NSMutableData *outData = nil;

    vImage_Buffer srcBuff;
    srcBuff.data = (void *)texData->getRawData();
    srcBuff.width = width;
    srcBuff.height = height;
    srcBuff.rowBytes = 4*width;

    switch (format) {
        case TexTypeUnsignedByte:
        case TexTypeSingleChannel:
        case TexTypeDoubleChannel:
            break;
        case TexTypeShort565:
            {
                outData = [[NSMutableData alloc] initWithCapacity:width*height*2];
                vImage_Buffer destBuff;
                destBuff.data = (void *)[outData bytes];
                destBuff.width = width;
                destBuff.height = height;
                destBuff.rowBytes = 2*width;
                vImageConvert_RGBA8888toRGB565(&srcBuff,&destBuff,kvImageNoFlags);
            }
            break;
        case TexTypeShort4444:
        {
            NSLog(@"TextureMTL: 4444 image format not support on Metal.");
        }
            break;
        case TexTypeShort5551:
        {
            outData = [[NSMutableData alloc] initWithCapacity:width*height*2];
            vImage_Buffer destBuff;
            destBuff.data = (void *)[outData bytes];
            destBuff.width = width;
            destBuff.height = height;
            destBuff.rowBytes = 2*width;
            vImageConvert_RGBA8888toRGBA5551(&srcBuff,&destBuff,kvImageNoFlags);
        }
            break;
    }

    if (outData)
        return RawDataRef(new RawNSDataReader(outData));
    else
        return texData;
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
            pixFormat = MTLPixelFormatRGBA8Unorm;
            // Note: Render target.  this is goofy
            if (!texData)
                pixFormat = MTLPixelFormatBGRA8Unorm;
            bytesPerRow = 4*width;
            break;
        case TexTypeShort565:
            // TODO: These aren't the right order
            pixFormat = MTLPixelFormatB5G6R5Unorm;
            bytesPerRow = 2*width;
            break;
        case TexTypeShort4444:
            // TODO: These aren't the right order
            pixFormat = MTLPixelFormatABGR4Unorm;
            bytesPerRow = 2*width;
            break;
        case TexTypeShort5551:
            // TODO: These aren't the right order
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
    
    // If there's no data, then we're using this as a target
    if (!texData) {
        desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    }
    
    mtlID = [setupInfo->mtlDevice newTextureWithDescriptor:desc];
    if (mtlID) {
        MTLRegion region = MTLRegionMake2D(0,0,width,height);
        if (texData) {
            RawDataRef convData = convertData();
            [mtlID replaceRegion:region mipmapLevel:0 withBytes:convData->getRawData() bytesPerRow:bytesPerRow];
        }
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
