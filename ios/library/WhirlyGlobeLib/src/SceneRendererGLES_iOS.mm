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
#import "SceneRendererGLES_iOS.h"

namespace WhirlyKit {
    
SceneRendererGLES_iOS::SceneRendererGLES_iOS(float scale)
    : layer(nil), context(nil)
{
    int version = kEAGLRenderingAPIOpenGLES3;
    
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!context) {
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        version = kEAGLRenderingAPIOpenGLES2;
    }
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    useContext();
    
    setup(version,0,0,scale);
    
    [EAGLContext setCurrentContext:oldContext];
}
    
SceneRendererGLES_iOS::SceneRendererGLES_iOS(int width,int height,float scale)
{
    int version = kEAGLRenderingAPIOpenGLES3;
    
    context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!context) {
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        version = kEAGLRenderingAPIOpenGLES2;
    }
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    useContext();
    
    setup(version,width,height,scale);
    
    [EAGLContext setCurrentContext:oldContext];
}
    
SceneRendererGLES_iOS::~SceneRendererGLES_iOS()
{
    EAGLContext *oldContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:context];

    // Have to clear these out here because we can set the context
    for (RenderTargetRef target : renderTargets)
        target->clear();
    
    renderTargets.clear();

    [EAGLContext setCurrentContext:oldContext];
}
    
EAGLContext *SceneRendererGLES_iOS::getContext()
{
    return context;
}

void SceneRendererGLES_iOS::useContext()
{
    if (!context)
        return;
    [EAGLContext setCurrentContext:context];
}

void SceneRendererGLES_iOS::defaultTargetInit(RenderTarget *renderTarget)
{
    if (!layer)
        return;
    
    if (![context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)layer])
        NSLog(@"SceneRendererES: Failure in renderbufferStorage");
}
    
void SceneRendererGLES_iOS::setLayer(CAEAGLLayer *inLayer)
{
    layer = inLayer;
}
    
void SceneRendererGLES_iOS::presentRender()
{
    if (!layer)
        return;
    
    [context presentRenderbuffer:GL_RENDERBUFFER];
}

void SceneRendererGLES_iOS::addSnapshotDelegate(NSObject<WhirlyKitSnapshot> *newDelegate)
{
    snapshotDelegates.push_back(newDelegate);
}

void SceneRendererGLES_iOS::removeSnapshotDelegate(NSObject<WhirlyKitSnapshot> *oldDelegate)
{
    snapshotDelegates.erase(std::remove(snapshotDelegates.begin(), snapshotDelegates.end(), oldDelegate), snapshotDelegates.end());
}

void SceneRendererGLES_iOS::snapshotCallback(TimeInterval now)
{
    for (auto snapshotDelegate : snapshotDelegates) {
        if (![snapshotDelegate needSnapshot:now])
            continue;
        
        // They'll want a snapshot of a specific render target (or the screen)
        for (auto target: renderTargets) {
            if (target->getId() == [snapshotDelegate renderTargetID]) {
                CGRect snapshotRect = [snapshotDelegate snapshotRect];
                NSData *dataWrapper = nil;
                
                if (target->getId() == EmptyIdentity) {
                    // Screen is special
                    NSInteger dataLength = framebufferWidth * framebufferHeight * 4;
                    NSMutableData *theData = [NSMutableData dataWithLength:dataLength];
                    
                    // Read pixel data from the framebuffer
                    glPixelStorei(GL_PACK_ALIGNMENT, 4);
                    glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, [theData mutableBytes]);
                    
                    // Also wrap up the raw data
                    dataWrapper = theData;
                } else {
                    // Offscreen render buffer
                    RawDataRef rawData;
                    if (snapshotRect.size.width == 0.0)
                        rawData = target->snapshot();
                    else
                        rawData = target->snapshot(snapshotRect.origin.x,snapshotRect.origin.y,snapshotRect.size.width,snapshotRect.size.height);
                    // Note: This is an extra copy
                    dataWrapper = [[NSData alloc] initWithBytes:rawData->getRawData() length:rawData->getLen()];
                }
                
                [snapshotDelegate snapshotData:dataWrapper];
                
                break;
            }
        }
    }
}

}
