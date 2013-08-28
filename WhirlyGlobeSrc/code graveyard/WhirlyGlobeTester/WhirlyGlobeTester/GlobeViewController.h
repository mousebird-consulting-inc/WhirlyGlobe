/*
 *  GlobeViewController.h
 *  WhirlyGlobeTester
 *
 *  Created by Steve Gifford on 10/26/11.
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
#import "WhirlyGlobe.h"
#import "InteractionLayer.h"
#import "OptionsViewController.h"
#import "PanDelegateFixed.h"

// Update the frame rate display this often
static const float FPSUpdateInterval = 4.0;

// Note: You need the mbtiles archive if you're going to turn this on.
//       I've moved it out of the source tree since it's so big.
static const bool UseMBTiles = false;
static const bool UseStamenTiles = false;

/** Globe View Controller
    This class pops up a a view controller with specific
    demo functionality for all the various data layers in WhirlyGlobe.
 */
@interface GlobeViewController : UIViewController <OptionsControllerDelegate,UIPopoverControllerDelegate>
{
    IBOutlet UIView *statsView;
    IBOutlet UILabel *fpsLabel;
    IBOutlet UILabel *drawLabel;
    IBOutlet UILabel *selectLabel;
    
	WhirlyKitEAGLView *glView;
	WhirlyKitSceneRendererES1 *sceneRenderer;
    
   	// Scene, view, and associated data created when controller is up
	WhirlyGlobe::GlobeScene *theScene;
	WhirlyGlobeView *theView;
	WhirlyKitTextureGroup *texGroup;
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitMBTileQuadSource *mbTiles;
    WhirlyKitNetworkTileQuadSource *netTiles;
    
	// Thread used to control Whirly Globe layers
	WhirlyKitLayerThread *layerThread;
	
	// Data layers, readers, and loaders
	WhirlyGlobeSphericalEarthLayer *earthLayer;
    WhirlyKitQuadDisplayLayer *quadLayer;
	WhirlyKitVectorLayer *vectorLayer;
	WhirlyKitLabelLayer *labelLayer;
    WhirlyKitParticleSystemLayer *particleSystemLayer;
    WhirlyKitMarkerLayer *markerLayer;
    WhirlyKitSelectionLayer *selectionLayer;
    WhirlyGlobeLoftLayer *loftLayer;
    InteractionLayer *interactLayer;
    
    // Gesture recognizer delegates
    WhirlyGlobePinchDelegate *pinchDelegate;
    PanDelegateFixed *panDelegate;
    WhirlyGlobeTapDelegate *tapDelegate;
    WhirlyGlobeLongPressDelegate *longPressDelegate;
    WhirlyGlobeRotateDelegate *rotateDelegate;    
    
    UIPopoverController *popoverController;
    OptionsViewController *optionsViewC;
}

/// Use this to create one of these
+ (GlobeViewController *)loadFromNib;

@end
