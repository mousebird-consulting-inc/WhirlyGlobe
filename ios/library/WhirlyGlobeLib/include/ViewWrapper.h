/*
 *  ViewWrapper.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/15/19.
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
#import "SceneRenderer.h"

/** OpenGL View is a
 base class for implementing an open GL rendering view.
 This is modeled off of the example.  We subclass this for
 our own purposes.
 */
@protocol WhirlyKitViewWrapper

/// Renderer we're using
@property (nonatomic) WhirlyKit::SceneRenderer *renderer;

/// True if we've got a displayLink turned on to animate.
- (BOOL)isAnimating;

/// Start animating.  Typically right before we're displayed
- (void) startAnimation;
/// Stop animating.  It can be restarted or destroyed after this.
- (void) stopAnimation;
/// Destroy the display link.  Cannot be restarted.
- (void) teardown;

@end
