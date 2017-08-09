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
/** 
    Asks the data source for a view controller to display as a preview for a selected object
 
    @param viewC the map requesting the view controller;
 
    @param selectedObj The object a preview is being requested for.
 
    @return a UIViewController, or nil if no preview should be displayed.
 */
- (UIViewController * _Nullable)maplyViewController:(MaplyBaseViewController * _Nonnull)viewC
                  previewViewControllerForSelection:(NSObject * _Nonnull)selectedObj;

/** 
    Asks the data source to present a preview view controller.
 
    the most likely implementation of this is [self show:previewViewC sender:self];
 
    @param viewC the map requesting the view controller;
 
    @param previewViewC the view controller to present.
 */
- (void)maplyViewController:(MaplyBaseViewController * _Nonnull)viewC
  showPreviewViewController:(UIViewController * _Nonnull)previewViewC;

@end
