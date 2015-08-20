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
#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"
#import "DictionaryWrapper_private.h"
#import "UIImage+Stuff.h"

@implementation WhirlyKitFrameMessage
@end

namespace WhirlyKit
{
    
MaplySceneRendererES2::MaplySceneRendererES2()
    : _dispatchRendering(false), SceneRendererES2()
{
    // We do this to pull in the categories without the -ObjC flag.
    // It's dumb, but it works
    static bool dummyInit = false;
    if (!dummyInit)
    {
        UIImageDummyFunc();
        NSDictionaryDummyFunc();
        NSDictionaryDummyFunc2();
        UIColorDummyFunc();
//        NSStringDummyFunc();
        dummyInit = true;
    }

    scale = [[UIScreen mainScreen] scale];
    
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];

    // This creates the buffers and such
    setup();
    
    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
}
    
MaplySceneRendererES2::~MaplySceneRendererES2()
{
}
    
void MaplySceneRendererES2::useContext()
{
    if (context && [EAGLContext currentContext] != context)
        [EAGLContext setCurrentContext:context];
}
    
BOOL MaplySceneRendererES2::resizeFromLayer(CAEAGLLayer *layer)
{
    renderSetup = false;
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];

	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    CheckGLError("SceneRendererES: glBindRenderbuffer");
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)layer];
    CheckGLError("SceneRendererES: glBindRenderbuffer");
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);

	// For this sample, we also need a depth buffer, so we'll create and attach one via another renderbuffer.
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    CheckGLError("SceneRendererES: glBindRenderbuffer");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, framebufferWidth, framebufferHeight);
    CheckGLError("SceneRendererES: glRenderbufferStorage");
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    CheckGLError("SceneRendererES: glFramebufferRenderbuffer");

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (oldContext != context)
            [EAGLContext setCurrentContext:oldContext];
		return NO;
	}

    lastDraw = 0;

    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];

    // If we've resized, we're looking at different content
    if (theView)
        theView->runViewUpdates();
    
	return YES;
}

// When the scene is set, we'll compile our shaders
void MaplySceneRendererES2::setScene(WhirlyKit::Scene *inScene)
{
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];

    SceneRendererES2::setScene(inScene);
    
    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
}

void MaplySceneRendererES2::render(TimeInterval duration)
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
    
    [context presentRenderbuffer:GL_RENDERBUFFER];
    CheckGLError("SceneRendererES2: presentRenderbuffer");
    
    // Note: Porting
    //    [context presentRenderbuffer:GL_RENDERBUFFER];
//    CheckGLError("SceneRendererES2: presentRenderbuffer");

    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
}
    
}

