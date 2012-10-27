//
//  MapViewController.h
//  WhirlyMapWorld
//
//  Created by Steve Gifford on 1/9/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "WhirlyGlobe.h"
#import "InteractionLayer.h"

@interface MapViewController : UIViewController
{
    WhirlyKit::CoordSystem *coordSys;
    
    WhirlyKitEAGLView *glView;
	WhirlyKitSceneRendererES1 *sceneRenderer;

   	// Scene, view, and associated data created when controller is up
	WhirlyGlobe::GlobeScene *theScene;
	WhirlyGlobeView *theView;
    
	// Thread used to control Whirly Globe layers
	WhirlyKitLayerThread *layerThread;

    // Data layers
	WhirlyKitVectorLayer *vectorLayer;
	WhirlyKitLabelLayer *labelLayer;
    WhirlyGlobeLoftLayer *loftLayer;
    InteractionLayer *interactLayer;
    
    // Gesture recognizer delegates
    WhirlyGlobePinchDelegate *pinchDelegate;
    WhirlyGlobePanDelegate *panDelegate;
    WhirlyGlobeTapDelegate *tapDelegate;
    WhirlyKit::GeoCoordSystem geoCoordSystem;
}

+ (MapViewController *)loadFromNib;

@end
