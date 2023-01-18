/*
 *  MaplyBaseViewController_private.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012-2022 mousebird consulting
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
#import "WGInteractionLayer_private.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyShader_private.h"
#import "MaplyActiveObject_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyRenderController_private.h"
#import "ViewPlacementActiveModel.h"
#import "ViewWrapper.h"
#import "SMCalloutView.h"

#if !MAPLY_MINIMAL
# import "MaplyComponentObject_private.h"
# import "MaplyVectorObject_private.h"
# import "visual_objects/MaplyCluster.h"
# import "gestures/Maply3dTouchPreviewDelegate.h"
# import "FontTextureManager_iOS.h"
#endif //!MAPLY_MINIMAL


@interface MaplyBaseViewController() <SMCalloutViewDelegate, ViewWrapperDelegateProtocol>
{
@public
    MaplyRenderController *renderControl;
    
    UIView<WhirlyKitViewWrapper> *wrapView;
        
#if !MAPLY_MINIMAL
    // List of views we're tracking for location
    NSMutableArray *viewTrackers;
    
    // List of annotations we're tracking for location
    NSMutableArray *annotations;
    
    /// View Placement logic used to move annotations around
    WhirlyKit::ViewPlacementActiveModelRef viewPlacementModel;
#endif //!MAPLY_MINIMAL

    /// Set if we're dumping out performance output
    bool _performanceOutput;
    
    /// Set while we're trying to track foreground/background
    bool wasAnimating;
    
    /// When an annotation comes up we may want to reposition the view.  This works poorly in some cases.
    bool allowRepositionForAnnnotations;
      
    /// 3dtouch preview context, so we can remove it.
    id <UIViewControllerPreviewing> previewingContext;
  
#if !MAPLY_MINIMAL
    /// Need to keep a ref to this because the system keeps a weak ref
    Maply3dTouchPreviewDelegate *previewTouchDelegate;
#endif //!MAPLY_MINIMAL
}

/// Indicates when the view position has been explicitly set since the last frame.
@property bool posChanged;
/// Indicates when the view height has been explicitly set since the last frame.
@property bool heightChanged;

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

/// Called when starting gesture, animation
- (void)handleStartMoving:(bool)userMotion;

/// Called when ending gesture, animation
- (void)handleStopMoving:(bool)userMotion;

- (void)report:(NSString * __nonnull)tag error:(NSError * __nonnull)error;
- (void)report:(NSString * __nonnull)tag exception:(NSException * __nonnull)error;

@end
