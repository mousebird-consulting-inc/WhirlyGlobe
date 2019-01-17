//
//  Maply3dTouchDelegate.h
//  WhirlyGlobeLib
//
//  Created by Jesse Crocker on 10/4/15.
//
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "Maply3DTouchPreviewDatasource.h"

@class MaplyBaseInteractionLayer;
@class MaplyBaseViewController;

@interface Maply3dTouchPreviewDelegate : NSObject <UIViewControllerPreviewingDelegate>

@property (nonatomic, strong) NSObject<Maply3dTouchPreviewDatasource> * _Nonnull datasource;

/// Create and configure new Maply3dTouchPreviewDelegate, but it is not activated.
+ (Maply3dTouchPreviewDelegate * _Nonnull)touchDelegate:(MaplyBaseViewController * _Nonnull)maplyViewC
                                  interactLayer:( MaplyBaseInteractionLayer* _Nonnull)interactLayer
                             datasource:(NSObject<Maply3dTouchPreviewDatasource>* _Nonnull)datasource;


@end
