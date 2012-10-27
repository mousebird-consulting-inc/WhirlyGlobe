/*
 *  WhirlyGlobeSimpleViewController.h
 *  WhirlyGlobeSimple
 *
 *  Created by Steve Gifford on 6/1/11.
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


@interface WhirlyGlobeSimpleViewController : UIViewController 
{
	WhirlyKitEAGLView *glView;
	WhirlyKitSceneRendererES1 *sceneRenderer;
        
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
    WhirlyKitParticleSystemLayer *particleSystemLayer;
    WhirlyKitMarkerLayer *markerLayer;

    // Gesture recognizer delegates
    WhirlyGlobePinchDelegate *pinchDelegate;
    WhirlyGlobePanDelegate *panDelegate;
    WhirlyGlobeTapDelegate *tapDelegate;
    WhirlyGlobeLongPressDelegate *longPressDelegate;
    WhirlyGlobeRotateDelegate *rotateDelegate;
    WhirlyKit::GeoCoordSystem geoCoordSystem;
}

@end
