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

#import "TextureGLES.h"
#import "SceneRendererGLES.h"
#import "WhirlyKitLog.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{
    
TextureGLES::TextureGLES(const std::string &name)
    : Texture(name), TextureBaseGLES(name), TextureBase(name)
{
}
    
TextureGLES::TextureGLES(const std::string &name,RawDataRef texData,bool isPVRTC)
    : Texture(name,texData,isPVRTC), TextureBaseGLES(name), TextureBase(name)
{
}
    
// Figure out the PKM data
unsigned char *TextureGLES::ResolvePKM(RawDataRef texData,int &pkmType,int &size,int &width,int &height)
{
    if (texData->getLen() < 16)
        return NULL;
    const unsigned char *header = (const unsigned char *)texData->getRawData();
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
bool TextureGLES::createInRenderer(const RenderSetupInfo *inSetupInfo)
{
    RenderSetupInfoGLES *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;
    
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
    CheckGLError("Texture::createInGL() glGenTextures()");
    
    glBindTexture(GL_TEXTURE_2D, glId);
    CheckGLError("Texture::createInGL() glBindTexture()");
    
    GLenum interTypeGL = (interpType == TexInterpNearest) ? GL_NEAREST : GL_LINEAR;
    // Set the texture parameters to use a minifying filter and a linear filter (weighted average)
    if (usesMipmaps)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interTypeGL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interTypeGL);
    
    CheckGLError("Texture::createInGL() glTexParameteri()");
    
    // Configure textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (wrapU ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (wrapV ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    
    CheckGLError("Texture::createInGL() glTexParameteri()");
    
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
            case TexTypeUnsignedByte:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             (convertedData ? convertedData->getRawData() : NULL));
                break;
            case TexTypeShort565:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
                             (convertedData ? convertedData->getRawData() : NULL));
                break;
            case TexTypeShort4444:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,
                             (convertedData ? convertedData->getRawData() : NULL));
                break;
            case TexTypeShort5551:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
                             (convertedData ? convertedData->getRawData() : NULL));
                break;
            case TexTypeSingleChannel:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE,
                             (convertedData ? convertedData->getRawData() : NULL));
                break;
            case TexTypeDoubleChannel:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE,
                             (convertedData ? convertedData->getRawData() : NULL));
                break;
            default:
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
        CheckGLError("Texture::createInGL() glTexImage2D()");
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
    RenderSetupInfoGLES *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;

    if (glId)
        setupInfo->memManager->removeTexID(glId);
}

}
