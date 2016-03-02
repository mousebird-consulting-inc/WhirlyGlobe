/*
 *  Texture.h
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

#import <UIKit/UIKit.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "BasicDrawable.h"

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
    /// Construct empty
	Texture(const std::string &name);
	/// Construct with raw texture data.  PVRTC is preferred.
	Texture(const std::string &name,NSData *texData,bool isPVRTC);
	/// Construct with a file name and extension
	Texture(const std::string &name,NSString *baseName,NSString *ext);
	/// Construct with a UIImage.  Expecting this to be a power of 2 on each side.
    /// If it's not we'll round up or down, depending on the flag
	Texture(const std::string &name,UIImage *inImage, bool roundUp=true);
    /// Construct by scaling the image to the given size
    Texture(const std::string &name,UIImage *inImage,int width,int height);
    /// Construct from a FILE, presumably because it was cached
    Texture(const std::string &name,FILE *fp);
	
	virtual ~Texture();
	    
    /// Process the data for display based on the format.
    NSData *processData();
    
    /// Set up from raw PKM (ETC2/EAC) data
    void setPKMData(NSData *data);
	
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
    /// Return the format
    GLenum getFormat() { return format; }
    /// Set the interpolation type used for min and mag
    void setInterpType(GLenum inType) { interpType = inType; }
    GLenum getInterpType() { return interpType; }
    /// If we're converting to a single byte, set the source
    void setSingleByteSource(WKSingleByteSource source) { byteSource = source; }

    /// Render side only.  Don't call this.  Create the openGL version
	virtual bool createInGL(OpenGLMemManager *memManager);
	
	/// Render side only.  Don't call this.  Destroy the openGL version
	virtual void destroyInGL(OpenGLMemManager *memManager);

    /// Sort the PKM data out from the NSData
    /// This is static so the dynamic (haha) textures can use it
    static unsigned char *ResolvePKM(NSData *texData,int &pkmType,int &size,int &width,int &height);

protected:
	/// Raw texture data
	NSData * __strong texData;
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
    GLenum interpType;
};
	
}
