/*
 *  SceneRenderereES_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/28/19.
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

#import <UIKit/UIKit.h>
#import "SceneRendererES_iOS.h"

namespace WhirlyKit {
    
SceneRendererES_iOS::SceneRendererES_iOS()
{
    int version = kEAGLRenderingAPIOpenGLES3;
    
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!context) {
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        version = kEAGLRenderingAPIOpenGLES2;
    }
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    useContext();
    
    setup(version,0,0);
    
    [EAGLContext setCurrentContext:oldContext];
}
    
SceneRendererES_iOS::~SceneRendererES_iOS()
{
    EAGLContext *oldContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:context];

    // Have to clear these out here so we can set the context
    for (RenderTargetRef target : renderTargets)
        target->clear();
    
    renderTargets.clear();

    [EAGLContext setCurrentContext:oldContext];
}
    
EAGLContext *SceneRendererES_iOS::getContext()
{
    return context;
}

void SceneRendererES_iOS::useContext()
{
    if (!context)
        return;
    [EAGLContext setCurrentContext:context];
}

void SceneRendererES_iOS::defaultTargetInit(RenderTarget *renderTarget)
{
    if (!layer)
        return;
    
    if (![context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)layer])
        NSLog(@"SceneRendererES: Failure in renderbufferStorage");
}
    
void SceneRendererES_iOS::setLayer(CAEAGLLayer *inLayer)
{
    layer = inLayer;
}
    
void SceneRendererES_iOS::presentRender()
{
    if (!layer)
        return;
    
    [context presentRenderbuffer:GL_RENDERBUFFER];
}
    
}
