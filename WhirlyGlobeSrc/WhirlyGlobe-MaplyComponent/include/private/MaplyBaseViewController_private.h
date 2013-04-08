/*
 *  MaplyBaseViewController_private.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012 mousebird consulting
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

#import "MaplyBaseViewController.h"
#import "MaplyViewControllerLayer_private.h"
#import "MaplyComponentObject_private.h"
#import "WGInteractionLayer_private.h"
#import "PanDelegateFixed.h"
#import "PinchDelegateFixed.h"
#import "MaplyQuadEarthWithMBTiles_private.h"
#import "MaplyQuadEarthWithRemoteTiles_private.h"
#import "MaplySphericalQuadEarthWithTexGroup_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyVectorObject_private.h"
#import "MaplyShader_private.h"
#import "MaplyQuadTestLayer_private.h"

@interface MaplyBaseViewController()
{
@public
    WhirlyKitEAGLView *glView;
    WhirlyKitSceneRendererES *sceneRenderer;
    
    WhirlyKitLayerThread *layerThread;

    // The standard set of layers we create
    WhirlyKitMarkerLayer *markerLayer;
    WhirlyKitLabelLayer *labelLayer;
    WhirlyKitVectorLayer *vectorLayer;
    WhirlyKitShapeLayer *shapeLayer;
    WhirlyKitSphericalChunkLayer *chunkLayer;
    WhirlyKitLayoutLayer *layoutLayer;
    WhirlyKitSelectionLayer *selectLayer;
    WhirlyKitLoftLayer *loftLayer;
    
    // Our own interaction layer does most of the work
    MaplyBaseInteractionLayer *interactLayer;

    // Layers (and associated data) created for the user
    NSMutableArray *userLayers;
    
    // List of views we're tracking for location
    NSMutableArray *viewTrackers;
    
    // If set we'll look for selectables
    bool selection;
    
    // General rendering and other display hints
    NSDictionary *hints;
    
    // Default description dictionaries for the various data types
    NSDictionary *screenMarkerDesc,*markerDesc,*screenLabelDesc,*labelDesc,*vectorDesc,*shapeDesc,*stickerDesc,*loftDesc;
    
    // Clear color we're using
    UIColor *theClearColor;
    
    /// Pointer to the scene.  The subclasses are keeping pointers with their specific subclass.
    WhirlyKit::Scene *scene;
    
    /// A pointer to the 3D view.  The subclasses are keeping points with the right subclass.
    WhirlyKitView *visualView;
    
    /// Active lights
    NSMutableArray *lights;
    
    /// Active shaders
    NSMutableArray *shaders;
    
    /// Set if we're doing performance output
    bool perfOutput;
}

/// This is called by the subclasses.  Don't call it yourself.
- (void) clear;

/// LoadSetup is where the Component does all the WhirlyGlobe/Maply specific setup.  If you override this,
///  be sure to call [super loadSetup] first and then do your thing.
- (void) loadSetup;

/// Create the EAGLView
- (void) loadSetup_glView;

/// If you have your own WhirlyGlobeView or MaplyView subclass, set it up here
- (WhirlyKitView *) loadSetup_view;

/// For loading the Maply or Globe view.  The subclasses call this, but you shouldn't
- (WhirlyKit::Scene *) loadSetup_scene;

/// Override this to set up the default lighting scheme (e.g. the shaders).
/// The base class provides an adequate default
- (void) loadSetup_lighting;

/// The base classes fill this in to return their own interaction layer subclass
- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer;

/// Make the renderer's GL context active.  This is used internally.
- (void) useGLContext;

/// Every shader created with a view controller needs to be tracked by the view controller
- (void) addShader:(MaplyShader *)shader;

@end
