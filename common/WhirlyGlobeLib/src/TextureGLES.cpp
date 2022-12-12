/*  TextureGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
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
 */

#import "TextureGLES.h"
#import "SceneRendererGLES.h"
#import "WhirlyKitLog.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{

TextureGLES::TextureGLES() :
    TextureBase(),      // virtual inheritance, we need to call the base constructors
    Texture(),
    TextureBaseGLES()
{
}

TextureGLES::TextureGLES(std::string name) :
    TextureBase(std::move(name)),
    Texture(),
    TextureBaseGLES()
{
}

TextureGLES::TextureGLES(std::string name, RawDataRef texData, bool isPVRTC) :
    TextureBase(std::move(name)),
    Texture(std::move(texData), isPVRTC),
    TextureBaseGLES()
{
}

TextureGLES::TextureGLES(std::string name, RawDataRef texData,
        TextureType fmt, int width, int height, bool isPVRTC) :
    TextureBase(std::move(name)),
    Texture(std::move(texData), fmt, width, height, isPVRTC),
    TextureBaseGLES()
{
}

static int bytesPerRow(TextureType tt, int width)
{
    switch (tt)
    {
        case TexTypeShort5551:
        case TexTypeShort4444:
        case TexTypeShort565:      return width * 4;
        case TexTypeSingleChannel:
        case TexTypeDoubleChannel:
        case TexTypeUnsignedByte:  return width * 1;
        default:                   return GL_NONE;
    }
}

static GLint mapInternalFormat(TextureType tt)
{
    switch (tt)
    {
        case TexTypeUnsignedByte:
        case TexTypeShort5551:
        case TexTypeShort4444:     return GL_RGBA;
        case TexTypeShort565:      return GL_RGB;
        case TexTypeSingleChannel: return GL_ALPHA;     // single channel source?
        case TexTypeDoubleChannel: return GL_RG8;
        default:                   return GL_NONE;
    }
}

static GLenum mapGLFormat(TextureType tt)
{
    switch (tt)
    {
        case TexTypeUnsignedByte:
        case TexTypeShort4444:
        case TexTypeShort5551:     return GL_RGBA;
        case TexTypeShort565:      return GL_RGB;
        case TexTypeSingleChannel: return GL_ALPHA;
        case TexTypeDoubleChannel: return GL_RG;
        default:                   return GL_NONE;
    }
}

static GLenum mapGLType(TextureType tt)
{
    switch (tt)
    {
        case TexTypeUnsignedByte:
        case TexTypeSingleChannel:
        case TexTypeDoubleChannel: return GL_UNSIGNED_BYTE;
        case TexTypeShort565:      return GL_UNSIGNED_SHORT_5_6_5;
        case TexTypeShort4444:     return GL_UNSIGNED_SHORT_4_4_4_4;
        case TexTypeShort5551:     return GL_UNSIGNED_SHORT_5_5_5_1;
        default:                   return GL_NONE;
    }
}

// Figure out the PKM data
unsigned char *TextureGLES::ResolvePKM(const RawDataRef &texData,int &pkmType,int &size,int &width,int &height)
{
    if (texData->getLen() < 16)
        return nullptr;
    const auto *header = (const unsigned char *)texData->getRawData();
    //    unsigned short *version = (unsigned short *)&header[4];
    const unsigned char *type = &header[7];
    
    // Verify the magic number
    if (strncmp((char *)header, "PKM ", 4) != 0)
        return nullptr;
    
    width = (header[8] << 8) | header[9];
    height = (header[10] << 8) | header[11];
    
    // Resolve the GL type
    int glType = -1;
    switch (*type)
    {

        case 0: break;   // ETC1 not supported
        case 1: size = width * height / 2; glType = GL_COMPRESSED_RGB8_ETC2;       break;
        case 2: break;  // Unused
        case 3: size = width * height;     glType = GL_COMPRESSED_RGBA8_ETC2_EAC;  break;
        case 4: size = width * height / 2; glType = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2; break;
        case 5: size = width * height / 2; glType = GL_COMPRESSED_R11_EAC;         break;
        case 6: size = width * height;     glType = GL_COMPRESSED_RG11_EAC;        break;
        case 7: size = width * height / 2; glType = GL_COMPRESSED_SIGNED_R11_EAC;  break;
        case 8: size = width * height;     glType = GL_COMPRESSED_SIGNED_RG11_EAC; break;
    }

    if (glType == -1)
    {
        return nullptr;
    }
    pkmType = glType;
    return (unsigned char*)&header[16];
}

// Define the texture in OpenGL
bool TextureGLES::createInRenderer(const RenderSetupInfo *inSetupInfo)
{
    auto *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;
    
    if (!texData && !isEmptyTexture)
        return false;
    
    // We'll only create this once
    if (glId)
        return true;
    
    // Allocate a texture and set up the various params
    if (setupInfo && setupInfo->memManager)
        glId = setupInfo->memManager->getTexID();
    else
        glGenTextures(1, &glId);
    CheckGLError("Texture::createInRenderer() glGenTextures()");
    
    glBindTexture(GL_TEXTURE_2D, glId);
    CheckGLError("Texture::createInRenderer() glBindTexture()");
    
    const GLenum interTypeGL = (interpType == TexInterpNearest) ? GL_NEAREST : GL_LINEAR;
    // Set the texture parameters to use a minifying filter and a linear filter (weighted average)
    if (usesMipmaps)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)interTypeGL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)interTypeGL);
    
    CheckGLError("Texture::createInRenderer() glTexParameteri()");
    
    // Configure textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (wrapU ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (wrapV ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    
    CheckGLError("Texture::createInRenderer() glTexParameteri()");
    
    RawDataRef convertedData = processData();
    
    // If it's in an optimized form, we can use that more efficiently
    if (isPVRTC)
    {
        // Note: Porting
#if 0
        // Will always be 4 bits per pixel and RGB
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, width, height, 0, (GLsizei)convertedData->getLen(), convertedData->getRawData());
        CheckGLError("Texture::createInGL() glCompressedTexImage2D()");
#endif
        wkLogLevel(Error, "PVRTC not supported");
    }
    else if (isPKM)
    {
        int compressedType,size;
        int thisWidth, thisHeight;
        if (unsigned char *rawData = ResolvePKM(texData,compressedType,size,thisWidth,thisHeight))
        {
            glCompressedTexImage2D(GL_TEXTURE_2D, /*level=*/0, compressedType,
                                   (GLsizei) width, (GLsizei) height, /*border=*/0,
                                   size, rawData);
            CheckGLError("Texture::createInRenderer() glCompressedTexImage2D()");
        }
        else
        {
            wkLogLevel(Error, "Failed to resolve PKM");
        }
    }
    else
    {
        // Depending on the format, we may need to mess around with the bytes
        const GLint internalFormat = mapInternalFormat(format);
        const GLenum glFormat = mapGLFormat(format);
        const GLenum glType = mapGLType(format);
        if (internalFormat != GL_NONE && glFormat != GL_NONE && glType != GL_NONE)
        {
            wkLog("glTexImage2D %dx%d %ld", width, height, convertedData ? convertedData->getLen() : 0);
            const auto *data = convertedData ? convertedData->getRawData() : nullptr;
            glTexImage2D(GL_TEXTURE_2D, /*level=*/0, internalFormat,
                         (GLsizei)width, (GLsizei)height, /*border=*/0,
                         glFormat, glType, data);
            CheckGLError("Texture::createInRenderer() glTexImage2D()");
        }
        else
        {
            wkLogLevel(Error, "Unknown texture type %d for GLES",(int)format);
            // Note: Porting
//            case GL_COMPRESSED_RGB8_ETC2:
//                glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, width, height, 0,
//                                       (GLsizei)(convertedData ? convertedData->getLen() : 0),
//                                       (convertedData ? convertedData->getRawData() : NULL));
//                break;
//            case GL_DEPTH_COMPONENT16:
//                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,
//                             (convertedData ? convertedData->getRawData() : NULL));
//                break;
        }
    }
    
    if (usesMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);
    
    // Once we've moved it over to OpenGL, let's get rid of this copy
    texData.reset();
    
    return true;
}

// Release the OpenGL texture
void TextureGLES::destroyInRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene)
{
    const auto *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;
    if (glId && setupInfo)
    {
        setupInfo->memManager->removeTexID(glId);
    }
}

}
