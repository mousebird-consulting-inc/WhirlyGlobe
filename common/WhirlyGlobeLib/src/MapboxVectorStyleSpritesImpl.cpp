/*
 *  MapboxVectorStyleSpritesImpl.cpp
 *  WhirlyGlobe-MaplyComponent
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

#import "MapboxVectorStyleSpritesImpl.h"
#import "MapboxVectorStyleSetC.h"
#import "WhirlyKitLog.h"
#import <vector>

namespace WhirlyKit
{

MapboxSpriteEntry::MapboxSpriteEntry()
: width(0), height(0), pixelRatio(0), x(0), y(0)
{ }

MapboxVectorStyleSprites::MapboxVectorStyleSprites(SimpleIdentity wholeTexID,int texWidth,int texHeight)
: wholeTexID(wholeTexID), texWidth(texWidth), texHeight(texHeight)
{    
}

bool MapboxVectorStyleSprites::parse(MapboxVectorStyleSetImplRef styleSet,DictionaryRef spriteDict)
{
    auto keys = spriteDict->getKeys();
    for (auto key : keys) {
        auto entry = spriteDict->getDict(key);
        if (entry) {
            MapboxSpriteEntry sprite;
            sprite.name = key;
            sprite.width = entry->getInt("width");
            sprite.height = entry->getInt("height");
            sprite.pixelRatio = entry->getInt("pixelRatio");
            sprite.x = entry->getInt("x");
            sprite.y = entry->getInt("y");
            sprite.subTex.texId = wholeTexID;
            sprite.subTex.setFromTex(TexCoord(sprite.x / (double)texWidth, sprite.y / (double)texHeight),
                                     TexCoord((sprite.x+sprite.width) / (double)texWidth, (sprite.y+sprite.height) / (double)texHeight));
            styleSet->scene->addSubTexture(sprite.subTex);
            
            sprites[key] = sprite;
        }
    }
    
    return true;
}

SubTexture MapboxVectorStyleSprites::getTexture(const std::string &spriteName,Point2d &size)
{
    auto it = sprites.find(spriteName);
    if (it == sprites.end()) {
        size.x() = 0.0;
        size.y() = 0.0;
        return SubTexture();
    }

    size.x() = it->second.width / it->second.pixelRatio;
    size.y() = it->second.height / it->second.pixelRatio;
    return it->second.subTex;
}

MapboxSpriteEntry MapboxVectorStyleSprites::getSprite(const std::string &spriteName)
{    
    auto it = sprites.find(spriteName);
    if (it == sprites.end()) {
        return MapboxSpriteEntry();
    }

    return it->second;
}

void MapboxVectorStyleSprites::shutdown(MapboxVectorStyleSetImpl *styleSet,ChangeSet &changes)
{
    for (auto sprite: sprites)
        styleSet->scene->removeSubTexture(sprite.second.subTex.getId());
    sprites.clear();
    if (wholeTexID != EmptyIdentity)
        changes.push_back(new RemTextureReq(wholeTexID));
    wholeTexID = EmptyIdentity;
}

}
