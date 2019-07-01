/*
 *  TextureAtlas.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/11.
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
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Texture.h"

namespace WhirlyKit
{

/** Sub Textures are used to index from an image into a larger texture atlas.
    We need to combine images together for efficiency's sake.  This lets us
    pretend we're dealing with individual textures.  You can use their IDs
    in place of texture IDs for most of the layers.
  */
class SubTexture : public Identifiable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    SubTexture() : texId(EmptyIdentity) { trans.setIdentity(); }
    SubTexture(SimpleIdentity subTexID) : Identifiable(subTexID) { }
    
    /// Set up the transform from destination texture coordinates
    void setFromTex(const TexCoord &texOrg,const TexCoord &texDest);
    
    /// Convert the texture coordinate to the destination texture
    TexCoord processTexCoord(const TexCoord &) const;
    
    /// Convert a list of texture coordinates to the dest texture
    void processTexCoords(std::vector<TexCoord> &) const;
    
    /// Sort operator
    bool operator < (const SubTexture &that) const { return this->myId < that.myId; }
    
    /// The larger texture we're pointing into
    SimpleIdentity texId;

    /// Transform from the source images texture coordinates to the target
    Eigen::Affine2f trans;
};
    
}

