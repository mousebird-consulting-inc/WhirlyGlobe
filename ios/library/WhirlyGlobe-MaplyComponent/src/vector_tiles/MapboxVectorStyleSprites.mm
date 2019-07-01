/*
 *  MapboxVectorStyleSprites.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/3/19.
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

#import "MapboxVectorStyleSprites.h"
#import <map>
#import <string>

namespace WhirlyKit
{
    
// Entry for a single sprite in the larger whole
class SpriteEntry {
public:
    SpriteEntry() : name(nil), width(0), height(0), pixelRatio(0), x(0), y(0), tex(nil) { }
    
    NSString *name;
    // Size, obviously
    int width,height;
    // Note totally sure
    int pixelRatio;
    // Offset within the larger image
    int x,y;
    // Might have been defined
    MaplyTexture *tex;
};
    
}

using namespace WhirlyKit;

@implementation MapboxVectorStyleSprites
{
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    std::map<std::string,SpriteEntry> sprites;
    MaplyTexture *wholeTex;
}

- (id)initWithJSON:(NSData *)spriteJSON image:(UIImage *)spriteIMG settings:(MaplyVectorStyleSettings *)settings viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    
    NSError *error = nil;
    NSDictionary *spriteDict = [NSJSONSerialization JSONObjectWithData:spriteJSON options:NULL error:&error];
    if (!spriteDict)
        return nil;

    for (NSString *key in spriteDict.allKeys) {
        NSDictionary *entry = spriteDict[key];
        if (![entry isKindOfClass:[NSDictionary class]]) {
            NSLog(@"Error parsing sprite sheet.  Failing.");
            return nil;
        }
        
        SpriteEntry newEntry;
        newEntry.name = key;
        newEntry.width = [entry[@"width"] intValue];
        newEntry.height = [entry[@"height"] intValue];
        newEntry.pixelRatio = [entry[@"pixelRatio"] intValue];
        newEntry.x = [entry[@"x"] intValue];
        newEntry.y = [entry[@"y"] intValue];
        const char *nameStr = [key cStringUsingEncoding:NSASCIIStringEncoding];

        if (nameStr)
            sprites[(std::string)nameStr] = newEntry;
    }
    
    // Create the sprite texture
    wholeTex = [viewC addTexture:spriteIMG desc:nil mode:MaplyThreadCurrent];

    return self;
}

- (MaplyTexture *)getWholeTexture
{
    return wholeTex;
}

- (MaplyTexture *)getTexture:(NSString *)spriteName
{
    if (!wholeTex)
        return nil;
    
    // Look for the name
    std::string nameStr = (std::string)[spriteName cStringUsingEncoding:NSASCIIStringEncoding];

    auto it = sprites.find(nameStr);
    if (it == sprites.end())
        return nil;

    // Already created it
    SpriteEntry entry = it->second;
    if (entry.tex)
        return entry.tex;
    
    // Nope, need to create it
    entry.tex = [viewC addSubTexture:wholeTex xOffset:entry.x yOffset:entry.y width:entry.width height:entry.height mode:MaplyThreadCurrent];
    
    sprites.erase(it);
    sprites[nameStr] = entry;
    
    return entry.tex;
}

- (void)shutdown
{
    NSMutableArray *toRemove = [NSMutableArray array];
    if (wholeTex) {
        [toRemove addObject:wholeTex];
        wholeTex = nil;
    }

    for (auto it : sprites) {
        if (it.second.tex)
            [toRemove addObject:it.second.tex];
    }
    sprites.clear();

    [viewC removeTextures:toRemove mode:MaplyThreadAny];
}

@end
