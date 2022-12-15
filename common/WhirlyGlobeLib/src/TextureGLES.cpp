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

#if !defined(STRINGIFY) && !defined(_STRINGIFY)
# define STRINGIFY_(x) #x
# define STRINGIFY(x) STRINGIFY_(x)
#endif
#define STRPAIR(X) { X, STRINGIFY(X) }
#define STRS(X,Y) { X, Y }

static const std::unordered_map<TextureType, const char* const> texTypeStrMap =   // NOLINT(cert-err58-cpp)
{
    STRPAIR(TexTypeUnsignedByte),
    STRPAIR(TexTypeShort565),
    STRPAIR(TexTypeShort4444),
    STRPAIR(TexTypeShort5551),
    STRPAIR(TexTypeSingleChannel),
    STRPAIR(TexTypeDoubleChannel),
    STRPAIR(TexTypeSingleFloat16),
    STRPAIR(TexTypeSingleFloat32),
    STRPAIR(TexTypeDoubleFloat16),
    STRPAIR(TexTypeDoubleFloat32),
    STRPAIR(TexTypeQuadFloat16),
    STRPAIR(TexTypeQuadFloat32),
    STRPAIR(TexTypeDepthFloat32),
    STRPAIR(TexTypeSingleInt16),
    STRPAIR(TexTypeSingleUInt16),
    STRPAIR(TexTypeDoubleUInt16),
    STRPAIR(TexTypeSingleUInt32),
    STRPAIR(TexTypeDoubleUInt32),
    STRPAIR(TexTypeQuadUInt32),
};
static const std::unordered_map<WKSingleByteSource, const char* const> bsStrMap =    // NOLINT(cert-err58-cpp)
{
    STRPAIR(WKSingleRed),
    STRPAIR(WKSingleGreen),
    STRPAIR(WKSingleBlue),
    STRPAIR(WKSingleRGB),
    STRPAIR(WKSingleAlpha),
};
static const std::unordered_map<int, const char* const> glStrMap =    // NOLINT(cert-err58-cpp)
{
    STRS(GL_ALPHA,                                    "GL_ALPHA"),
    STRS(GL_BYTE,                                     "GL_BYTE"),
    STRS(GL_COMPRESSED_R11_EAC,                       "GL_COMPRESSED_R11_EAC"),
    STRS(GL_COMPRESSED_RG11_EAC,                      "GL_COMPRESSED_RG11_EAC"),
    STRS(GL_COMPRESSED_RGB8_ETC2,                     "GL_COMPRESSED_RGB8_ETC2"),
    STRS(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2"),
    STRS(GL_COMPRESSED_RGBA8_ETC2_EAC,                "GL_COMPRESSED_RGBA8_ETC2_EAC"),
    STRS(GL_COMPRESSED_SIGNED_R11_EAC,                "GL_COMPRESSED_SIGNED_R11_EAC"),
    STRS(GL_COMPRESSED_SIGNED_RG11_EAC,               "GL_COMPRESSED_SIGNED_RG11_EAC"),
    STRS(GL_DEPTH_COMPONENT,                          "GL_DEPTH_COMPONENT"),
    STRS(GL_DEPTH_COMPONENT32F,                       "GL_DEPTH_COMPONENT32F"),
    STRS(GL_DEPTH_STENCIL,                            "GL_DEPTH_STENCIL"),
    STRS(GL_FLOAT,                                    "GL_FLOAT"),
    STRS(GL_FLOAT_32_UNSIGNED_INT_24_8_REV,           "GL_FLOAT_32_UNSIGNED_INT_24_8_REV"),
    STRS(GL_HALF_FLOAT,                               "GL_HALF_FLOAT"),
    STRS(GL_INT,                                      "GL_INT"),
    STRS(GL_LUMINANCE,                                "GL_LUMINANCE"),
    STRS(GL_LUMINANCE_ALPHA,                          "GL_LUMINANCE_ALPHA"),
    STRS(GL_NONE,                                     "GL_NONE"),
    STRS(GL_R16F,                                     "GL_R16F"),
    STRS(GL_R16I,                                     "GL_R16I"),
    STRS(GL_R16UI,                                    "GL_R16UI"),
    STRS(GL_R32F,                                     "GL_R32F"),
    STRS(GL_R32UI,                                    "GL_R32UI"),
    STRS(GL_R8,                                       "GL_R8"),
    STRS(GL_RED,                                      "GL_RED"),
    STRS(GL_RED_INTEGER,                              "GL_RED_INTEGER"),
    STRS(GL_RG,                                       "GL_RG"),
    STRS(GL_RG16F,                                    "GL_RG16F"),
    STRS(GL_RG16UI,                                   "GL_RG16UI"),
    STRS(GL_RG32F,                                    "GL_RG32F"),
    STRS(GL_RG32UI,                                   "GL_RG32UI"),
    STRS(GL_RG8,                                      "GL_RG8"),
    STRS(GL_RGB,                                      "GL_RGB"),
    STRS(GL_RGB565,                                   "GL_RGB565"),
    STRS(GL_RGB5_A1,                                  "GL_RGB5_A1"),
    STRS(GL_RGBA,                                     "GL_RGBA"),
    STRS(GL_RGBA32F,                                  "GL_RGBA32F"),
    STRS(GL_RGBA32UI,                                 "GL_RGBA32UI"),
    STRS(GL_RGBA8,                                    "GL_RGBA8"),
    STRS(GL_RGBA_INTEGER,                             "GL_RGBA_INTEGER"),
    STRS(GL_RGB_INTEGER,                              "GL_RGB_INTEGER"),
    STRS(GL_RG_INTEGER,                               "GL_RG_INTEGER"),
    STRS(GL_SHORT,                                    "GL_SHORT"),
    STRS(GL_UNSIGNED_BYTE,                            "GL_UNSIGNED_BYTE"),
    STRS(GL_UNSIGNED_INT,                             "GL_UNSIGNED_INT"),
    STRS(GL_UNSIGNED_INT_10F_11F_11F_REV,             "GL_UNSIGNED_INT_10F_11F_11F_REV"),
    STRS(GL_UNSIGNED_INT_24_8,                        "GL_UNSIGNED_INT_24_8"),
    STRS(GL_UNSIGNED_INT_2_10_10_10_REV,              "GL_UNSIGNED_INT_2_10_10_10_REV"),
    STRS(GL_UNSIGNED_INT_5_9_9_9_REV,                 "GL_UNSIGNED_INT_5_9_9_9_REV"),
    STRS(GL_UNSIGNED_SHORT,                           "GL_UNSIGNED_SHORT"),
    STRS(GL_UNSIGNED_SHORT_4_4_4_4,                   "GL_UNSIGNED_SHORT_4_4_4_4"),
    STRS(GL_UNSIGNED_SHORT_5_5_5_1,                   "GL_UNSIGNED_SHORT_5_5_5_1"),
    STRS(GL_UNSIGNED_SHORT_5_6_5,                     "GL_UNSIGNED_SHORT_5_6_5"),
};
static const std::unordered_map<TextureInterpType, const char* const> interpStrMap =    // NOLINT(cert-err58-cpp)
{
    STRPAIR(TexInterpNearest),
    STRPAIR(TexInterpLinear),
};
template <typename T>
__attribute__((unused)) static inline const char* mapStr(T tt, const std::unordered_map<T,const char* const> &m) {
    const auto it = m.find(tt);
    return (it == m.end()) ? "(unknown)" : it->second;
}
__attribute__((unused)) static inline const char* texStr(TextureType tt) { return mapStr(tt, texTypeStrMap); }
__attribute__((unused)) static inline const char* bsStr(WKSingleByteSource tt) { return mapStr(tt, bsStrMap); }
__attribute__((unused)) static inline const char* glStr(int tt) { return mapStr(tt, glStrMap); }
__attribute__((unused)) static inline const char* interpStr(TextureInterpType tt) { return mapStr(tt, interpStrMap); }


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

static unsigned getBytesPerRow(TextureType tt, unsigned width)
{
    switch (tt)
    {
        case TexTypeSingleChannel: return width * 1;
        case TexTypeDoubleChannel:
        case TexTypeSingleFloat16:
        case TexTypeShort5551:
        case TexTypeShort4444:
        case TexTypeShort565:
        case TexTypeSingleInt16:
        case TexTypeSingleUInt16:  return width * 2;
        case TexTypeDoubleUInt16:
        case TexTypeDoubleFloat16:
        case TexTypeSingleFloat32:
        case TexTypeDepthFloat32:
        case TexTypeUnsignedByte:  return width * 4;
        case TexTypeSingleUInt32:
        case TexTypeDoubleFloat32:
        case TexTypeDoubleUInt32:
        case TexTypeQuadFloat16:   return width * 8;
        case TexTypeQuadFloat32:
        case TexTypeQuadUInt32:    return width * 16;
        default:                   return GL_NONE;
    }
}

static bool isFilterable(GLenum internalFormat)
{
    // https://docs.gl/es3/glTexImage2D#idp103216
    switch (internalFormat)
    {
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16F:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16F:
        case GL_RGB8:
        case GL_SRGB8:
        case GL_RGB565:
        case GL_RGB8_SNORM:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RGB16F:
        case GL_RGBA8:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA8_SNORM:
        case GL_RGB5_A1:
        case GL_RGBA4:
        case GL_RGB10_A2:
        case GL_RGBA16F: return true;
        default:         return false;
    }
}

// "Specifies the number of color components in the texture. Must be one of base internal formats
//  given in Table 1, or one of the sized internal formats given in Table 2, below."
// Table 1: GL_RGB, GL_RGBA, GL_LUMINANCE_ALPHA, GL_LUMINANCE, GL_ALPHA
// Table 2: GL_R8, GL_R8_SNORM, GL_R16F, GL_R32F, ... (truncated)
// https://docs.gl/es3/glTexImage2D
static GLint mapInternalFormat(TextureType tt, WKSingleByteSource byteSource)
{
    switch (tt)
    {
        case TexTypeUnsignedByte:  return GL_RGBA8;     // type = GL_UNSIGNED_BYTE
        case TexTypeShort5551:     return GL_RGB5_A1;   // type = GL_UNSIGNED_SHORT_5_5_5_1
        case TexTypeShort4444:     return GL_RGBA;      // type = GL_UNSIGNED_SHORT_4_4_4_4
        case TexTypeShort565:      return GL_RGB565;    // type = GL_UNSIGNED_SHORT_5_6_5
        case TexTypeSingleChannel:
            switch (byteSource)
            {
                case WKSingleAlpha: return GL_ALPHA;
                case WKSingleRed:   return GL_R8;
                case WKSingleGreen:                     // not supported?
                case WKSingleBlue:
                case WKSingleRGB:                       // 3/3/2?
                default:            return GL_NONE;
            }
        case TexTypeDoubleChannel:  return GL_RG8;
        case TexTypeSingleFloat16:  return GL_R16F;
        case TexTypeDoubleFloat16:  return GL_RG16F;
        case TexTypeQuadFloat16:    return GL_HALF_FLOAT;
        case TexTypeSingleFloat32:  return GL_R32F;
        case TexTypeDoubleFloat32:  return GL_RG32F;
        case TexTypeDepthFloat32:   return GL_DEPTH_COMPONENT32F;
        case TexTypeQuadFloat32:    return GL_RGBA32F;
        case TexTypeSingleInt16:    return GL_R16I;                 // format=GL_RED_INTEGER type=GL_SHORT R=i16 colorRenderable=Y
        case TexTypeSingleUInt16:   return GL_R16UI;                // format=GL_RED_INTEGER type=GL_UNSIGNED_SHORT R=ui16 colorRenderable=Y
        case TexTypeDoubleUInt16:   return GL_RG16UI;
        case TexTypeSingleUInt32:   return GL_R32UI;
        case TexTypeDoubleUInt32:   return GL_RG32UI;
        case TexTypeQuadUInt32:     return GL_RGBA32UI;
        default:                    return GL_NONE;
    }
}

// "Specifies the format of the pixel data. The following symbolic values are accepted:
//  GL_RED, GL_RED_INTEGER, GL_RG, GL_RG_INTEGER, GL_RGB, GL_RGB_INTEGER, GL_RGBA, GL_RGBA_INTEGER,
//  GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL, GL_LUMINANCE_ALPHA, GL_LUMINANCE, and GL_ALPHA."
static GLenum mapGLFormat(TextureType tt, WKSingleByteSource byteSource)
{
    switch (tt)
    {
        case TexTypeSingleChannel:
            switch (byteSource)
            {
                case WKSingleAlpha: return GL_ALPHA;
                case WKSingleRed:   return GL_RED;
                case WKSingleGreen:
                case WKSingleBlue:
                case WKSingleRGB:
                default:            return GL_NONE;
            }
        case TexTypeSingleFloat16:
        case TexTypeSingleFloat32: return GL_RED;
        case TexTypeSingleInt16:
        case TexTypeSingleUInt16:
        case TexTypeSingleUInt32:  return GL_RED_INTEGER;
        case TexTypeDoubleFloat16:
        case TexTypeDoubleFloat32: return GL_RG;
        case TexTypeDoubleChannel:
        case TexTypeDoubleUInt16:
        case TexTypeDoubleUInt32:  return GL_RG_INTEGER;
        case TexTypeShort565:      return GL_RGB;
        case TexTypeUnsignedByte:
        case TexTypeShort4444:
        case TexTypeShort5551:
        case TexTypeQuadFloat16:
        case TexTypeQuadFloat32:   return GL_RGBA;
        case TexTypeQuadUInt32:    return GL_RGBA_INTEGER;
        case TexTypeDepthFloat32:  return GL_DEPTH_COMPONENT;
        default:                   return GL_NONE;
    }
}

// "Specifies the data type of the pixel data. The following symbolic values are accepted:
//  GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_HALF_FLOAT,
//  GL_FLOAT, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_5_5_5_1,
//  GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV,
//  GL_UNSIGNED_INT_24_8, and GL_FLOAT_32_UNSIGNED_INT_24_8_REV."
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
        case TexTypeSingleFloat16:
        case TexTypeDoubleFloat16:
        case TexTypeQuadFloat16:   return GL_HALF_FLOAT;
        case TexTypeSingleFloat32:
        case TexTypeDoubleFloat32:
        case TexTypeQuadFloat32:
        case TexTypeDepthFloat32:  return GL_FLOAT;
        case TexTypeSingleInt16:   return GL_SHORT;
        case TexTypeSingleUInt16:
        case TexTypeDoubleUInt16:  return GL_UNSIGNED_SHORT;
        case TexTypeSingleUInt32:
        case TexTypeDoubleUInt32:
        case TexTypeQuadUInt32:    return GL_UNSIGNED_INT;
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

    if (texData && !convertedData)
    {
        wkLogLevel(Error, "TextureGLES::processData Failed");
        return false;
    }

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
        const GLint internalFormat = mapInternalFormat(format, byteSource);
        const GLenum glFormat = mapGLFormat(format, byteSource);
        const GLenum glType = mapGLType(format);
        const unsigned bytesPerRow = getBytesPerRow(format, width);
        if (internalFormat != GL_NONE && glFormat != GL_NONE && glType != GL_NONE)
        {
            if (!isFilterable(internalFormat) && interpType != TexInterpNearest)
            {
                wkLogLevel(Warn, "Texture format %s (%s) doesn't support %s",
                           texStr(format), glStr(internalFormat), interpStr(interpType));
            }

            if (convertedData && convertedData->getLen() != bytesPerRow * height)
            {
                wkLogLevel(Warn, "Texture %lld/%d data size mismatch fmt=%s=>%s w=%d h=%d expected=%d actual=%d",
                           getId(), getGLId(), texStr(format), glStr(internalFormat),
                           width, height, bytesPerRow * height, convertedData->getLen());

                // If we have too few bytes, the `glTexImage2D` is likely to crash by walking off
                // the end of the allocated memory block.
                // Otherwise, we'll probably get an incorrect texture, but let it succeed to make
                // the problem more obvious.
                if (convertedData->getLen() < bytesPerRow * height)
                {
                    return false;
                }
            }
            const auto *data = convertedData ? convertedData->getRawData() : nullptr;
            //if (data)
            //{
            //    wkLog("Loading texture id=%d from fmt=%s(%s) w=%d h=%d => if=%s f=%s t=%s b=%d raw=%.8x %.8x ...",
            //          glId, texStr(format), bsStr(byteSource), width, height,
            //          glStr(internalFormat), glStr(glFormat), glStr(glType), bytesPerRow,
            //          ((int*)data)[0], ((int*)data)[1]);
            //}
            glTexImage2D(GL_TEXTURE_2D, /*level=*/0, internalFormat,
                         (GLsizei)width, (GLsizei)height, /*border=*/0,
                         glFormat, glType, data);
            CheckGLError("Texture::createInRenderer() glTexImage2D()");
        }
        else
        {
            wkLogLevel(Error, "Unknown texture type %d for GLES", (int)format);
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
