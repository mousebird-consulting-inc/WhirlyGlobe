/*
 *  DynamicTextureAtlasGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/18/19.
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

#import <vector>
#import <set>

#import "DynamicTextureAtlas.h"

namespace WhirlyKit
{
    
/** The dynamic texture can have pieces of itself replaced in the layer thread while
 being used in the renderer.  It's used to implement dynamic texture atlases.
 */
class DynamicTextureGLES : public DynamicTexture
{
public:
    /// Construct with a name, square texture size, cell size (in texels), and the memory format
    DynamicTexture(const std::string &name,int texSize,int cellSize,GLenum format,bool clearTextures);
    
    /// Represents a region in the texture
    class Region
    {
    public:
        Region();
        int sx,sy,ex,ey;
    };
    
    /// Create an appropriately empty texture in OpenGL ES
    virtual bool createInRenderer(RenderSetupInfo *setupInfo);
    
    /// Render side only.  Don't call this.  Destroy the OpenGL ES version
    virtual void destroyInRenderer(RenderSetupInfo *setupInfo);

    /// Set the interpolation type used for min and mag
    void setInterpType(GLenum inType) { interpType = inType; }
    GLenum getInterpType() { return interpType; }
    
protected:
    /// Interpolation type
    GLenum interpType;
    /// Texture memory format
    GLenum format,type;
};

/** The dynamic texture atlas manages a variable number of dynamic textures into which it will stuff
 individual textures.  You use it by adding your individual Textures and passing the
 change requests on to the layer thread (or Scene).  You can also clear your Textures later
 by region.
 */
class DynamicTextureAtlasGLES : public DynamicTextureAtlas
{
public:
    DynamicTextureAtlasGLES(int texSize,int cellSize,GLenum format,int imageDepth=1,bool mainThreadMerge=false);
    virtual ~DynamicTextureAtlasGLES();
    
    /// Set the interpolation type used for min and mag
    void setInterpType(GLenum inType) { interpType = inType; }
    GLenum getInterpType() { return interpType; }
    
    /// Try to add the texture to one of our dynamic textures, or create one.
    bool addTexture(const std::vector<Texture *> &textures,int frame,Point2f *realSize,Point2f *realOffset,SubTexture &subTex,ChangeSet &changes,int borderPixels,int bufferPixels=0,TextureRegion *outTexRegion=NULL);
    
    /// Return the dynamic texture's format
    GLenum getFormat() { return format; }
    
protected:
    GLenum format;
    /// Interpolation type
    GLenum interpType;
};
    
}
