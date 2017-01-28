/*
 *  EAGLView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/5/11.
 *  Copyright 2011-2015 mousebird consulting
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
    
    if (resizeFail)
        [self layoutSubviews];

    if (_animating)
        [_renderer render:displayLink.duration*displayLink.frameInterval];
    else
        [_renderer processScene];
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
    
    // Try to resize the renderer, multiple times if necessary
	if (![_renderer resizeFromLayer:(CAEAGLLayer*)self.layer])
    {
        if (!resizeFail)
        {
            resizeFail = true;
            resizeFailRetry = 10;
        } else
            resizeFailRetry--;
        
        return;
    }
    resizeFail = false;
    resizeFailRetry = 0;

    [self drawView:nil];
}



- (void) pause {
    if(displayLink)
    {
        displayLink.paused = YES;
    }
}


- (void) unpause {
    if(displayLink)
    {
        displayLink.paused = NO;
    }
}


@end
