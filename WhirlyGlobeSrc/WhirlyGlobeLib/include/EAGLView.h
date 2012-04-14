/*
 *  WhirlyGlobeEAGLView .h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/5/11.
 *  Copyright 2011 mousebird consulting
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

#import "ESRenderer.h"

/** OpenGL View is a
	base class for implementing an open GL rendering view.
	This is modeled off of the example.  We subclass this for
    our own purposes.
 */
@interface WhirlyKitEAGLView  : UIView 
{
    // We're only expecting this to be set once
	id <WhirlyKitESRenderer> renderer;

    // This is in units of 60/frameRate.  Set it to 4 to get 15 frames/sec (at most)
	NSInteger frameInterval;
    BOOL animating;
    CADisplayLink *displayLink;
}

@property (nonatomic) id<WhirlyKitESRenderer> renderer;
@property (nonatomic) NSInteger frameInterval;
@property (readonly, nonatomic, getter=isAnimating) BOOL animating;

/// Start the animation.  Typically right before we're displayed
- (void) startAnimation;
/// Stop the animation.  It can be restarted or destroyed after this.
- (void) stopAnimation;

/// Draw into the actual view
- (void) drawView:(id)sender;

@end
