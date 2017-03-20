/*
 *  WhirlyGlobeAppViewController.h
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 1/12/11.
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
#import "PanDelegateFixed.h"
#import "WebViewController.h"
#import "OptionsViewController.h"

// Update the frame rate display this much
static const float FPSUpdateInterval = 4.0;

/* Whirly Globe View Controller
	View controller that pops up a Whirly Globe view.
 */
@interface WhirlyGlobeAppViewController : UIViewController <UIPopoverControllerDelegate, 
        OptionsViewControllerDelegate, WhirlyKitSceneRendererDelegate>
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
    WhirlyGlobeLoftLayer *loftLayer;
	InteractionLayer *interactLayer;
    
    UIPopoverController *popOverController;	
	NSString *tmpURLString; // used by Web View
	NSString *tmpTitleString;  // used by Web View
    
    IBOutlet UIView *mainView;
    IBOutlet UILabel *label;
    
    // Popover for configuring data
    IBOutlet UIButton *buttonOpenPopOver; // we are using a UIButton without a bar so it is cleaner and we can focus on the globe.
        
    OptionsViewController *optionsViewController;

}

@property (nonatomic, retain) IBOutlet UIView *mainView;
@property (nonatomic, retain) IBOutlet UIPopoverController *popOverController;
@property (nonatomic, retain) IBOutlet NSString *tmpURLString;
@property (nonatomic, retain) IBOutlet NSString *tmpTitleString;

@property (nonatomic, retain) UILabel *label;
@property (nonatomic, retain) IBOutlet UIButton *buttonOpenPopOver;
@property (nonatomic, retain) OptionsViewController *optionsViewController;

-(IBAction)togglePopOverController:(id)sender;
-(IBAction)showAboutUsWebController:(id)sender;


@end

