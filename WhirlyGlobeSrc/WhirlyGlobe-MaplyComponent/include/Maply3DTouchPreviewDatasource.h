//
//  Maply3DTouchPreviewDatasource.h
//  WhirlyGlobe-MaplyComponent
//
//  Created by Jesse Crocker on 10/4/15.
//
//

@class MaplyBaseViewController;

@protocol Maply3dTouchPreviewDatasource <NSObject>

@required
/** @brief Asks the data source for a view controller to display as a preview for a selected object
 @param viewC the map requesting the view controller;
 @param selectedObj The object a preview is being requested for.
 @return a UIViewController, or nil if no preview should be displayed.
 */
- (UIViewController * _Nullable)maplyViewController:(MaplyBaseViewController * _Nonnull)viewC
                  previewViewControllerForSelection:(NSObject * _Nonnull)selectedObj;

/** @brief Asks the data source to present a preview view controller.
 @details the most likely implementation of this is [self show:previewViewC sender:self];
 @param viewC the map requesting the view controller;
 @param previewViewC the view controller to present.
 */
- (void)maplyViewController:(MaplyBaseViewController * _Nonnull)viewC
  showPreviewViewController:(UIViewController * _Nonnull)previewViewC;

@end
