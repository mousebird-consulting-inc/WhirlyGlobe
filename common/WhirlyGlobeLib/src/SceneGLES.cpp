/*  SceneGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/14/19.
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

#import "SceneGLES.h"
#import "TextureGLES.h"
#import "FontTextureManager.h"

namespace WhirlyKit
{
    
SceneGLES::SceneGLES(CoordSystemDisplayAdapter *adapter)
    : Scene(adapter)
{
}

GLuint SceneGLES::getGLTexture(SimpleIdentity texIdent)
{
    if (texIdent == EmptyIdentity)
        return 0;
    
    std::lock_guard<std::mutex> guardLock(textureLock);

    // Might be a texture ref
    const auto it = textures.find(texIdent);
    if (it != textures.end())
    {
        if (auto tex = dynamic_cast<TextureBaseGLES*>(it->second.get())) {
            return tex->getGLId();
        }
    }
    return 0;
}

void SceneGLES::teardown(PlatformThreadInfo* threadInfo)
{
    for (const auto& it : drawables)
        it.second->teardownForRenderer(setupInfo,this, nullptr);
    drawables.clear();
    for (const auto& it : textures) {
        it.second->destroyInRenderer(setupInfo,this);
    }
    textures.clear();
    
    memManager.clearBufferIDs();
    memManager.clearTextureIDs();

    if (fontTextureManager)
    {
        fontTextureManager->teardown(threadInfo);
    }
}

}
