/*
 *  ChangeRequest.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/8/19.
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

#import "ChangeRequest.h"
#import "Texture.h"
#import "Drawable.h"
#import "SceneRenderer.h"

namespace WhirlyKit
{

void RenderTeardownInfo::destroyTexture(SceneRenderer *renderer,const TextureBaseRef &tex)
{
    tex->destroyInRenderer(renderer->getRenderSetupInfo(), renderer->getScene());
}

void RenderTeardownInfo::destroyDrawable(SceneRenderer *renderer,const DrawableRef &draw)
{
    draw->teardownForRenderer(renderer->getRenderSetupInfo(), renderer->getScene(), renderer->teardownInfo);
}

ChangeRequest::ChangeRequest() : when(0.0) { }

ChangeRequest::~ChangeRequest()
{
}

bool ChangeRequest::needsFlush() { return false; }

void ChangeRequest::setupForRenderer(const RenderSetupInfo *,Scene *scene) { }

bool ChangeRequest::needPreExecute() { return false; }

}
