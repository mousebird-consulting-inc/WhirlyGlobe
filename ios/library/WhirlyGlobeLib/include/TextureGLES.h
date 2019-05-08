/*
 *  TextureGLES.h
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
class TextureBaseGL : public TextureBase
{
public:
    /// Return the unique GL ID.
    GLuint getGLId() const { return glId; }
protected:
    /// OpenGL ES ID
    /// Set to 0 if we haven't loaded yet
    GLuint glId;
};
    
/** Your basic Texture representation.
 This is how you get an image sent over to the
 rendering engine.  Set up one of these and add it.
 If you want to remove it, you need to use its
 Identifiable ID.
 */
class TextureGL : public Texture, public TextureBaseGL
{
public:
    /// Set the format (before createInGL() is called)
    void setFormat(GLenum inFormat) { format = inFormat; }
    /// Return the format
    GLenum getFormat() { return format; }
    /// Set the interpolation type used for min and mag
    void setInterpType(GLenum inType) { interpType = inType; }
    GLenum getInterpType() { return interpType; }
    
    /// Render side only.  Don't call this.  Create the openGL version
    virtual bool createInRenderer(RenderSetupInfo *setupInfo);
    
    /// Render side only.  Don't call this.  Destroy the openGL version
    virtual void destroyInRenderer(RenderSetupInfo *setupInfo);

protected:
    /// If not PVRTC, the format we'll use for the texture
    GLenum format;
    GLenum interpType;
};
    
}
