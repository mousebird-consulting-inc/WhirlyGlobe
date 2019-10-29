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

#import "control/MaplyRenderController.h"
#import "MaplyComponentObject_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyVectorObject_private.h"
#import "MaplyShader_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyQuadSampler_private.h"
#import "SceneRendererGLES_iOS.h"

@class MaplyBaseInteractionLayer;

/**
    The Render Controller is a standalone WhirlyGlobe-Maply renderer.
 
    This Render Controller is used for offline rendering.
  */
@interface MaplyRenderController()<WhirlyKitSnapshot>
{
@public
    /// A pointer to the 3D view.  The subclasses are keeping points with the right subclass.
    WhirlyKit::ViewRef visualView;

    // OpenGL or Metal
    WhirlyKit::SceneRenderer::Type renderType;
    
    // Scene renderer... renders the scene
    WhirlyKit::SceneRendererRef sceneRenderer;
    
    // General pointer to the view
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;

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
        
    /// Used to be screen objects were always drawn last.  Now that's optional.
    int screenDrawPriorityOffset;
    
    /// The thread this render controller started on.  Usually it'll be the main thread.
    NSThread * __weak mainThread;
    
    WhirlyKitLayerThread *baseLayerThread;
    WhirlyKitLayoutLayer *layoutLayer;
    NSMutableArray *layerThreads;

    // Layers (and associated data) created for the user
    NSMutableArray *userLayers;
    
    /// Active models
    NSMutableArray *activeObjects;
    
    /// The default cluster generator (group 0)
    MaplyBasicClusterGenerator *defaultClusterGenerator;
    
    /// Used to render font glyphs on this platform
    WhirlyKit::FontTextureManager_iOSRef fontTexManager;
    
    /// Current draw priority if we're assigning them ourselves
    int layerDrawPriority;

    /// Shared sampling layers (used for loaders)
    std::vector<MaplyQuadSamplingLayer *> samplingLayers;
    
    /// Shared tile fetcher used by default for loaders
    std::vector<MaplyRemoteTileFetcher *> tileFetchers;

    /// Number of simultaneous tile fetcher connections (per tile fetcher)
    int tileFetcherConnections;
}

- (int)screenObjectDrawPriorityOffset;
- (void)setScreenObjectDrawPriorityOffset:(int)newPriority;

// Setup
- (void)loadSetup;
- (void)setupShaders;

// Used in shutting down controls
- (void)teardown;
- (void)clear;

- (void) useGLContext;

- (void)updateLights;

/// Called internally to mark a block of work being done
- (bool) startOfWork;

/// Called internally to end a block of work being done
- (void) endOfWork;

/// Look for a sampling layer that matches the given parameters
/// We'll also keep it around until the user lets us know we're done
- (MaplyQuadSamplingLayer *)findSamplingLayer:(const WhirlyKit::SamplingParams &)params forUser:(WhirlyKit::QuadTileBuilderDelegateRef)userObj;

/// The given user object is done with the given sampling layer.  So we may shut it down.
- (void)releaseSamplingLayer:(MaplyQuadSamplingLayer *)layer forUser:(WhirlyKit::QuadTileBuilderDelegateRef)userObj;

// Used for setup by the view controllers
- (void)loadSetup_scene:(MaplyBaseInteractionLayer *)newInteractLayer;
- (void)loadSetup_view:(WhirlyKit::ViewRef)view;

@end
