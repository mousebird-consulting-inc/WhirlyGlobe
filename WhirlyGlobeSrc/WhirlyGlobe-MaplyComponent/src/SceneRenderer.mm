/*
 *  SceneRenderer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/16/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "SceneRenderer_private.h"

namespace WhirlyKit
{
    
MaplySceneRendererES2::MaplySceneRendererES2()
    : _dispatchRendering(false)
{
}

void MaplySceneRendererES2::render(NSTimeInterval duration)
{
    // Let anyone who cares know the frame draw is starting
    WhirlyKitFrameMessage *frameMsg = [[WhirlyKitFrameMessage alloc] init];
    frameMsg.frameStart = CFAbsoluteTimeGetCurrent();
    frameMsg.frameInterval = duration;
    frameMsg.renderer = this;
    [[NSNotificationCenter defaultCenter] postNotificationName:kWKFrameMessage object:frameMsg];
    
    if (_dispatchRendering)
    {
        if (dispatch_semaphore_wait(frameRenderingSemaphore, DISPATCH_TIME_NOW) != 0)
            return;

        dispatch_async(contextQueue,
                       ^{
                           renderAsync();
                           dispatch_semaphore_signal(frameRenderingSemaphore);
                       });
    } else
    renderAsync();
}

void MaplySceneRendererES2::renderAsync()
{
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];
//    CheckGLError("SceneRendererES2: setCurrentContext");

    // C++ side rendering
    // Note: Porting.  The timing is wrong here for the performance timer
    SceneRendererES2::render();
    
    // Note: Porting
    //    [context presentRenderbuffer:GL_RENDERBUFFER];
//    CheckGLError("SceneRendererES2: presentRenderbuffer");

    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
}
    
}

