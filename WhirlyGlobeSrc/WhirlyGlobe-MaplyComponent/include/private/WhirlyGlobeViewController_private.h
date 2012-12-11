/*
 *  WhirlyGlobeViewController_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 10/26/12.
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

#import <UIKit/UIKit.h>
#import <WhirlyGlobe.h>
#import "WhirlyGlobeViewController.h"
#import "MaplyViewControllerLayer_private.h"
#import "MaplyComponentObject_private.h"
#import "WGInteractionLayer_private.h"
#import "PanDelegateFixed.h"
#import "PinchDelegateFixed.h"
#import "WGSphericalEarthWithTexGroup_private.h"
#import "MaplyQuadEarthWithMBTiles_private.h"
#import "MaplyQuadEarthWithRemoteTiles_private.h"

/// This is the private interface to WhirlyGlobeViewController.
/// Only pull this in if you're subclassing
@interface WhirlyGlobeViewController()
{
@public
    WhirlyKitEAGLView *glView;
    WhirlyKitSceneRendererES1 *sceneRenderer;
    
    WhirlyGlobe::GlobeScene *globeScene;
    WhirlyGlobeView *globeView;
    
    WhirlyKitLayerThread *layerThread;
    
    // The standard set of layers we create
    WhirlyKitMarkerLayer *markerLayer;
    WhirlyKitLabelLayer *labelLayer;
    WhirlyKitVectorLayer *vectorLayer;
    WhirlyKitShapeLayer *shapeLayer;
    WhirlyKitSphericalChunkLayer *chunkLayer;
    WhirlyKitLayoutLayer *layoutLayer;
    WhirlyKitSelectionLayer *selectLayer;
    
    // Our own interaction layer does most of the work
    WGInteractionLayer *interactLayer;
    
    // Layers (and associated data) created for the user
    NSMutableArray *userLayers;
    
    // Gesture recognizers
    WGPinchDelegateFixed *pinchDelegate;
    PanDelegateFixed *panDelegate;
    WhirlyGlobeTapDelegate *tapDelegate;
    WhirlyGlobeRotateDelegate *rotateDelegate;
    AnimateViewRotation *animateRotation;
    
    // List of views we're tracking for location
    NSMutableArray *viewTrackers;
    
    // If set we'll look for selectables
    bool selection;
    
    // General rendering and other display hints
    NSDictionary *hints;
    
    // Default description dictionaries for the various data types
    NSDictionary *screenMarkerDesc,*markerDesc,*screenLabelDesc,*labelDesc,*vectorDesc,*shapeDesc,*stickerDesc;
    
    // Clear color we're using
    UIColor *theClearColor;    
}

/// LoadSetup is where the Component does all the WhirlyGlobe specific setup.  If you override this,
///  be sure to call [super loadSetup] first and then do your thing.
- (void) loadSetup;

/// If you have your own WhirlyGlobeView subclass, set it up here
- (void) loadSetup_view;

@end
