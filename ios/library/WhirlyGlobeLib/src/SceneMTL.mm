/*  SceneMTL.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "SceneMTL.h"
#import "TextureMTL.h"
#import "DrawableMTL.h"
#import "FontTextureManager.h"

namespace WhirlyKit
{

SceneMTL::SceneMTL(CoordSystemDisplayAdapter *adapter)
    : Scene(adapter)
{
}
    
id<MTLTexture> SceneMTL::getMTLTexture(SimpleIdentity texIdent)
{
    if (texIdent == EmptyIdentity)
        return nil;
    
    id<MTLTexture> ret = nil;
    std::lock_guard<std::mutex> guardLock(textureLock);
    // Might be a texture ref
    auto it = textures.find(texIdent);
    if (it != textures.end())
    {
        TextureBaseMTLRef tex = std::dynamic_pointer_cast<TextureBaseMTL> (it->second);
        ret = tex->getMTLID();
    }
    
    return ret;
}

void SceneMTL::teardown(PlatformThreadInfo *inst)
{
    for (const auto &it : drawables) {
        if (auto draw = dynamic_cast<DrawableMTL *>(it.second.get())) {
            draw->teardownForRenderer((RenderSetupInfoMTL *)setupInfo,this,nullptr);
        }
    }
    drawables.clear();
    for (auto it : textures) {
        it.second->destroyInRenderer(setupInfo,this);
    }
    textures.clear();
    if (fontTextureManager) {
        fontTextureManager->teardown(inst);
    }
}
    
}
