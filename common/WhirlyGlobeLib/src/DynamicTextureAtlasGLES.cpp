/*
 *  DynamicTextureAtlasGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/19.
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

#import "DynamicTextureAtlasGLES.h"
#import "MemManagerGLES.h"
#import "UtilsGLES.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
DynamicTextureGLES::DynamicTextureGLES(const std::string &name)
    : DynamicTexture(name), TextureBase(name), glType(0), compressed(false), TextureBaseGLES(name)
{
}

void DynamicTextureGLES::setup(int texSize,int cellSize,TextureType inType,bool clearTextures)
{
    DynamicTexture::setup(texSize,cellSize,inType,clearTextures);
    
    // Check for the formats we'll accept
    switch (inType)
    {
        case TexTypeShort565:
            format = GL_RGB;
            glType = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case TexTypeUnsignedByte:
            format = GL_RGBA;
            glType = GL_UNSIGNED_BYTE;
            break;
        case TexTypeShort4444:
            format = GL_RGBA;
            glType = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case TexTypeShort5551:
            format = GL_RGBA;
            glType = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        case TexTypeSingleChannel:
            format = GL_ALPHA;
            glType = GL_UNSIGNED_BYTE;
            break;
            // Note: Porting
    #if 0
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
            compressed = true;
            format = GL_RGBA;
            type = inFormat;
            break;
    #endif
//        case GL_COMPRESSED_RGB8_ETC2:
//            compressed = true;
//            format = GL_RGB;
//            type = inFormat;
//            break;
//        case GL_COMPRESSED_RGBA8_ETC2_EAC:
//            compressed = true;
//            format = GL_RGBA;
//            type = inFormat;
//            break;
//        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
//            compressed = true;
//            format = GL_RGBA;
//            type = inFormat;
//            break;
//        case GL_COMPRESSED_R11_EAC:
//            compressed = true;
//            format = GL_ALPHA;
//            type = inFormat;
//            break;
//        case GL_COMPRESSED_SIGNED_R11_EAC:
//            compressed = true;
//            format = GL_ALPHA;
//            type = inFormat;
//            break;
//        case GL_COMPRESSED_RG11_EAC:
//            compressed = true;
//            format = GL_ALPHA;
//            type = inFormat;
//            break;
//        case GL_COMPRESSED_SIGNED_RG11_EAC:
//            compressed = true;
//            format = GL_ALPHA;
//            type = inFormat;
//            break;
        default:
            return;
            break;
    }
}

// If set we'll try to clear the images when we're not using them.
static const bool ClearImages = false;

// Create the OpenGL texture, empty
bool DynamicTextureGLES::createInRenderer(const RenderSetupInfo *inSetupInfo)
{
    RenderSetupInfoGLES *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;
    
    // Already setup
    if (glId != 0)
        return true;
    
    glId = setupInfo->memManager->getTexID();
    if (!glId)
        return false;
    glBindTexture(GL_TEXTURE_2D, glId);
    CheckGLError("DynamicTexture::createInGL() glBindTexture()");
    
    GLenum interTypeGL = (interpType == TexInterpNearest) ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interTypeGL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interTypeGL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CheckGLError("DynamicTexture::createInGL() glTexParameteri()");
    
    if (compressed)
    {
        size_t size = 0;
        switch (glType)
        {
            case GL_COMPRESSED_RGB8_ETC2:
            case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            case GL_COMPRESSED_R11_EAC:
            case GL_COMPRESSED_SIGNED_R11_EAC:
                size = texSize * texSize / 2;
                break;
            case GL_COMPRESSED_RGBA8_ETC2_EAC:
            case GL_COMPRESSED_RG11_EAC:
            case GL_COMPRESSED_SIGNED_RG11_EAC:
                size = texSize * texSize;
                break;
        }
        
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, glType, texSize, texSize, 0, (GLsizei)size, NULL);
    } else {
        // Turn this on to provide glTexImage2D with empty memory so Instruments doesn't complain
        if (ClearImages)
        {
            size_t size = texSize*texSize*4;
            unsigned char *zeroMem = (unsigned char *)malloc(size);
            memset(zeroMem, 0, size);
            glTexImage2D(GL_TEXTURE_2D, 0, format, texSize, texSize, 0, format, glType, zeroMem);
            free(zeroMem);
        } else
            glTexImage2D(GL_TEXTURE_2D, 0, format, texSize, texSize, 0, format, glType, NULL);
    }
    CheckGLError("DynamicTexture::createInGL() glTexImage2D()");
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

void DynamicTextureGLES::destroyInRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene)
{
    RenderSetupInfoGLES *setupInfo = (RenderSetupInfoGLES *)inSetupInfo;

    if (glId)
        setupInfo->memManager->removeTexID(glId);
    glId = 0;
}

void DynamicTextureGLES::addTextureData(int startX,int startY,int width,int height,RawDataRef data)
{
    if (data)
    {
        //        if (startX+width > texSize || startY+height > texSize)
        //            NSLog(@"Pixels outside bounds in dynamic texture.");
        
        glBindTexture(GL_TEXTURE_2D, glId);
        CheckGLError("DynamicTexture::createInGL() glBindTexture()");
        if (compressed)
        {
            int pkmType;
            int size,thisWidth,thisHeight;
            unsigned char *pixData = TextureGLES::ResolvePKM(data,pkmType, size, thisWidth, thisHeight);
            if (!pixData || pkmType != type || thisWidth != width || thisHeight != height)
                wkLogLevel(Warn,"Compressed texture doesn't match atlas.");
            else
                glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, startX, startY, thisWidth, thisHeight, pkmType, (GLsizei)size, pixData);
        } else
            glTexSubImage2D(GL_TEXTURE_2D, 0, startX, startY, width, height, format, glType, data->getRawData());
        CheckGLError("DynamicTexture::addTexture() glTexSubImage2D()");
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void DynamicTextureGLES::clearTextureData(int startX,int startY,int width,int height,ChangeSet &changes,bool mainThreadMerge,unsigned char *emptyData)
{
    if (!clearTextures)
        return;
    
    glBindTexture(GL_TEXTURE_2D, glId);
    
    if (compressed)
    {
        // Note: Can't do this for PKM currently
        //        int pkmType;
        //        int size,thisWidth,thisHeight;
        //        unsigned char *pixData = Texture::ResolvePKM(data,pkmType, size, thisWidth, thisHeight);
        //        if (!pixData || pkmType != type || thisWidth != width || thisHeight != height)
        //            NSLog(@"Compressed texture doesn't match atlas.");
        //        else
        //            glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, startX, startY, thisWidth, thisHeight, pkmType, (GLsizei)size, pixData);
    } else {
        if (ClearImages)
        {
            if (mainThreadMerge) {
                RawDataRef clearData(new RawDataWrapper(emptyData,width*height*4,false));
                changes.push_back(new DynamicTextureAddRegion(getId(),
                                                              startX, startY, width, height,
                                                              clearData));
            } else
                glTexSubImage2D(GL_TEXTURE_2D, 0, startX, startY, width, height, format, glType, emptyData);
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}
    
}
