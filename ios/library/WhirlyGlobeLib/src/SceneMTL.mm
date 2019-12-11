/*
 *  SceneMTL.mm
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

#import "SceneMTL.h"
#import "TextureMTL.h"
#import "DrawableMTL.h"

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

/// Retain a buffer to release it in a few frames
void SceneMTL::releaseBuffer(id buf)
{
    if (!buf)
        return;
    
    std::lock_guard<std::mutex> guardLock(bufferLock);
    buffers[WKMaxFramesInFlight-1].push_back(buf);
}

/// Do the dance to release buffers at the end of a frame
void SceneMTL::endOfFrameBufferClear()
{
    std::lock_guard<std::mutex> guardLock(bufferLock);
    buffers[0].clear();
    for (int ii=0;ii<WKMaxFramesInFlight-1;ii++) {
        buffers[ii] = buffers[ii+1];
    }
    buffers[WKMaxFramesInFlight-1].clear();
}

void SceneMTL::teardown()
{
    for (auto it : drawables) {
        DrawableMTL *draw = dynamic_cast<DrawableMTL *>(it.second.get());
        if (draw)
            draw->teardownForRenderer((RenderSetupInfoMTL *)setupInfo,this);
    }
    drawables.clear();
    for (auto it : textures) {
        it.second->destroyInRenderer(setupInfo,this);
    }
    textures.clear();
    for (auto &bufferList : buffers)
        bufferList.clear();
}
    
}
