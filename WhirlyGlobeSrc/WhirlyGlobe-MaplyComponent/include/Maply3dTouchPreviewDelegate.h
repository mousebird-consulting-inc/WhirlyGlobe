//
//  Maply3dTouchDelegate.h
//  WhirlyGlobeLib
//
//  Created by Jesse Crocker on 10/4/15.
//
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@class MaplyBaseInteractionLayer;
@class MaplyBaseViewController;

@protocol Maply3dTouchPreviewDatasource <NSObject>

@required
- (UIViewController * _Nullable)maplyViewController:(MaplyBaseViewController * _Nonnull)viewC
                  previewViewControllerForSelection:(NSObject * _Nonnull)selectedObj;

- (void)maplyViewController:(MaplyBaseViewController * _Nonnull)viewC
                          showPreviewViewController:(UIViewController * _Nonnull)previewViewC;

@end

@interface Maply3dTouchPreviewDelegate : NSObject <UIViewControllerPreviewingDelegate>

@property (nonatomic, strong) NSObject<Maply3dTouchPreviewDatasource> * _Nonnull datasource;

+ (Maply3dTouchPreviewDelegate * _Nonnull)touchDelegate:(MaplyBaseViewController * _Nonnull)maplyViewC
                                  interactLayer:( MaplyBaseInteractionLayer* _Nonnull)interactLayer
                             datasource:(NSObject<Maply3dTouchPreviewDatasource>* _Nonnull)datasource;


@end
