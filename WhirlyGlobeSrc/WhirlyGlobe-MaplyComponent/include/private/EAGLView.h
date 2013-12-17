/*
 *  WhirlyGlobeEAGLView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/5/11.
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

#import <UIKit/UIKit.h>

#import "SceneRenderer_private.h"

/** OpenGL View is a
	base class for implementing an open GL rendering view.
	This is modeled off of the example.  We subclass this for
    our own purposes.
 */
@interface WhirlyKitEAGLView  : UIView 

/// We're only expecting this to be set once
@property (nonatomic) WhirlyKit::MaplySceneRendererES2 *renderer;
/// This is in units of 60/frameRate.  Set it to 4 to get 15 frames/sec (at most)
@property (nonatomic) NSInteger frameInterval;
/// True if we've got a displayLink turned on to animate.
@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
/// Set this false if you don't want the doubling for the retina display.
/// True by default.
@property (nonatomic, assign) BOOL useRetina;
/// If set, we'll expect to respond to outside calls to drawView: and our own
///  display link will be less aggressive.  This works well with UIScrollView.
@property (nonatomic) bool reactiveMode;

/// Start animating.  Typically right before we're displayed
- (void) startAnimation;
/// Stop animating.  It can be restarted or destroyed after this.
- (void) stopAnimation;

/// Draw into the actual view
- (void) drawView:(id)sender;

@end
