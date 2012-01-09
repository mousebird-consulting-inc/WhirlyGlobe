/*
 *  RenderCache.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/19/11.
 *  Copyright 2011 mousebird consulting
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
#import <map>
#import "Texture.h"
#import "Drawable.h" 
#import "GlobeScene.h"

namespace WhirlyGlobe 
{
    
/// Check for the given render cache
bool RenderCacheExists(NSString *baseName);
    
/** The Render Cache Writer helps consolidate drawables in
    to an on-disk render cache that can be loaded quickly
    on startup.  It's used in conjuction with specific layers.
  */
class RenderCacheWriter
{
public:
    RenderCacheWriter(NSString *fileName);
    ~RenderCacheWriter();
    
    /// Textures are just written out to the cache area, not
    ///  as part of the cache file
    /// We'll track the tex ID to name mapping for writing
    ///  drawables as well.
    /// Returns an empty string on failure
    std::string addTexture(SimpleIdentity texId,UIImage *);
    
    /// Add a drawable to the cache file
    /// Returns false on failure
    bool addDrawable(const Drawable *);
    
    /// Ignore any texture references
    /// They just get turned in to EmptyIdentity
    void setIgnoreTextures();
    
protected:
    std::string fileBase;
    unsigned int numTextures,numDrawables;
    TextureIDMap texIDMap;
    FILE *fp;
    bool ignoreTextures;
};
    
/** The Render Cache Reader reads render caches, logically enough.
    It reads textures first and then drawables.  You can fetch the
    data as a couple of vectors or as callbacks, assuming you want
    to merge as you go.
  */
class RenderCacheReader
{
public:
    RenderCacheReader(NSString *fileName);
    ~RenderCacheReader();

    /// Read in the textures and drawables in the cache
    /// Caller responsible for deletion
    bool getDrawablesAndTextures(std::vector<Texture *> &textures,std::vector<Drawable *> &drawables);

    /// Read in the textures and drawables and add them to the
    ///  scene as we go.  Presumably the caller doesn't need to
    ///  make any changes.
    /// Returns the lists of textures and drawables added
    bool getDrawablesAndTexturesAddToScene(GlobeScene *scene,SimpleIDSet &texIDs,SimpleIDSet &drawIDs,float fade);
    
protected:
    unsigned int numTextures,numDrawables;
    std::string fileBase;
    std::string fileDir;
    TextureIDMap texIDMap;
    FILE *fp;
};

}
