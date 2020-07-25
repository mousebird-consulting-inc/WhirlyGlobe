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

SceneRendererGLES_Android::SceneRendererGLES_Android()
        : context(0)
{
    extraFrameMode = true;
    context = eglGetCurrentContext();
    // If we pass in a size, we get an offscreen buffer
    setup(3, 0, 0, 1.0);
}

SceneRendererGLES_Android::SceneRendererGLES_Android(int width,int height)
{
    extraFrameMode = true;
    context = eglGetCurrentContext();
    setup(3,width,height, 1.0);
}

// Called when the window changes size (or on startup)
bool SceneRendererGLES_Android::resize(int width, int height)
{
    context = eglGetCurrentContext();

    RenderTargetGLESRef defaultTarget = std::dynamic_pointer_cast<RenderTargetGLES>(renderTargets.back());
    defaultTarget->initFromState(width, height);

    framebufferWidth = width;
    framebufferHeight = height;
    lastDraw = 0;
    forceRenderSetup();

    return true;
}

void SceneRendererGLES_Android::snapshotCallback(TimeInterval now)
{
    for (auto snapshotDelegate : snapshotDelegates) {
        if (!snapshotDelegate->needsSnapshot(now))
            continue;

        snapshotDelegate->runSnapshot(this);
    }
}

void SceneRendererGLES_Android::addSnapshotDelegate(Snapshot_AndroidRef snapshotDelegate)
{
    for (unsigned int which = 0;which<snapshotDelegates.size();which++)
        if (snapshotDelegate == snapshotDelegates[which])
            return;
    snapshotDelegates.push_back(snapshotDelegate);
}

void SceneRendererGLES_Android::removeSnapshotDelegate(Snapshot_AndroidRef snapshotDelegate)
{
    for (unsigned int which = 0;which<snapshotDelegates.size();which++) {
        if (snapshotDelegate == snapshotDelegates[which]) {
            snapshotDelegates.erase(snapshotDelegates.begin()+which);
            return;
        }
    }
}

}

