//
//  MapViewController.h
//  WhirlyMapWorld
//
//  Created by Steve Gifford on 1/9/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <WhirlyGlobe/WhirlyGlobe.h>
#import "InteractionLayer.h"

@interface MapViewController : UIViewController
{
    WhirlyKit::CoordSystem *coordSys;
    
    WhirlyGlobeEAGLView *glView;
	WhirlyGlobeSceneRendererES1 *sceneRenderer;

   	// Scene, view, and associated data created when controller is up
	WhirlyGlobe::GlobeScene *theScene;
	WhirlyMapView *theView;
    
	// Thread used to control Whirly Globe layers
	WhirlyGlobeLayerThread *layerThread;

    // Data layers
	WhirlyGlobeVectorLayer *vectorLayer;
	WhirlyGlobeLabelLayer *labelLayer;
    WhirlyGlobeLoftLayer *loftLayer;
    InteractionLayer *interactLayer;
    
    // Gesture recognizer delegates
    WhirlyMapPinchDelegate *pinchDelegate;
    WhirlyMapPanDelegate *panDelegate;
    WhirlyMapTapDelegate *tapDelegate;
}

+ (MapViewController *)loadFromNib;

@end
