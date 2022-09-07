/*  Texture.h
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

#import "Platform.h"
#import "RawData.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "BasicDrawable.h"

namespace WhirlyKit
{

/** Base class for textures.  This is enough information to track it in the Scene, but little else.
*/
struct TextureBase : virtual public Identifiable
{
    /// Construct for comparison
    TextureBase() : Identifiable() { }
    TextureBase(SimpleIdentity thisId) : Identifiable(thisId) { }
    TextureBase(std::string name) : Identifiable(), name(std::move(name)) { }

    virtual ~TextureBase() = default;
    
    /// Render side only.  Don't call this.  Create the openGL version
    virtual bool createInRenderer(const RenderSetupInfo *setupInfo) = 0;

    /// Render side only.  Don't call this.  Destroy the openGL version
    virtual void destroyInRenderer(const RenderSetupInfo *setupInfo,Scene *scene) = 0;

    const std::string& getName() const { return name; }

protected:
    /// Used for debugging
    std::string name;
};
    
typedef std::shared_ptr<TextureBase> TextureBaseRef;
    
/// For single byte pixels, what's the source, R G B or A?
typedef enum WKSingleByteSource_t {
    WKSingleRed,
    WKSingleGreen,
    WKSingleBlue,
    WKSingleRGB,
    WKSingleAlpha
} WKSingleByteSource;

/// Texture formats we allow
typedef enum TextureType_t {
    TexTypeUnsignedByte,    // RGBA8Unorm
    TexTypeShort565,
    TexTypeShort4444,
    TexTypeShort5551,
    TexTypeSingleChannel,   // A8Unorm
    TexTypeDoubleChannel,   // ABGR8 => RG8Unorm
    TexTypeSingleFloat16,
    TexTypeSingleFloat32,
    TexTypeDoubleFloat16,
    TexTypeDoubleFloat32,
    TexTypeQuadFloat16,
    TexTypeQuadFloat32,
    TexTypeDepthFloat32,
    TexTypeSingleInt16,     // R16Sint
    TexTypeSingleUInt16,     // R16Uint
    TexTypeDoubleUInt16,    // RG16Unorm
    TexTypeSingleUInt32,
    TexTypeDoubleUInt32,
    TexTypeQuadUInt32
} TextureType;

/// Interpolation types for upscaling
typedef enum TextureInterpType_t {
    TexInterpNearest,
    TexInterpLinear
} TextureInterpType;

/** Your basic Texture representation.
    This is how you get an image sent over to the rendering engine.
    Set up one of these and add it.
    If you want to remove it, you need to use its Identifiable ID.
 */
struct Texture : virtual public TextureBase
{
    virtual ~Texture() = default;

    /// Set the format (before createInGL() is called)
    void setFormat(TextureType inFormat) { format = inFormat; }
    /// Return the format
    TextureType getFormat() const { return format; }

    /// Set the interpolation type used for min and mag
    void setInterpType(TextureInterpType inType) { interpType = inType; }
    TextureInterpType getInterpType() const { return interpType; }

    /// Set the raw data directly
    /// Texture takes possession of the bytes.  It will free them.
    virtual void setRawData(RawData *rawData, int width, int height, int depth, int channels);

    /// Set the raw data directly
    virtual void setRawData(RawDataRef rawData, int width, int height, int depth, int channels);

    /// Process the data for display based on the format.
    virtual RawDataRef processData();
    
    /// Set up from raw PKM (ETC2/EAC) data
    virtual void setPKMData(RawDataRef data);

    /// Set the texture width
    void setWidth(unsigned int newWidth) { width = newWidth; }
    /// Get the texture width
    int getWidth() const { return width; }
    /// Set the texture height
    void setHeight(unsigned int newHeight) { height = newHeight; }
    /// Get the texture height
    int getHeight() const { return height; }
    /// Set this to have a mipmap generated and used for minification
    void setUsesMipmaps(bool use) { usesMipmaps = use; }
    /// Set this to let the texture wrap in the appropriate directions
    void setWrap(bool inWrapU,bool inWrapV) { wrapU = inWrapU;  wrapV = inWrapV; }

    /// If we're converting to a single byte, set the source
    void setSingleByteSource(WKSingleByteSource source) { byteSource = source; }
    /// If set, this is a texture we're creating for output purposes
    void setIsEmptyTexture(bool inIsEmptyTexture) { isEmptyTexture = inIsEmptyTexture; }

protected:
    Texture() = default;
    Texture(RawDataRef texData, bool isPVRTC);
    Texture(RawDataRef texData, TextureType fmt, int width, int height, bool isPVRTC);

    /// Raw texture data
    RawDataRef texData;
    int rawDepth = 0;
    int rawChannels = 0;

    /// Need to know how we're going to load it
    bool isPVRTC = false;
    /// This one has a header
    bool isPKM = false;

    bool usesMipmaps = false;
    bool wrapU = false;
    bool wrapV = false;
    bool isEmptyTexture = false;

    /// If we're converting down to one byte, where do we get it?
    WKSingleByteSource byteSource = WKSingleRGB;
    /// If not PVRTC, the format we'll use for the texture
    TextureType format = TexTypeUnsignedByte;
    TextureInterpType interpType = TexInterpLinear;

    unsigned int width = 0;
    unsigned int height = 0;
};
    
typedef std::shared_ptr<Texture> TextureRef;

// Pull two 8 byte channels out of an RGBA image
extern RawDataRef ConvertRGBATo16(const RawDataRef &inData,int width,int height,bool pad);
extern RawDataRef ConvertRGBATo565(const RawDataRef &inData);
extern RawDataRef ConvertRGBATo4444(const RawDataRef &inData);
extern RawDataRef ConvertRGBATo5551(const RawDataRef &inData);
extern RawDataRef ConvertAToA(const RawDataRef &inData,int width,int height);
extern RawDataRef ConvertRGToRG(const RawDataRef &inData,int width,int height);
extern RawDataRef ConvertRGBATo8(const RawDataRef &inData,WKSingleByteSource source);

}
