/*
 *  DynamicTextureAtlasMTL.mm
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

#import "DynamicTextureAtlasMTL.h"

namespace WhirlyKit
{
    
DynamicTextureMTL::DynamicTextureMTL(const std::string &name)
: DynamicTexture(name), TextureBase(name), TextureBaseMTL(name)
{
}

void DynamicTextureMTL::setup(int texSize,int cellSize,TextureType format,bool clearTextures)
{
    // TODO: Implement
}

void DynamicTextureMTL::addTextureData(int startX,int startY,int width,int height,RawDataRef data)
{
    // TODO: Implement
}

void DynamicTextureMTL::clearTextureData(int startX,int startY,int width,int height,ChangeSet &changes,bool mainThreadMerge,unsigned char *emptyData)
{
    // TODO: Implement
}

bool DynamicTextureMTL::createInRenderer(const RenderSetupInfo *setupInfo)
{
    // TODO: Implement
    return false;
}

void DynamicTextureMTL::destroyInRenderer(const RenderSetupInfo *setupInfo)
{
    // TODO: Implement
}
    
}
