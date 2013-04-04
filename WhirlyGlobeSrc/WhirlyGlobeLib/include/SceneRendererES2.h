/*
 *  SceneRendererES2.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
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
#import "Lighting.h"

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

/// @cond
@class WhirlyKitSceneRendererES1;
/// @endcond

/** Scene Renderer for OpenGL ES2.
     This implements the actual rendering.  In theory it's
     somewhat composable, but in reality not all that much.
     Just set this up as in the examples and let it run.
 */
@interface WhirlyKitSceneRendererES2 : WhirlyKitSceneRendererES

/// Add a light to the existing set
- (void)addLight:(WhirlyKitDirectionalLight *)light;

/// Replace all the lights at once. nil turns off lighting
- (void)replaceLights:(NSArray *)lights;

/// Set the default material
- (void)setDefaultMaterial:(WhirlyKitMaterial *)mat;

@end
