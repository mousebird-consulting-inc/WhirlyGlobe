/*
 *  TextureAtlas.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/11.
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

#import <vector>
#import "glwrapper.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Texture.h"
<<<<<<< HEAD
=======
#import "GlobeScene.h"
#import "LayerThread.h"
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

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
    SubTexture() : texId(EmptyIdentity) { trans.setIdentity(); }
    
    /// Set up the transform from destination texture coordinates
    void setFromTex(const TexCoord &texOrg,const TexCoord &texDest);
    
    /// Convert the texture coordinate to the destination texture
    TexCoord processTexCoord(const TexCoord &);
    
    /// Convert a list of texture coordinates to the dest texture
    void processTexCoords(std::vector<TexCoord> &);
    
    /// Sort operator
    bool operator < (const SubTexture &that) const { return this->myId < that.myId; }
    
    /// The larger texture we're pointing into
    SimpleIdentity texId;

    /// Transform from the source images texture coordinates to the target
    Eigen::Affine2f trans;
};
    
}

<<<<<<< HEAD
// Note: Porting
///** A Texture Atlas is used to consolidate textures
//    for performance.  OpenGL doesn't like having a lot of little
//    textures and would much prefer one big one.  This is how we do
//    that.
//    Texture Atlases are typically built on the fly with images that
//    come in from other sources.
// */
//@interface TextureAtlas : NSObject
//
///// This is the texture ID that will be assigned when the texture is created
//@property (nonatomic,readonly) WhirlyKit::SimpleIdentity texId;
//
///// Construct with texture size (needs to be a power of 2).
///// We sort images into buckets (sizeX/gridX,sizeY/gridY)
//- (id)initWithTexSizeX:(unsigned int)texSizeX texSizeY:(unsigned int)texSizeY cellSizeX:(unsigned int)cellSizeX cellSizeY:(unsigned int)cellSizeY;
//    
///// Add the image to this atlas and return texture coordinates
/////  to map into.
///// Returns false if there wasn't room
//- (BOOL)addImage:(UIImage *)image texOrg:(WhirlyKit::TexCoord &)org texDest:(WhirlyKit::TexCoord &)dest;
//
///// We cache the images and their coordinates.  Query the cache
//- (BOOL)getImageLayout:(UIImage *)image texOrg:(WhirlyKit::TexCoord &)org texDest:(WhirlyKit::TexCoord &)dest;
//
///// Generate a texture from the images
///// If the retImage pointer is set, you get that back.  It's autreleased.
//- (WhirlyKit::Texture *)createTexture:(UIImage **)retImage;
//
//@end
//
///** The Texture Atlas Builder is used to build up a list of texture atlases which will be used to
//    speed up rendering related to textures.  You give this object UIImages
//    and it will sort them into appropriate texture atlases.  In return you get an
//    ID which you can use to uniquely identify your texture subset and a mapping into
//    the target texture atlas.  These IDs can be used in place of texture IDs with
//    most layers.  Lastly, you'll need to add all the textures and sub texture mappings
//    into the scene.
//  */
//@interface TextureAtlasBuilder : NSObject
//
///// Construct with the size of the texture atlases to be produced.
///// Must be a power of two.
//- (id)initWithTexSizeX:(unsigned int)texSizeX texSizeY:(unsigned int)texSizeY;
//
///// Add the given image to a texture atlas.  You'll get a sub texture mapping back.
///// Check the ID of SubTexture.  It will be EmptyIdentity on failure.
//- (WhirlyKit::SimpleIdentity)addImage:(UIImage *)image;
//
///// Runs through the altases created and adds the resulting textures to the scene.
///// Also puts the sub texture mappings in to the scene for use on the layer side.
//- (void)processIntoScene:(WhirlyKit::Scene *)scene layerThread:(WhirlyKitLayerThread *)layerThread texIDs:(std::set<WhirlyKit::SimpleIdentity> *)texIDs;
//
//@end
=======
/** A Texture Atlas is used to consolidate textures
    for performance.  OpenGL doesn't like having a lot of little
    textures and would much prefer one big one.  This is how we do
    that.
    Texture Atlases are typically built on the fly with images that
    come in from other sources.
 */
@interface TextureAtlas : NSObject

/// This is the texture ID that will be assigned when the texture is created
@property (nonatomic,readonly) WhirlyKit::SimpleIdentity texId;

/// Construct with texture size (needs to be a power of 2).
/// We sort images into buckets (sizeX/gridX,sizeY/gridY)
- (id)initWithTexSizeX:(unsigned int)texSizeX texSizeY:(unsigned int)texSizeY cellSizeX:(unsigned int)cellSizeX cellSizeY:(unsigned int)cellSizeY;
    
/// Add the image to this atlas and return texture coordinates
///  to map into.
/// Returns false if there wasn't room
- (BOOL)addImage:(UIImage *)image texOrg:(WhirlyKit::TexCoord &)org texDest:(WhirlyKit::TexCoord &)dest;

/// We cache the images and their coordinates.  Query the cache
- (BOOL)getImageLayout:(UIImage *)image texOrg:(WhirlyKit::TexCoord &)org texDest:(WhirlyKit::TexCoord &)dest;

/// Generate a texture from the images
/// If the retImage pointer is set, you get that back.  It's autreleased.
- (WhirlyKit::Texture *)createTexture:(UIImage **)retImage;

@end

/** The Texture Atlas Builder is used to build up a list of texture atlases which will be used to
    speed up rendering related to textures.  You give this object UIImages
    and it will sort them into appropriate texture atlases.  In return you get an
    ID which you can use to uniquely identify your texture subset and a mapping into
    the target texture atlas.  These IDs can be used in place of texture IDs with
    most layers.  Lastly, you'll need to add all the textures and sub texture mappings
    into the scene.
  */
@interface TextureAtlasBuilder : NSObject

/// Construct with the size of the texture atlases to be produced.
/// Must be a power of two.
- (id)initWithTexSizeX:(unsigned int)texSizeX texSizeY:(unsigned int)texSizeY;

/// Add the given image to a texture atlas.  You'll get a sub texture mapping back.
/// Check the ID of SubTexture.  It will be EmptyIdentity on failure.
- (WhirlyKit::SimpleIdentity)addImage:(UIImage *)image;

/// Runs through the altases created and adds the resulting textures to the scene.
/// Also puts the sub texture mappings in to the scene for use on the layer side.
- (void)processIntoScene:(WhirlyKit::Scene *)scene layerThread:(WhirlyKitLayerThread *)layerThread texIDs:(std::set<WhirlyKit::SimpleIdentity> *)texIDs;

@end
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
