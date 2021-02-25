/*
 *  DynamicTextureAtlasMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "DynamicTextureAtlas.h"
#import "TextureMTL.h"
#import "WrapperMTL.h"

namespace WhirlyKit
{
    
/** The dynamic texture can have pieces of itself replaced in the layer thread while
 being used in the renderer.  It's used to implement dynamic texture atlases.
 */
class DynamicTextureMTL : virtual public DynamicTexture, virtual public TextureBaseMTL
{
public:
    /// Construct with a name, square texture size, cell size (in texels), and the memory format
    DynamicTextureMTL(const std::string &name);
    
    /// Called after construction to do the actual work
    void setup(int texSize,int cellSize,TextureType format,bool clearTextures);
    
    /// Add the data at a given location in the texture
    void addTextureData(int startX,int startY,int width,int height,RawDataRef data);
    
    /// Clear out the low level data
    void clearTextureData(int startX,int startY,int width,int height,ChangeSet &changes,bool mainThreadMerge,unsigned char *emptyData);
    
    /// Create an appropriately empty texture in OpenGL ES
    virtual bool createInRenderer(const RenderSetupInfo *setupInfo);
    
    /// Render side only.  Don't call this.  Destroy the OpenGL ES version
    virtual void destroyInRenderer(const RenderSetupInfo *setupInfo,Scene *scene);
    
protected:
    bool valid;
    MTLPixelFormat pixFormat;
    TextureType type;
    int bytesPerPixel;
    int bytesPerRow;
};
    
}
