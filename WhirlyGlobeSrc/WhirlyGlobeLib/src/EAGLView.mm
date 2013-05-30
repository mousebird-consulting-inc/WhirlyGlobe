/*
 *  EAGLView.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/5/11.
 *  Copyright 2011-2012 mousebird consulting
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
    bool resizeFail;
    int resizeFailRetry;
}

@synthesize renderer;
@synthesize animating;
@synthesize useRetina;

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
		
		animating = FALSE;
		frameInterval = 1;
        self.useRetina = TRUE;
        resizeFail = false;
        resizeFailRetry = 0;
    }
    
    return self;
}

- (void)dealloc
{
    [displayLink invalidate];
}

- (void)setUseRetina:(BOOL)newVal
{
    useRetina = newVal;

    // Either use the device scale or just 1.0
    float scale = [UIScreen mainScreen].scale;
    if (!useRetina)
            scale = 1.0;
    self.contentScaleFactor = scale;
    self.layer.contentsScale = scale;
}


- (NSInteger)frameInterval
{
    return frameInterval;
}

- (void)setFrameInterval:(NSInteger)newFrameInterval
{
    if (newFrameInterval >= 1)
    {
        frameInterval = newFrameInterval;
        [displayLink setFrameInterval:frameInterval];
    }
}

- (void)startAnimation
{
    if (!animating)
    {
        if (!displayLink)
        {
            displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
            [displayLink setFrameInterval:frameInterval];
            if (_reactiveMode)
                [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            else
                [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];            
        } else
            displayLink.paused = NO;
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        displayLink.paused = YES;
        animating = FALSE;
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

    [renderer render:displayLink.duration*displayLink.frameInterval];
}

- (void) setFrame:(CGRect)newFrame
{
	[super setFrame:newFrame];
}

- (void) layoutSubviews
{
    // Try to resize the renderer, multiple times if necessary
	if (![renderer resizeFromLayer:(CAEAGLLayer*)self.layer])
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

	[renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView:nil];
}


@end
