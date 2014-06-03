/*
 *  Texture.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
 *  Copyright 2011-2013 mousebird consulting
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

#import "glwrapper.h"
#import "Platform.h"
#import "RawData.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Drawable.h"

namespace WhirlyKit
{
    
/** Base class for textures.  This is enough information to
    track it in the Scene, but little else.
  */
class TextureBase : public Identifiable
{
public:
    /// Construct for comparison
    TextureBase(SimpleIdentity thisId) : Identifiable(thisId), glId(0) { }
    TextureBase(const std::string &name) : name(name), glId(0) { }
    
    virtual ~TextureBase() { }
    
    /// Return the unique GL ID.
    GLuint getGLId() const { return glId; }

    /// Render side only.  Don't call this.  Create the openGL version
	virtual bool createInGL(OpenGLMemManager *memManager) {  return false; }
	
	/// Render side only.  Don't call this.  Destroy the openGL version
	virtual void destroyInGL(OpenGLMemManager *memManager) { }

protected:
	/// OpenGL ES ID
	/// Set to 0 if we haven't loaded yet
	GLuint glId;    
    
    /// Used for debugging
    std::string name;
};
    
/// For single byte pixels, what's the source, R G B or A?
typedef enum {WKSingleRed,WKSingleGreen,WKSingleBlue,WKSingleRGB,WKSingleAlpha} WKSingleByteSource;

/** Your basic Texture representation.
    This is how you get an image sent over to the
    rendering engine.  Set up one of these and add it.
    If you want to remove it, you need to use its
    Identifiable ID.
 */
class Texture : public TextureBase
{
public:
    /// Construct emty
	Texture(const std::string &name);
	/// Construct with raw texture data.  PVRTC is preferred.
	Texture(const std::string &name,RawDataRef,bool isPVRTC);
    /// Construct from a FILE, presumably because it was cached
    Texture(const std::string &name,FILE *fp);
	
	virtual ~Texture();
    
    /// Set the raw data directly
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
    /// Set the format (before createInGL() is called)
    void setFormat(GLenum inFormat) { format = inFormat; }
    /// If we're converting to a single byte, set the source
    void setSingleByteSource(WKSingleByteSource source) { byteSource = source; }

    /// Render side only.  Don't call this.  Create the openGL version
    virtual bool createInGL(OpenGLMemManager *memManager);
	
    /// Render side only.  Don't call this.  Destroy the openGL version
    virtual void destroyInGL(OpenGLMemManager *memManager);
	
    /// Sort the PKM data out from the NSData
    /// This is static so the dynamic (haha) textures can use it
    static unsigned char *ResolvePKM(RawDataRef texData,int &pkmType,int &size,int &width,int &height);

protected:
    /// Raw texture data
    RawDataRef texData;
    /// Need to know how we're going to load it
    bool isPVRTC;
    /// This one has a header
    bool isPKM;
    /// If not PVRTC, the format we'll use for the texture
    GLenum format;
    /// If we're converting down to one byte, where do we get it?
    WKSingleByteSource byteSource;
	
	unsigned int width,height;
    bool usesMipmaps;
    bool wrapU,wrapV;
};
	
}
