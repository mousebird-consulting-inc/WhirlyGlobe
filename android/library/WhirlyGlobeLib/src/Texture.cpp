/*
 *  Texture.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
 *  Copyright 2011-2016 mousebird consulting
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

#import "GLUtils.h"
#import "Texture.h"

using namespace WhirlyKit;

// Convert a buffer in RGBA to 2-byte 565
// Code courtesy: http://stackoverflow.com/questions/7930148/opengl-es-on-ios-texture-loading-how-do-i-get-from-a-rgba8888-png-file-to-a-r
RawData *ConvertRGBATo565(RawDataRef inData)
{
    uint32_t pixelCount = inData->getLen()/4;
    void *temp = malloc(pixelCount * 2);
    const uint32_t *inPixel32  = (uint32_t *)inData->getRawData();
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 3);
        uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 2);
        uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 3);
        
        *outPixel16++ = (r << 11) | (g << 5) | (b << 0);
    }
    
    return new RawDataWrapper(temp,pixelCount*2,true);
}


// Convert a buffer in RGBA to 2-byte 4444
RawData *ConvertRGBATo4444(RawDataRef inData)
{
    uint32_t pixelCount = inData->getLen()/4;
    void *temp = malloc(pixelCount * 2);
    const uint32_t *inPixel32  = (uint32_t *)inData->getRawData();
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 4);
        uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 4);
        uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 4);
        uint32_t a = (((*inPixel32 >> 24) & 0xFF) >> 4);
        
        *outPixel16++ = (r << 12) | (g << 8) | (b << 4) | (a<< 0);
    }
    
    return new RawDataWrapper(temp,pixelCount*2,true);
}

// Convert a buffer in RGBA to 2-byte 5551
RawData *ConvertRGBATo5551(RawDataRef inData)
{
    uint32_t pixelCount = inData->getLen()/4;
    void *temp = malloc(pixelCount * 2);
    uint32_t *inPixel32  = (uint32_t *)inData->getRawData();
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 3);
        uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 3);
        uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 3);
        uint32_t a = (((*inPixel32 >> 24) & 0xFF) >> 7);
        
        *outPixel16++ = (r << 11) | (g << 6) | (b << 1) | (a << 0);
    }
    
    return new RawDataWrapper(temp,pixelCount*2,true);
}

// Convert a buffer in RGBA to 1-byte alpha
RawData *ConvertRGBATo8(RawDataRef inData,WKSingleByteSource source)
{
    uint32_t pixelCount = inData->getLen()/4;
    void *temp = malloc(pixelCount);
    uint32_t *inPixel32  = (uint32_t *)inData->getRawData();
    uint8_t *outPixel8 = (uint8_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        uint32_t r = ((*inPixel32 >> 0)  & 0xFF);
        uint32_t g = ((*inPixel32 >> 8)  & 0xFF);
        uint32_t b = ((*inPixel32 >> 16) & 0xFF);
        uint32_t a = ((*inPixel32 >> 24) & 0xFF);
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
    
    return new RawDataWrapper(temp,pixelCount,true);
}

namespace WhirlyKit
{
	
Texture::Texture(const std::string &name)
	: TextureBase(name), isPVRTC(false), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(GL_UNSIGNED_BYTE), byteSource(WKSingleRGB), interpType(GL_LINEAR)
{
}
	
// Construct with raw texture data
Texture::Texture(const std::string &name,RawDataRef texData,bool isPVRTC)
	: TextureBase(name), texData(texData), isPVRTC(isPVRTC), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(GL_UNSIGNED_BYTE), byteSource(WKSingleRGB), interpType(GL_LINEAR)
{ 
}

Texture::~Texture()
{
}
    
void Texture::setRawData(RawData *inRawData,int inWidth,int inHeight)
{
    texData = RawDataRef(inRawData);
    width = inWidth;
    height = inHeight;
}

RawDataRef Texture::processData()
{
    if (isPVRTC)
    {
        return texData;
    } else {
        // Depending on the format, we may need to mess around with the bytes
        switch (format)
        {
            case GL_UNSIGNED_BYTE:
            default:
                return texData;
                break;
            case GL_UNSIGNED_SHORT_5_6_5:
                return RawDataRef(ConvertRGBATo565(texData));
                break;
            case GL_UNSIGNED_SHORT_4_4_4_4:
                return RawDataRef(ConvertRGBATo4444(texData));
                break;
            case GL_UNSIGNED_SHORT_5_5_5_1:
                return RawDataRef(ConvertRGBATo5551(texData));
                break;
            case GL_ALPHA:
                return RawDataRef(ConvertRGBATo8(texData,byteSource));
                break;
                // Note: Porting
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

// Figure out the PKM data
unsigned char *Texture::ResolvePKM(RawDataRef texData,int &pkmType,int &size,int &width,int &height)
{
    // Note: Porting
    return NULL;
#if 0
    
    if (texData->getLen() < 16)
        return NULL;
    const unsigned char *header = (const unsigned char *)texData->getRawData();
//    unsigned short *version = (unsigned short *)&header[4];
    const unsigned char *type = &header[7];
    
    // Verify the magic number
    if (strncmp((char *)header, "PKM ", 4))
        return NULL;
    
    // Resolve the GL type
    int glType = -1;
    switch (*type)
    {
        case 0:
            // ETC1 not supported
            break;
        case 1:
            glType = GL_COMPRESSED_RGB8_ETC2;
            break;
        case 2:
            // Unused
            break;
        case 3:
            glType = GL_COMPRESSED_RGBA8_ETC2_EAC;
            break;
        case 4:
            glType = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
            break;
        case 5:
            glType = GL_COMPRESSED_R11_EAC;
            break;
        case 6:
            glType = GL_COMPRESSED_RG11_EAC;
            break;
        case 7:
            glType = GL_COMPRESSED_SIGNED_R11_EAC;
            break;
        case 8:
            glType = GL_COMPRESSED_SIGNED_RG11_EAC;
            break;
    }
    if (glType == -1)
        return NULL;
    pkmType = glType;

    width = (header[8] << 8) | header[9];
    height = (header[10] << 8) | header[11];;
    // Skipping original width/height

    size = width * height / 2;

    return (unsigned char*)&header[16];
    
#endif
}
    
// Define the texture in OpenGL
// Note: Should load the texture from disk elsewhere
bool Texture::createInGL(OpenGLMemManager *memManager)
{
	if (!texData)
		return false;

    // We'll only create this once
	if (glId)
        return true;
	
	// Allocate a texture and set up the various params
    glId = memManager->getTexID();
    CheckGLError("Texture::createInGL() glGenTextures()");

	glBindTexture(GL_TEXTURE_2D, glId);
    CheckGLError("Texture::createInGL() glBindTexture()");
	
	// Set the texture parameters to use a minifying filter and a linear filter (weighted average)
    if (usesMipmaps)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpType);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpType);

    CheckGLError("Texture::createInGL() glTexParameteri()");
	
	// Configure textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (wrapU ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (wrapV ? GL_REPEAT : GL_CLAMP_TO_EDGE));

    CheckGLError("Texture::createInGL() glTexParameteri()");
    
    RawDataRef convertedData = processData();
	
	// If it's in an optimized form, we can use that more efficiently
	if (isPVRTC)
	{
	    // Will always be 4 bits per pixel and RGB
        // Note: Porting
//	    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, width, height, 0, (GLsizei)convertedData->getLen(), convertedData->getRawData());
//            CheckGLError("Texture::createInGL() glCompressedTexImage2D()");
    } else if (isPKM) {
            int compressedType,size;
            int thisWidth,thisHeight;
            unsigned char *rawData = ResolvePKM(texData,compressedType,size,thisWidth,thisHeight);
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, compressedType, width, height, 0, size, rawData);
            CheckGLError("Texture::createInGL() glCompressedTexImage2D()");
	} else {
         // Depending on the format, we may need to mess around with the bytes
         switch (format)
         {
            case GL_UNSIGNED_BYTE:
            default:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedData->getRawData());
                break;
            case GL_UNSIGNED_SHORT_5_6_5:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, convertedData->getRawData());
                break;
            case GL_UNSIGNED_SHORT_4_4_4_4:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, convertedData->getRawData());
                break;
            case GL_UNSIGNED_SHORT_5_5_5_1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, convertedData->getRawData());
                break;
            case GL_ALPHA:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, convertedData->getRawData());
                break;
                 // Note: Porting
//            case GL_COMPRESSED_RGB8_ETC2:
//                glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, width, height, 0, (GLsizei)convertedData->getLen(), convertedData->getRawData());
//                break;
         }
         CheckGLError("Texture::createInGL() glTexImage2D()");
 	}	
    
    if (usesMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);
	
    // Once we've moved it over to OpenGL, let's get rid of this copy
    texData.reset();
	
	return true;
}

// Release the OpenGL texture
void Texture::destroyInGL(OpenGLMemManager *memManager)
{
	if (glId)
        memManager->removeTexID(glId);
}

void TextureWrapper::destroyInGL(OpenGLMemManager *memManager)
{
    if (glId)
        memManager->removeTexID(glId);
}
    
}
