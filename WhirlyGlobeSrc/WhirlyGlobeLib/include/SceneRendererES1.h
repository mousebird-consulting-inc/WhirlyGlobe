/*
 *  SceneRendererES1.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
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

#import "SceneRendererES.h"

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

/// @cond
@class WhirlyKitSceneRendererES1;
/// @endcond

/** Protocol for the scene render callbacks.
    These are all optional, but if set will be called
     at various points within the rendering process.
 */
@protocol WhirlyKitSceneRendererES1Delegate

@optional

/// Return true if the lighting has changed since last lightingSetup: was called
- (BOOL)lightingChanged:(WhirlyKitSceneRendererES1 *)sceneRenderer;

/// This overrides the setup view, including lights and modes
/// Be sure to do *all* the setup if you do this
- (void)lightingSetup:(WhirlyKitSceneRendererES1 *)sceneRenderer;

/// Called right before a frame is rendered
- (void)preFrame:(WhirlyKitSceneRendererES1 *)sceneRenderer;

/// Called right after a frame is rendered
- (void)postFrame:(WhirlyKitSceneRendererES1 *)sceneRenderer;
@end

/** Scene Renderer for OpenGL ES1.
    This implements the actual rendering.  In theory it's
    somewhat composable, but in reality not all that much.
    Just set this up as in the examples and let it run.
 */
@interface WhirlyKitSceneRendererES1 : WhirlyKitSceneRendererES
{
    /// Delegate called at specific points in the rendering process
    NSObject<WhirlyKitSceneRendererES1Delegate> * __weak delegate;
}

@property (nonatomic,weak) NSObject<WhirlyKitSceneRendererES1Delegate> *delegate;

@end
