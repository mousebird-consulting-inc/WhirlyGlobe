/*
 *  MaplyBaseViewController_private.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012-2019 mousebird consulting
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

#import "control/MaplyBaseViewController.h"
#import "MaplyControllerLayer_private.h"
#import "MaplyComponentObject_private.h"
#import "WGInteractionLayer_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyVectorObject_private.h"
#import "MaplyShader_private.h"
#import "MaplyActiveObject_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "visual_objects/MaplyCluster.h"
#import "SMCalloutView.h"
#import "gestures/Maply3dTouchPreviewDelegate.h"
#import "MaplyRenderController_private.h"
#import "ViewPlacementActiveModel.h"
#import "FontTextureManager_iOS.h"
#import "ViewWrapper.h"

@interface MaplyBaseViewController() <SMCalloutViewDelegate>
{
@public
    MaplyRenderController *renderControl;
    
    UIView<WhirlyKitViewWrapper> *wrapView;
        
    // List of views we're tracking for location
    NSMutableArray *viewTrackers;
    
    // List of annotations we're tracking for location
    NSMutableArray *annotations;

    /// View Placement logic used to move annotations around
    WhirlyKit::ViewPlacementActiveModelRef viewPlacementModel;
                
    /// Set if we're dumping out performance output
    bool _performanceOutput;
    
    /// Set while we're trying to track foreground/background
    bool wasAnimating;
    
    /// When an annotation comes up we may want to reposition the view.  This works poorly in some cases.
    bool allowRepositionForAnnnotations;
      
    /// 3dtouch preview context, so we can remove it.
    id <UIViewControllerPreviewing> previewingContext;
  
    /// Need to keep a ref to this because the system keeps a weak ref
    Maply3dTouchPreviewDelegate *previewTouchDelegate;
}

/// This is called by the subclasses.  Don't call it yourself.
- (void) clear;

/// LoadSetup is where the Component does all the WhirlyGlobe/Maply specific setup.  If you override this,
///  be sure to call [super loadSetup] first and then do your thing.
- (void) loadSetup;

/// Create the MetalView
- (void) loadSetup_mtlView;

/// If you have your own WhirlyGlobeView or MaplyView subclass, set it up here
- (WhirlyKit::ViewRef) loadSetup_view;

/// Override this to set up the default lighting scheme (e.g. the shaders).
/// The base class provides an adequate default
- (void) loadSetup_lighting;

/// The base classes fill this in to return their own interaction layer subclass
- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer;

@end
