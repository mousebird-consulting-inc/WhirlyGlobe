/*
 *  TextureGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/19.
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
 *
 */

#import "Platform.h"
#import "RawData.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Texture.h"
#import "WrapperGLES.h"

namespace WhirlyKit
{
    
/** Base class for textures.  This is enough information to
 track it in the Scene, but little else.
 */
struct TextureBaseGLES : virtual public TextureBase
{
    TextureBaseGLES() = default;
    TextureBaseGLES(SimpleIdentity thisId) : TextureBase(thisId) { }
    TextureBaseGLES(std::string name) : TextureBase(std::move(name)) { }
    
    /// Return the unique GL ID.
    GLuint getGLId() const { return glId; }

protected:
    /// OpenGL ES ID
    /// Set to 0 if we haven't loaded yet
    GLuint glId = 0;
};
    
typedef std::shared_ptr<TextureBaseGLES> TextureBaseGLESRef;

/** Your basic Texture representation.
 This is how you get an image sent over to the rendering engine.  Set up one of these and add it.
 If you want to remove it, you need to use its Identifiable ID.
 */
struct TextureGLES : virtual public Texture, virtual public TextureBaseGLES
{
    TextureGLES();
    TextureGLES(std::string name);

    /// Construct with raw texture data.  PVRTC is preferred.
    TextureGLES(std::string name, RawDataRef texData, bool isPVRTC);

    TextureGLES(RawDataRef texData, TextureType fmt, int width, int height, bool isPVRTC);
    TextureGLES(std::string name, RawDataRef texData,
                TextureType fmt, int width, int height, bool isPVRTC);

    /// Render side only.  Don't call this.  Create the openGL version
    virtual bool createInRenderer(const RenderSetupInfo *setupInfo);
    
    /// Render side only.  Don't call this.  Destroy the openGL version
    virtual void destroyInRenderer(const RenderSetupInfo *setupInfo,Scene *scene);

    /// Sort the PKM data out from the NSData
    /// This is static so the dynamic (haha) textures can use it
    static unsigned char *ResolvePKM(const RawDataRef &texData,int &pkmType,int &size,int &width,int &height);
};
    
typedef std::shared_ptr<TextureGLES> TextureGLESRef;
    
}
