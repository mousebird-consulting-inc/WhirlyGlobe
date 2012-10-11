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
    }
    
    return self;
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
        
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
        [displayLink setFrameInterval:frameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [displayLink invalidate];
        displayLink = nil;
        animating = FALSE;
    }
}

- (void) drawView:(id)sender
{
    [renderer render:displayLink.duration*displayLink.frameInterval];
}

- (void) setFrame:(CGRect)newFrame
{
	[super setFrame:newFrame];
}

- (void) layoutSubviews
{
	[renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView:nil];
}

@end
