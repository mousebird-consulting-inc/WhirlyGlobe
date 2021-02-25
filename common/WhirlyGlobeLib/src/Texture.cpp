/*
 *  Texture.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
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

#import "Texture.h"
#import "WhirlyKitLog.h"

using namespace WhirlyKit;
using namespace Eigen;

#ifdef __clang_analyzer__
# define ANALYSIS_ASSUME_FREED(X) free(temp);
#else
# define ANALYSIS_ASSUME_FREED(X)
#endif

namespace WhirlyKit
{

// Convert a buffer in RGBA to 2-byte 565
// Code courtesy: http://stackoverflow.com/questions/7930148/opengl-es-on-ios-texture-loading-how-do-i-get-from-a-rgba8888-png-file-to-a-r
RawDataRef ConvertRGBATo565(RawDataRef inData)
{
    const uint32_t pixelCount = inData->getLen()/4;
    void *temp = malloc(pixelCount * 2);
    const uint32_t *inPixel32  = (uint32_t *)inData->getRawData();
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        const uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 3);
        const uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 2);
        const uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 3);
        
        *outPixel16++ = (r << 11) | (g << 5) | (b << 0);
    }

    ANALYSIS_ASSUME_FREED(temp);

    return std::make_shared<RawDataWrapper>(temp,pixelCount*2,true);
}


// Convert a buffer in RGBA to 2-byte 4444
RawDataRef ConvertRGBATo4444(RawDataRef inData)
{
    const uint32_t pixelCount = inData->getLen()/4;
    void *temp = malloc(pixelCount * 2);
    const uint32_t *inPixel32  = (uint32_t *)inData->getRawData();
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        const uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 4);
        const uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 4);
        const uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 4);
        const uint32_t a = (((*inPixel32 >> 24) & 0xFF) >> 4);
        
        *outPixel16++ = (r << 12) | (g << 8) | (b << 4) | (a<< 0);
    }

    ANALYSIS_ASSUME_FREED(temp);

    return std::make_shared<RawDataWrapper>(temp,pixelCount*2,true);
}

// Convert a buffer in RGBA to 2-byte 5551
RawDataRef ConvertRGBATo5551(RawDataRef inData)
{
    const uint32_t pixelCount = inData->getLen()/4;
    void *temp = malloc(pixelCount * 2);
    const uint32_t *inPixel32  = (uint32_t *)inData->getRawData();
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        const uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 3);
        const uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 3);
        const uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 3);
        const uint32_t a = (((*inPixel32 >> 24) & 0xFF) >> 7);
        
        *outPixel16++ = (r << 11) | (g << 6) | (b << 1) | (a << 0);
    }

    ANALYSIS_ASSUME_FREED(temp);

    return std::make_shared<RawDataWrapper>(temp,pixelCount*2,true);
}

// Convert a buffer in A to 1-byte alpha but align it to 32 bits
RawDataRef ConvertAToA(RawDataRef inData,int width,int height)
{
    if (width % 4 == 0)
        return inData;
    
    int extra = 4 - (width % 4);
    if (extra == 4) extra = 0;
    int outWidth = width + extra;
    
    unsigned char *temp = (unsigned char *)malloc(outWidth*height);
    
    const unsigned char *inBytes = (const unsigned char *)inData->getRawData();
    unsigned char *outBytes = (unsigned char *)temp;
    for (int32_t h=0;h<height;h++) {
        bzero(&outBytes[width], extra);
        bcopy(inBytes, outBytes, width);
        inBytes += width;
        outBytes += outWidth;
    }

    ANALYSIS_ASSUME_FREED(temp);

    return std::make_shared<RawDataWrapper>(temp,outWidth*height,true);
}

// Convert a buffer in RG to a 2-byte RG but align it to 32 bits
RawDataRef ConvertRGToRG(RawDataRef inData,int width,int height)
{
    if (width % 2 == 0)
        return inData;
    
    int extra = 2 - (width % 2);
    if (extra == 2) extra = 0;
    const int outWidth = width + extra;
    
    unsigned char *temp = (unsigned char *)malloc(outWidth*height*2);
    
    const unsigned char *inBytes = (const unsigned char *)inData->getRawData();
    unsigned char *outBytes = (unsigned char *)temp;
    for (int32_t h=0;h<height;h++) {
        bzero(&outBytes[width], 2*extra);
        bcopy(inBytes, outBytes, 2*width);
        inBytes += 2*width;
        outBytes += 2*outWidth;
    }

    ANALYSIS_ASSUME_FREED(temp);

    return std::make_shared<RawDataWrapper>(temp,outWidth*height*2,true);
}

RawDataRef ConvertRGBATo16(RawDataRef inData,int width,int height,bool pad)
{
    int extra = 2 - (width % 2);
    if (extra == 2) extra = 0;
    
    // Metal doesn't seem to care if we pad
    if (!pad)
        extra = 0;
    
    const int outWidth = width + extra;

    unsigned char *temp = (unsigned char *)malloc(outWidth*height*2);
    bzero(temp,outWidth*height*2);
    
    const uint32_t *inPixel32row  = (uint32_t *)inData->getRawData();
    uint8_t *outPixel8row = (uint8_t *)temp;
    for (int32_t h=0;h<height;h++) {
        const uint32_t *inPixel32 = inPixel32row;
        uint8_t *outPixel8 = outPixel8row;
        for (int32_t w=0;w<width;w++) {
            uint32_t r = ((*inPixel32 >> 0)  & 0xFF);
            uint32_t g = ((*inPixel32 >> 8)  & 0xFF);
            outPixel8[0] = r;
            outPixel8[1] = g;
            
            inPixel32++;
            outPixel8+=2;
        }
        
        inPixel32row += width;
        outPixel8row += 2*outWidth;
    }

    ANALYSIS_ASSUME_FREED(temp);

    return std::make_shared<RawDataWrapper>(temp,outWidth*height*2,true);
}

// Convert a buffer in RGBA to 1-byte alpha
RawDataRef ConvertRGBATo8(RawDataRef inData,WKSingleByteSource source)
{
    const uint32_t pixelCount = inData->getLen()/4;
    void *temp = malloc(pixelCount);
    const uint32_t *inPixel32  = (uint32_t *)inData->getRawData();
    uint8_t *outPixel8 = (uint8_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        const uint32_t r = ((*inPixel32 >> 0)  & 0xFF);
        const uint32_t g = ((*inPixel32 >> 8)  & 0xFF);
        const uint32_t b = ((*inPixel32 >> 16) & 0xFF);
        const uint32_t a = ((*inPixel32 >> 24) & 0xFF);
        int sum = 0;
        switch (source)
        {
            case WKSingleRed:
                sum = r;
                break;
            case WKSingleGreen:
                sum = g;
                break;
            case WKSingleBlue:
                sum = b;
                break;
            case WKSingleRGB:
                sum = ((int)r + (int)g + (int)b)/3;
                break;
            case WKSingleAlpha:
                sum = a;
                break;
        }
        *outPixel8++ = (uint8_t)sum;
    }

    ANALYSIS_ASSUME_FREED(temp);

    return std::make_shared<RawDataWrapper>(temp,pixelCount,true);
}
    
TextureBase::TextureBase(SimpleIdentity thisId)
: Identifiable(thisId)
{
}

TextureBase::TextureBase()
{
}

TextureBase::TextureBase(const std::string &name) : name(name)
{
}
    
TextureBase::~TextureBase()
{
}

Texture::Texture()
: TextureBase(), isPVRTC(false), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(TexTypeUnsignedByte), byteSource(WKSingleRGB), interpType(TexInterpLinear), isEmptyTexture(false)
{    
}

Texture::Texture(const std::string &name)
	: TextureBase(name), isPVRTC(false), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(TexTypeUnsignedByte), byteSource(WKSingleRGB), interpType(TexInterpLinear), isEmptyTexture(false)
{
}

// Construct with raw texture data
Texture::Texture(const std::string &name,RawDataRef texData,bool isPVRTC)
	: TextureBase(name), texData(texData), isPVRTC(isPVRTC), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(TexTypeUnsignedByte), byteSource(WKSingleRGB), interpType(TexInterpLinear), isEmptyTexture(false)
{ 
}

Texture::~Texture()
{
}
    
void Texture::setRawData(RawData *rawData,int inWidth,int inHeight)
{
    texData = RawDataRef(rawData);
    width = inWidth;
    height = inHeight;
}

RawDataRef Texture::processData()
{
    if (!texData)
        return NULL;
    
	if (isPVRTC || isPKM)
	{
        return texData;
	} else {
        // Depending on the format, we may need to mess around with the bytes
        switch (format)
        {
            case TexTypeUnsignedByte:
            default:
                return texData;
                break;
            case TexTypeShort565:
                return ConvertRGBATo565(texData);
                break;
            case TexTypeShort4444:
                return ConvertRGBATo4444(texData);
                break;
            case TexTypeShort5551:
                return ConvertRGBATo5551(texData);
                break;
            case TexTypeSingleChannel:
                if (texData->getLen() == width * height)
                    return ConvertAToA(texData,width,height);
                return ConvertRGBATo8(texData,byteSource);
                break;
            case TexTypeDoubleChannel:
                if (texData->getLen()  == width * height * 2)
                    return ConvertRGToRG(texData,width,height);
                else if (texData->getLen() == width * height * 4)
                    return ConvertRGBATo16(texData,width,height,true);
                wkLogLevel(Error,"Texture: Not handling RG conversion case.");
                break;
//            case GL_COMPRESSED_RGB8_ETC2:
//                // Can't convert this (for now)
//                return texData;
//                break;
        }
	}
    
    return RawDataRef();
}
    
void Texture::setPKMData(RawDataRef inData)
{
    texData = inData;
    isPKM = true;
}

}
