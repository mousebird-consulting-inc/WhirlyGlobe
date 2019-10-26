/*
 *  Texture.h
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

#import "Platform.h"
#import "RawData.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "BasicDrawable.h"

namespace WhirlyKit
{
    
/** Base class for textures.  This is enough information to
    track it in the Scene, but little else.
  */
class TextureBase : virtual public Identifiable
{
public:
    /// Construct for comparison
    TextureBase(SimpleIdentity thisId);
    TextureBase(const std::string &name);
    
    virtual ~TextureBase();
    
    /// Render side only.  Don't call this.  Create the openGL version
    virtual bool createInRenderer(const RenderSetupInfo *setupInfo) = 0;
	
	/// Render side only.  Don't call this.  Destroy the openGL version
    virtual void destroyInRenderer(const RenderSetupInfo *setupInfo,Scene *scene) = 0;

protected:
    /// Used for debugging
    std::string name;
};
    
typedef std::shared_ptr<TextureBase> TextureBaseRef;
    
/// For single byte pixels, what's the source, R G B or A?
typedef enum {WKSingleRed,WKSingleGreen,WKSingleBlue,WKSingleRGB,WKSingleAlpha} WKSingleByteSource;

/// Texture formats we allow
typedef enum {TexTypeUnsignedByte,TexTypeShort565,TexTypeShort4444,TexTypeShort5551,TexTypeSingleChannel,TexTypeDoubleChannel,TexTypeSingleFloat16,TexTypeSingleFloat32,TexTypeDoubleFloat16,TexTypeDoubleFloat32,TexTypeQuadFloat16,TexTypeQuadFloat32,TexTypeDepthFloat32} TextureType;
/// Interpolation types for upscaling
typedef enum {TexInterpNearest,TexInterpLinear} TextureInterpType;
    
/** Your basic Texture representation.
    This is how you get an image sent over to the
    rendering engine.  Set up one of these and add it.
    If you want to remove it, you need to use its
    Identifiable ID.
 */
class Texture : virtual public TextureBase
{
public:
    /// Construct empty
	Texture(const std::string &name);
	/// Construct with raw texture data.  PVRTC is preferred.
	Texture(const std::string &name,RawDataRef texData,bool isPVRTC);
	
	virtual ~Texture();
    
    /// Set the format (before createInGL() is called)
    void setFormat(TextureType inFormat) { format = inFormat; }
    /// Return the format
    TextureType getFormat() { return format; }

    /// Set the interpolation type used for min and mag
    void setInterpType(TextureInterpType inType) { interpType = inType; }
    TextureInterpType getInterpType() { return interpType; }

    /// Set the raw data directly
    /// Texture takes possession of the bytes.  It will free them.
    void setRawData(RawData *rawData,int width,int height);
	    
    /// Process the data for display based on the format.
    RawDataRef processData();
    
    /// Set up from raw PKM (ETC2/EAC) data
    void setPKMData(RawDataRef data);
	
    /// Set the texture width
    void setWidth(unsigned int newWidth) { width = newWidth; }
    /// Get the texture width
    int getWidth() { return width; }
    /// Set the texture height
    void setHeight(unsigned int newHeight) { height = newHeight; }
    /// Get the texture height
    int getHeight() { return height; }
    /// Set this to have a mipmap generated and used for minification
    void setUsesMipmaps(bool use) { usesMipmaps = use; }
    /// Set this to let the texture wrap in the appropriate directions
    void setWrap(bool inWrapU,bool inWrapV) { wrapU = inWrapU;  wrapV = inWrapV; }
    
    /// If we're converting to a single byte, set the source
    void setSingleByteSource(WKSingleByteSource source) { byteSource = source; }
    /// If set, this is a texture we're creating for output purposes
    void setIsEmptyTexture(bool inIsEmptyTexture) { isEmptyTexture = inIsEmptyTexture; }

    /// Raw texture data
    RawDataRef texData;

protected:
    /// Used by subclass
    Texture();

    /// Need to know how we're going to load it
	bool isPVRTC;
    /// This one has a header
    bool isPKM;
    /// If we're converting down to one byte, where do we get it?
    WKSingleByteSource byteSource;
	
    /// If not PVRTC, the format we'll use for the texture
    TextureType format;
    TextureInterpType interpType;

	unsigned int width,height;
    bool usesMipmaps;
    bool wrapU,wrapV;
    bool isEmptyTexture;
};
    
typedef std::shared_ptr<Texture> TextureRef;

// Pull two 8 byte channels out of an RGBA image
extern RawDataRef ConvertRGBATo16(RawDataRef inData,int width,int height,bool pad);
    
}
