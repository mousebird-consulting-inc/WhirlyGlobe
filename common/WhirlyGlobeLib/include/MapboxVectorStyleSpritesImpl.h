/*
*  MapboxVestorStyleSpritsImple.h
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 7/28/20.
*  Copyright 2011-2020 mousebird consulting
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

#import "Dictionary.h"
#import "Scene.h"
#import "TextureAtlas.h"
#import <string>

namespace WhirlyKit
{

class MapboxVectorStyleSetImpl;
typedef std::shared_ptr<MapboxVectorStyleSetImpl> MapboxVectorStyleSetImplRef;

// Entry for a single sprite in the larger whole
class MapboxSpriteEntry {
public:
    MapboxSpriteEntry();
    
    std::string name;
    // Size, obviously
    int width,height;
    // Not totally sure
    int pixelRatio;
    // Offset within the larger image
    int x,y;
    // Defines the sub-texture within the larger sprite sheet
    SubTexture subTex;
};

/**
  Holds the sprite sheet information for use in the symbol creation.
 */
class MapboxVectorStyleSprites
{
public:
    // Set up the sprints (including creating the texture)
    MapboxVectorStyleSprites(SimpleIdentity wholeTexID,int texWidth,int texHeight);
    
    // Parse the sprite.  Return false on failure.
    bool parse(MapboxVectorStyleSetImplRef styleSet,DictionaryRef spriteDict);
    
    // Return the sub-texture for a given sprite and the total size (in pixels)
    SubTexture getTexture(const std::string &spriteName,Point2d &size);
    
    // Return the sprite corresponding to the name
    MapboxSpriteEntry getSprite(const std::string &spriteName);
    
    // Clean up all the resources associated with the sprite sheet
    void shutdown(MapboxVectorStyleSetImpl *styleSet,ChangeSet &changes);
    
public:
    int texWidth,texHeight;
    SimpleIdentity wholeTexID;
    std::map<std::string,MapboxSpriteEntry> sprites;
};
typedef std::shared_ptr<MapboxVectorStyleSprites> MapboxVectorStyleSpritesRef;

}
