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
#import <WhirlyGlobe/WhirlyGlobe.h>
#import "InteractionLayer.h"
#import "OptionsViewController.h"
#import "PanDelegateFixed.h"

// Update the frame rate display this often
static const float FPSUpdateInterval = 4.0;

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
    
	EAGLView *glView;
	SceneRendererES1 *sceneRenderer;
    
   	// Scene, view, and associated data created when controller is up
	WhirlyGlobe::GlobeScene *theScene;
	WhirlyGlobeView *theView;
	TextureGroup *texGroup;
    
	// Thread used to control Whirly Globe layers
	WhirlyGlobeLayerThread *layerThread;
	
	// Data layers, readers, and loaders
	SphericalEarthLayer *earthLayer;
	VectorLayer *vectorLayer;
	LabelLayer *labelLayer;
    ParticleSystemLayer *particleSystemLayer;
    WGMarkerLayer *markerLayer;
    WGSelectionLayer *selectionLayer;
    WGLoftLayer *loftLayer;
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
