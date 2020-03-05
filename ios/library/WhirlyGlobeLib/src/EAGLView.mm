/*
 *  EAGLView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/5/11.
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

#import "EAGLView.h"
#import <QuartzCore/QuartzCore.h>
#import "SceneRendererGLES_iOS.h"

using namespace WhirlyKit;

@implementation WhirlyKitEAGLView 
{
    CADisplayLink *displayLink;
    bool resizeFail;
    int resizeFailRetry;
    NSInteger _frameInterval;
}

@synthesize frameInterval=_frameInterval;

// You must implement this method
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id)init
{
    self = [super init];
	if (self)
    {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
        self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		
        _animating = FALSE;
        _frameInterval = 1;
        self.useRetina = TRUE;
        resizeFail = false;
        resizeFailRetry = 0;
        _pauseDisplayLink = true;
    }
    
    return self;
}

- (void)dealloc
{
    [displayLink invalidate];
}

- (void)setUseRetina:(BOOL)newVal
{
    _useRetina = newVal;

    // Either use the device scale or just 1.0
    float scale = [UIScreen mainScreen].scale;
    if (!_useRetina)
            scale = 1.0;
    self.contentScaleFactor = scale;
    self.layer.contentsScale = scale;
}


- (NSInteger)frameInterval
{
    return _frameInterval;
}

- (void)setFrameInterval:(NSInteger)newFrameInterval
{
    if (newFrameInterval >= 1)
    {
        _frameInterval = newFrameInterval;
        [displayLink setFrameInterval:_frameInterval];
    }
}

- (void)startAnimation
{
    if (!_animating)
    {
        if (!displayLink)
        {
            displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
            [displayLink setFrameInterval:_frameInterval];
            if (_reactiveMode)
            {
                [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            } else
                [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];            
        } else
            displayLink.paused = NO;

        _animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (_animating)
    {
        _animating = FALSE;
        if (_pauseDisplayLink)
            displayLink.paused = true;
    }
}

- (void) teardown
{
    if (_animating)
    {
        _animating = NO;
    }
    if (displayLink)
    {
        [displayLink invalidate];
        displayLink = nil;
    }
}

- (void) drawView:(id)sender
{
    // Tried to resize and failed too many times
    if (resizeFail && resizeFailRetry <= 0)
        return;
    
    if (!_renderer)
        return;
    
    if (resizeFail)
        [self layoutSubviews];

    EAGLContext *oldContext = [EAGLContext currentContext];
    SceneRendererGLES_iOS *sceneRender = (SceneRendererGLES_iOS *)_renderer;
    sceneRender->useContext();

    if (_animating) {
        if (sceneRender->hasChanges())
            sceneRender->render(displayLink.frameInterval * 1/60.0);
        else {
            // Process the scene even if the window isn't up
            TimeInterval now = TimeGetCurrent();
            sceneRender->processScene(now);
        }
    } else {
        TimeInterval now = TimeGetCurrent();
        sceneRender->processScene(now);
    }
    
    [EAGLContext setCurrentContext:oldContext];
}

- (void) setFrame:(CGRect)newFrame
{
	[super setFrame:newFrame];
}

- (void) layoutSubviews
{
    // Make sure we're not backgrounded
    UIApplicationState state = [[UIApplication sharedApplication] applicationState];
    if (state == UIApplicationStateBackground)
    {
        resizeFail = true;
        resizeFailRetry = 10;
        return;
    }
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    SceneRendererGLES_iOS *sceneRender = (SceneRendererGLES_iOS *)_renderer;
    sceneRender->useContext();

    // Try to resize the renderer, multiple times if necessary
	if (!sceneRender->resize((int)self.frame.size.width*self.contentScaleFactor,(int)self.frame.size.height*self.contentScaleFactor))
    {
        if (!resizeFail)
        {
            resizeFail = true;
            resizeFailRetry = 10;
        } else
            resizeFailRetry--;
        
        [EAGLContext setCurrentContext:oldContext];

        return;
    }
    resizeFail = false;
    resizeFailRetry = 0;

    [EAGLContext setCurrentContext:oldContext];

    [self drawView:nil];
}


@end
