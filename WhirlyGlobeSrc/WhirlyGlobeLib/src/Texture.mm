/*
 *  Texture.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
 *  Copyright 2011-2015 mousebird consulting
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
#import "UIImage+Stuff.h"

using namespace WhirlyKit;

// Convert a buffer in RGBA to 2-byte 565
// Code courtesy: http://stackoverflow.com/questions/7930148/opengl-es-on-ios-texture-loading-how-do-i-get-from-a-rgba8888-png-file-to-a-r
NSData *ConvertRGBATo565(NSData *inData)
{
    uint32_t pixelCount = (uint32_t)[inData length]/4;
    void *temp = malloc(pixelCount * 2);
    uint32_t *inPixel32  = (uint32_t *)[inData bytes];
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 3);
        uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 2);
        uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 3);
        
        *outPixel16++ = (r << 11) | (g << 5) | (b << 0);
    }
    
    return [NSData dataWithBytesNoCopy:temp length:pixelCount*2 freeWhenDone:YES];
}


// Convert a buffer in RGBA to 2-byte 4444
NSData *ConvertRGBATo4444(NSData *inData)
{
    uint32_t pixelCount = (uint32_t)[inData length]/4;
    void *temp = malloc(pixelCount * 2);
    uint32_t *inPixel32  = (uint32_t *)[inData bytes];
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 4);
        uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 4);
        uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 4);
        uint32_t a = (((*inPixel32 >> 24) & 0xFF) >> 4);
        
        *outPixel16++ = (r << 12) | (g << 8) | (b << 4) | (a<< 0);
    }
    
    return [NSData dataWithBytesNoCopy:temp length:pixelCount*2 freeWhenDone:YES];
}

// Convert a buffer in RGBA to 2-byte 5551
NSData *ConvertRGBATo5551(NSData *inData)
{
    uint32_t pixelCount = (uint32_t)[inData length]/4;
    void *temp = malloc(pixelCount * 2);
    uint32_t *inPixel32  = (uint32_t *)[inData bytes];
    uint16_t *outPixel16 = (uint16_t *)temp;
    
    for(uint32_t i=0; i<pixelCount; i++, inPixel32++)
    {
        uint32_t r = (((*inPixel32 >> 0)  & 0xFF) >> 3);
        uint32_t g = (((*inPixel32 >> 8)  & 0xFF) >> 3);
        uint32_t b = (((*inPixel32 >> 16) & 0xFF) >> 3);
        uint32_t a = (((*inPixel32 >> 24) & 0xFF) >> 7);
        
        *outPixel16++ = (r << 11) | (g << 6) | (b << 1) | (a << 0);
    }
    
    return [NSData dataWithBytesNoCopy:temp length:pixelCount*2 freeWhenDone:YES];
}

// Convert a buffer in RGBA to 1-byte alpha
NSData *ConvertRGBATo8(NSData *inData,WKSingleByteSource source)
{
    uint32_t pixelCount = (uint32_t)[inData length]/4;
    void *temp = malloc(pixelCount);
    uint32_t *inPixel32  = (uint32_t *)[inData bytes];
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
    
    return [NSData dataWithBytesNoCopy:temp length:pixelCount freeWhenDone:YES];
}

namespace WhirlyKit
{
	
Texture::Texture(const std::string &name)
	: TextureBase(name), texData(NULL), isPVRTC(false), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(GL_UNSIGNED_BYTE), byteSource(WKSingleRGB), interpType(GL_LINEAR)
{
}
	
// Construct with raw texture data
Texture::Texture(const std::string &name,NSData *texData,bool isPVRTC)
	: TextureBase(name), texData(texData), isPVRTC(isPVRTC), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(GL_UNSIGNED_BYTE), byteSource(WKSingleRGB), interpType(GL_LINEAR)
{ 
}

// Set up the texture from a filename
Texture::Texture(const std::string &name,NSString *baseName,NSString *ext)
    : TextureBase(name), texData(nil), isPVRTC(false), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(GL_UNSIGNED_BYTE), byteSource(WKSingleRGB), interpType(GL_LINEAR)
{	
	if (![ext compare:@"pvrtc"])
	{
		isPVRTC = true;

		// Look for an absolute version or one from the bundle
		// Only for pvrtc, though		
		NSString* path = [NSString stringWithFormat:@"%@.%@",baseName,ext];
		
		if (![[NSFileManager defaultManager] fileExistsAtPath:path])
			path = [[NSBundle mainBundle] pathForResource:baseName ofType:ext];

		if (!path)
			return;
		texData = [[NSData alloc] initWithContentsOfFile:path];
		if (!texData)
			return;
	} else {
		// Otherwise load it the normal way
		UIImage *image = [UIImage imageNamed:[NSString stringWithFormat:@"%@.%@",baseName,ext]];
		if (!image)
        {
            image = [[UIImage alloc] initWithContentsOfFile:[NSString stringWithFormat:@"%@.%@",baseName,ext]];
            if (!image)
                return;
        }
		texData = [image rawDataRetWidth:&width height:&height roundUp:true];
	}
}

// Construct with a UIImage
Texture::Texture(const std::string &name,UIImage *inImage,bool roundUp)
    : TextureBase(name), texData(nil), isPVRTC(false), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(GL_UNSIGNED_BYTE), byteSource(WKSingleRGB), interpType(GL_LINEAR)
{
	texData = [inImage rawDataRetWidth:&width height:&height roundUp:roundUp];
}

Texture::Texture(const std::string &name,UIImage *inImage,int inWidth,int inHeight)
    : TextureBase(name), texData(nil), isPVRTC(false), isPKM(false), usesMipmaps(false), wrapU(false), wrapV(false), format(GL_UNSIGNED_BYTE), byteSource(WKSingleRGB), interpType(GL_LINEAR)
{
    texData = [inImage rawDataScaleWidth:inWidth height:inHeight border:0];
    width = inWidth;  height = inHeight;
}

Texture::~Texture()
{
	texData = nil;
}

NSData *Texture::processData()
{
	if (isPVRTC || isPKM)
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
                return ConvertRGBATo565(texData);
                break;
            case GL_UNSIGNED_SHORT_4_4_4_4:
                return ConvertRGBATo4444(texData);
                break;
            case GL_UNSIGNED_SHORT_5_5_5_1:
                return ConvertRGBATo5551(texData);
                break;
            case GL_ALPHA:
                return ConvertRGBATo8(texData,byteSource);
                break;
            case GL_COMPRESSED_RGB8_ETC2:
                // Can't convert this (for now)
                return texData;
                break;
        }
	}
    
    return nil;
}
    
void Texture::setPKMData(NSData *inData)
{
    texData = inData;
    isPKM = true;
}

// Figure out the PKM data
unsigned char *Texture::ResolvePKM(NSData *texData,int &pkmType,int &size,int &width,int &height)
{
    if ([texData length] < 16)
        return NULL;
    const unsigned char *header = (const unsigned char *)[texData bytes];
//    unsigned short *version = (unsigned short *)&header[4];
    const unsigned char *type = &header[7];
    
    // Verify the magic number
    if (strncmp((char *)header, "PKM ", 4))
        return NULL;
    
    width = (header[8] << 8) | header[9];
    height = (header[10] << 8) | header[11];;
    
    // Resolve the GL type
    int glType = -1;
    switch (*type)
    {
        case 0:
            // ETC1 not supported
            break;
        case 1:
            glType = GL_COMPRESSED_RGB8_ETC2;
            size = width * height / 2;
            break;
        case 2:
            // Unused
            break;
        case 3:
            glType = GL_COMPRESSED_RGBA8_ETC2_EAC;
            size = width * height;
            break;
        case 4:
            glType = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
            size = width * height / 2;
            break;
        case 5:
            glType = GL_COMPRESSED_R11_EAC;
            size = width * height / 2;
            break;
        case 6:
            glType = GL_COMPRESSED_RG11_EAC;
            size = width * height;
            break;
        case 7:
            glType = GL_COMPRESSED_SIGNED_R11_EAC;
            size = width * height / 2;
            break;
        case 8:
            glType = GL_COMPRESSED_SIGNED_RG11_EAC;
            size = width * height;
            break;
    }
    if (glType == -1)
        return NULL;
    pkmType = glType;

    return (unsigned char*)&header[16];
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
    
    NSData *convertedData = processData();
	
	// If it's in an optimized form, we can use that more efficiently
	if (isPVRTC)
	{
		// Will always be 4 bits per pixel and RGB
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, width, height, 0, (GLsizei)[convertedData length], [convertedData bytes]);
        CheckGLError("Texture::createInGL() glCompressedTexImage2D()");
	} else if (isPKM)
    {
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
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, [convertedData bytes]);
                break;
            case GL_UNSIGNED_SHORT_5_6_5:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, [convertedData bytes]);
                break;
            case GL_UNSIGNED_SHORT_4_4_4_4:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, [convertedData bytes]);
                break;
            case GL_UNSIGNED_SHORT_5_5_5_1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, [convertedData bytes]);
                break;
            case GL_ALPHA:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, [convertedData bytes]);
                break;
            case GL_COMPRESSED_RGB8_ETC2:
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, width, height, 0, (GLsizei)[convertedData length], [convertedData bytes]);
                break;
        }
        CheckGLError("Texture::createInGL() glTexImage2D()");
	}	
    
    if (usesMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);
	
    // Once we've moved it over to OpenGL, let's get rid of this copy
    texData = nil;
	
	return true;
}

// Release the OpenGL texture
void Texture::destroyInGL(OpenGLMemManager *memManager)
{
	if (glId)
        memManager->removeTexID(glId);
}

}
