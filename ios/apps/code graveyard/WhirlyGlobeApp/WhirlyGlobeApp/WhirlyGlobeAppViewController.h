/*
 *  WhirlyGlobeAppViewController.h
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 1/12/11.
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
#import "WhirlyGlobe.h"
#import "InteractionLayer.h"
#import "PanDelegateFixed.h"

// Update the frame rate display this much
static const float FPSUpdateInterval = 4.0;

// If set, we'll turn on a grid layer
static const bool UseGridLayer = false;

/* Whirly Globe View Controller
	View controller that pops up a Whirly Globe view.
 */
@interface WhirlyGlobeAppViewController : UIViewController
{
	WhirlyKitEAGLView *glView;
	WhirlyKitSceneRendererES1 *sceneRenderer;
	
	UILabel *fpsLabel;
	UILabel *drawLabel;

	// Various interaction delegates when this view controller is up
	WhirlyGlobePinchDelegate *pinchDelegate;
	WhirlyGlobeSwipeDelegate *swipeDelegate;
	PanDelegateFixed *panDelegate;
	WhirlyGlobeTapDelegate *tapDelegate;
    WhirlyGlobeLongPressDelegate *pressDelegate;
    WhirlyGlobeRotateDelegate *rotateDelegate;

	// Scene, view, and associated data created when controller is up
	WhirlyGlobe::GlobeScene *theScene;
	WhirlyGlobeView *theView;
	WhirlyKitTextureGroup *texGroup;
	
	// Thread used to control Whirly Globe layers
	WhirlyKitLayerThread *layerThread;
	
	// Data layers, readers, and loaders
	WhirlyGlobeSphericalEarthLayer *earthLayer;
	WhirlyKitVectorLayer *vectorLayer;
	WhirlyKitLabelLayer *labelLayer;
    WhirlyKitGridLayer *gridLayer;
	InteractionLayer *interactLayer;
    
    WhirlyKit::GeoCoordSystem geoCoordSystem;
}

@end

