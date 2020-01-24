/*
 *  MTLView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/20/19.
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

#import "MTLView.h"

using namespace WhirlyKit;

@interface WhirlyKitMTLView()<SceneRendererMTLDrawableGetter>
@end

@implementation WhirlyKitMTLView
{
    bool animating;
}

- (id)initWithDevice:(id<MTLDevice>)mtlDevice
{
    self = [super initWithFrame:CGRectZero device:mtlDevice];

    self.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    self.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
    self.framebufferOnly = true;
    animating = true;
//    self.preferredFramesPerSecond = 120;
    
    return self;
}

// Apparently they change this.  Because of course they can
- (void)setContentScaleFactor:(CGFloat)contentScaleFactor
{
    super.contentScaleFactor = contentScaleFactor;
    
    if (_renderer)
        _renderer->setScale(contentScaleFactor);
}

- (void)setRenderer:(WhirlyKit::SceneRenderer *)renderer
{
    SceneRendererMTL *renderMTL = dynamic_cast<SceneRendererMTL *>(renderer);
    if (!renderMTL)
        return;

    _renderer = renderer;
    _renderer->setScale(self.contentScaleFactor);

    renderMTL->setup(self.frame.size.width, self.frame.size.height,false);
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    
    SceneRendererMTL *renderMTL = dynamic_cast<SceneRendererMTL *>(_renderer);
    if (!renderMTL)
        return;

    renderMTL->resize((int)self.frame.size.width*self.contentScaleFactor,
                      (int)self.frame.size.height*self.contentScaleFactor);
}

- (id<CAMetalDrawable>)getDrawable
{
    return self.currentDrawable;
}

- (void)draw
{
    [super draw];
    
    SceneRendererMTL *renderMTL = dynamic_cast<SceneRendererMTL *>(_renderer);
    if (!renderMTL)
        return;
    
    if (animating) {
        renderMTL->getView()->animate();

        if (renderMTL->hasChanges()) {
            MTLRenderPassDescriptor *renderPassDesc = self.currentRenderPassDescriptor;
            if (!renderPassDesc)
                return;

            renderMTL->render(1.0/self.preferredFramesPerSecond,renderPassDesc,self);
        }
    } else {
        TimeInterval now = TimeGetCurrent();
        renderMTL->processScene(now);
    }
}

- (BOOL)isAnimating
{
    return animating;
}

- (void) startAnimation
{
    animating = true;
}

- (void) stopAnimation
{
    animating = false;
}

- (void) teardown
{
    _renderer = nil;
}

@end
