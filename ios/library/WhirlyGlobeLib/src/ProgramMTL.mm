/*
 *  ProgramMTL.mm
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

#import <MetalKit/MetalKit.h>
#import "ProgramMTL.h"
#import "TextureMTL.h"
#import "DefaultShadersMTL.h"
#import "SceneRendererMTL.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
ProgramMTL::TextureEntry::TextureEntry()
: slot(-1), tex(nil)
{
}
    
ProgramMTL::ProgramMTL() : lightsLastUpdated(0.0), valid(false)
{
}
    
ProgramMTL::ProgramMTL(const std::string &inName,id<MTLFunction> vertFunc,id<MTLFunction> fragFunc)
    : vertFunc(vertFunc), fragFunc(fragFunc), lightsLastUpdated(0.0), valid(true)
{
    name = inName;
}

ProgramMTL::~ProgramMTL()
{
}

bool ProgramMTL::isValid()
{
    return valid;
}

bool ProgramMTL::hasLights()
{
    // Lights are set up once for the renderer, so this makes no difference
    return true;
}

bool ProgramMTL::setLights(const std::vector<DirectionalLight> &lights, TimeInterval lastUpdated, Material *mat, Eigen::Matrix4f &modelMat)
{
    // We don't do lights this way, so it's all good
    return true;
}
    
bool ProgramMTL::setTexture(StringIdentity nameID,TextureBase *tex,int textureSlot)
{
    TextureBaseMTL *texMTL = dynamic_cast<TextureBaseMTL *>(tex);
    if (!texMTL)
        return false;
    
    TextureEntry texEntry;
    texEntry.slot = textureSlot;
    texEntry.tex = texMTL->getMTLID();
    textures.push_back(texEntry);
    
    return true;
}

const std::string &ProgramMTL::getName()
{ return name; }

void ProgramMTL::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene)
{
    // Don't really need to do anything here
}

void ProgramMTL::addResources(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,SceneMTL *scene)
{
    // Slot in the textures
    for (auto texEntry : textures) {
        [cmdEncode setVertexTexture:texEntry.tex atIndex:WKSTextureEntryLookup+texEntry.slot];
        [cmdEncode setFragmentTexture:texEntry.tex atIndex:WKSTextureEntryLookup+texEntry.slot];
    }
}
    
}
