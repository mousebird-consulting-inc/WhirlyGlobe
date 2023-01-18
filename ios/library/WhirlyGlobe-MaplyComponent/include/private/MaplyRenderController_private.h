/*  MaplyRenderController_private.h
 *  WhirlyGlobeMaplyComponent
 *
 *  Created by Stephen Gifford on 1/19/18.
 *  Copyright 2012-2022 Saildrone Inc.
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
 */

#import "control/MaplyRenderController.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyShader_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyQuadSampler_private.h"
#import "ProgramMTL.h"

#if !MAPLY_MINIMAL
# import "MaplyComponentObject_private.h"
# import "MaplyVectorObject_private.h"
#endif //!MAPLY_MINIMAL


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
#if !MAPLY_MINIMAL
    WhirlyKitLayoutLayer *layoutLayer;
#endif //!MAPLY_MINIMAL
    NSMutableArray *layerThreads;

    // Layers (and associated data) created for the user
    NSMutableArray *userLayers;
    
    /// Active models
    NSMutableArray *activeObjects;
    
#if !MAPLY_MINIMAL
    /// The default cluster generator (group 0)
    MaplyBasicClusterGenerator *defaultClusterGenerator;
    
    /// Used to render font glyphs on this platform
    WhirlyKit::FontTextureManager_iOSRef fontTexManager;
#endif //!MAPLY_MINIMAL

    /// Current draw priority if we're assigning them ourselves
    int layerDrawPriority;

    /// Shared sampling layers (used for loaders)
    std::vector<MaplyQuadSamplingLayer *> samplingLayers;
    
    /// Shared tile fetcher used by default for loaders
    std::vector<MaplyRemoteTileFetcher *> tileFetchers;
    
    // Used for masking features against each other
    MaplyTexture *maskTex;
    MaplyRenderTarget *maskRenderTarget;
}

- (int)screenObjectDrawPriorityOffset;
- (void)setScreenObjectDrawPriorityOffset:(int)newPriority;

// Setup
- (void)loadSetup;
- (void)setupShaders;

// Used in shutting down controls
- (void)teardown;
- (void)clear;

- (void)updateLights;

/// Look for a sampling layer that matches the given parameters
/// We'll also keep it around until the user lets us know we're done
- (MaplyQuadSamplingLayer * _Nullable)findSamplingLayer:(const WhirlyKit::SamplingParams &)params forUser:(WhirlyKit::QuadTileBuilderDelegateRef)userObj;

/// The given user object is done with the given sampling layer.  So we may shut it down.
- (void)releaseSamplingLayer:(MaplyQuadSamplingLayer * _Nonnull)layer forUser:(WhirlyKit::QuadTileBuilderDelegateRef)userObj;

// Used for setup by the view controllers
- (void)loadSetup_scene:(MaplyBaseInteractionLayer * _Nonnull)newInteractLayer;
- (void)loadSetup_view:(WhirlyKit::ViewRef)view;

// Version of remove objects that takes raw IDs
- (void)removeObjectsByID:(const WhirlyKit::SimpleIDSet &)compObjIDs mode:(MaplyThreadMode)threadMode;

- (void)addShader:(NSString * _Nonnull)inName program:(WhirlyKit::ProgramRef)program;

- (MaplyComponentObject * _Nullable)addShapes:(NSArray * _Nonnull)shapes info:(WhirlyKit::ShapeInfo &)shapeInfo desc:(NSDictionary* _Nullable)desc mode:(MaplyThreadMode)threadMode;

- (void)report:(NSString * __nonnull)tag error:(NSError * __nonnull)error;
- (void)report:(NSString * __nonnull)tag exception:(NSException * __nonnull)error;

@end

