/*
 *  MaplyRenderController_private.h
 *  WhirlyGlobeMaplyComponent
 *
 *  Created by Stephen Gifford on 1/19/18.
 *  Copyright 2012-2018 Saildrone Inc.
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

#import "MaplyRenderController.h"
#import "MaplyComponentObject_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyVectorObject_private.h"
#import "MaplyShader_private.h"
#import "MaplyCoordinateSystem_private.h"

@class MaplyBaseInteractionLayer;

/**
    The Render Controller is a standalone WhirlyGlobe-Maply renderer.
 
    This Render Controller is used for offline rendering.
  */
@interface MaplyRenderController()
{
@public
    // Scene renderer... renders the scene
    WhirlyKitSceneRendererES3 *sceneRenderer;
    
    // Our own interaction layer does most of the work
    MaplyBaseInteractionLayer *interactLayer;

    // General rendering and other display hints
    NSDictionary *hints;
    
    // Clear color we're using
    UIColor *theClearColor;
    
    /// Pointer to the scene.  The subclasses are keeping pointers with their specific subclass.
    WhirlyKit::Scene *scene;

    /// Active lights
    NSMutableArray *lights;
    
    /// Active shaders
    NSMutableArray *shaders;
    
    /// Used to be screen objects were always drawn last.  Now that's optional.
    int screenDrawPriorityOffset;
}

- (int)screenObjectDrawPriorityOffset;
- (void)setScreenObjectDrawPriorityOffset:(int)newPriority;

// Setup
- (void)loadSetup;

// Used in shutting down controls
- (void)teardown;
- (void)clear;

- (void) useGLContext;

- (void)updateLights;

/// Called internally to mark a block of work being done
- (bool) startOfWork;

/// Called internally to end a block of work being done
- (void) endOfWork;

@end
