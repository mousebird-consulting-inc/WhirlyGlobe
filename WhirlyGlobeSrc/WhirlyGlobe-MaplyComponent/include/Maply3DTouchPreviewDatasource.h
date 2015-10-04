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
- (UIViewController * _Nullable)maplyViewController:(MaplyBaseViewController * _Nonnull)viewC
                  previewViewControllerForSelection:(NSObject * _Nonnull)selectedObj;

- (void)maplyViewController:(MaplyBaseViewController * _Nonnull)viewC
  showPreviewViewController:(UIViewController * _Nonnull)previewViewC;

@end
