//
//  Maply3dTouchDelegate.mm
//  WhirlyGlobeLib
//
//  Created by Jesse Crocker on 10/4/15.
//
//

#import "Maply3dTouchPreviewDelegate.h"
#import <MaplyBaseInteractionLayer_private.h>


@interface Maply3dTouchPreviewDelegate () {
  MaplyBaseViewController * _Nonnull viewC;
  MaplyBaseInteractionLayer * interactLayer;
}


@end

@implementation Maply3dTouchPreviewDelegate

+ (Maply3dTouchPreviewDelegate * _Nonnull)touchDelegate:(MaplyBaseViewController * _Nonnull)maplyViewC
                                   interactLayer:( MaplyBaseInteractionLayer*)interactLayer
                                      datasource:(NSObject<Maply3dTouchPreviewDatasource>* _Nonnull)datasource
{
  Maply3dTouchPreviewDelegate *touchDelegate = [[Maply3dTouchPreviewDelegate alloc] init];
  touchDelegate.datasource = datasource;
  touchDelegate->interactLayer = interactLayer;
  return touchDelegate;
}

- (UIViewController * _Nullable)previewingContext:(id<UIViewControllerPreviewing> _Nonnull)previewingContext
                        viewControllerForLocation:(CGPoint)location
{
  NSObject *selectedObject = [interactLayer selectedObjectForScreenPoint:location];
//  if(!selectedObject) {
//    selectedObject = [interactLayer findVectorsInPoint:<#(id)#>]
//  }
  
  if(!selectedObject) {
    return nil;
  }
  return [self.datasource maplyViewController:viewC
            previewViewControllerForSelection:selectedObject];
}


- (void)previewingContext:(id<UIViewControllerPreviewing> _Nonnull)previewingContext
     commitViewController:(UIViewController * _Nonnull)viewControllerToCommit {
  [self.datasource maplyViewController:viewC
             showPreviewViewController:viewControllerToCommit];
}

@end
