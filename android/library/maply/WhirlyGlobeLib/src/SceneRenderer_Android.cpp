/*
 *  SceneRenderer_Android.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/27/19.
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

#import "SceneRenderer_Android.h"

namespace WhirlyKit {

SceneRendererES2_Android::SceneRendererES2_Android()
        : context(0)
{
    extraFrameMode = true;
}

SceneRendererES2_Android::SceneRendererES2_Android(int width,int height)
{
    context = eglGetCurrentContext();
    setup(3,width,height);
}

// Called when the window changes size (or on startup)
bool SceneRendererES2_Android::resize(int width, int height)
{
    context = eglGetCurrentContext();

    if (renderTargets.empty()) {
        RenderTargetRef defaultTarget(new RenderTarget(EmptyIdentity));
        defaultTarget->initFromState(width, height);
        renderTargets.push_back(defaultTarget);
    } else {
        RenderTargetRef defaultTarget = renderTargets.back();
        defaultTarget->initFromState(width, height);
    }

    framebufferWidth = width;
    framebufferHeight = height;
    lastDraw = 0;
    forceRenderSetup();

    return true;
}

}

