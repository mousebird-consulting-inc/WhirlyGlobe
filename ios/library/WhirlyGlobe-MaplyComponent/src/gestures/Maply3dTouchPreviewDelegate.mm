//
//  Maply3dTouchDelegate.mm
//  WhirlyGlobeLib
//
//  Created by Jesse Crocker on 10/4/15.
//
//

#import "gestures/Maply3dTouchPreviewDelegate.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyViewController.h"
#import "control/WhirlyGlobeViewController.h"

@interface Maply3dTouchPreviewDelegate () {
    MaplyBaseViewController * _Nonnull __weak viewC;
    MaplyBaseInteractionLayer * __weak interactLayer;
}


@end

using namespace WhirlyKit;

@implementation Maply3dTouchPreviewDelegate

+ (Maply3dTouchPreviewDelegate * _Nonnull)touchDelegate:(MaplyBaseViewController * _Nonnull)maplyViewC
                                          interactLayer:( MaplyBaseInteractionLayer*)interactLayer
                                             datasource:(NSObject<Maply3dTouchPreviewDatasource>* _Nonnull)datasource
{
    Maply3dTouchPreviewDelegate *touchDelegate = [[Maply3dTouchPreviewDelegate alloc] init];
    touchDelegate.datasource = datasource;
    touchDelegate->viewC = maplyViewC;
    touchDelegate->interactLayer = interactLayer;
    return touchDelegate;
}

- (UIViewController * _Nullable)previewingContext:(id<UIViewControllerPreviewing> _Nonnull)previewingContext
                        viewControllerForLocation:(CGPoint)location
{
    const auto __strong vc = viewC;
    const auto __strong layer = interactLayer;
    NSObject *selectedObject = [layer selectLabelsAndMarkerForScreenPoint:location];
    if(!selectedObject)
    {
        if([vc isKindOfClass:[MaplyViewController class]])
        {
            const MaplyCoordinate coord = [(MaplyViewController*)vc geoFromScreenPoint:location];
            selectedObject = [[layer findVectorsInPoint:Point2f(coord.x, coord.y)] firstObject];
        }
        else
        { //globe
            MaplyCoordinate coord;
            if([(WhirlyGlobeViewController*)vc geoPointFromScreen:location geoCoord:&coord])
            {
                selectedObject = [[layer findVectorsInPoint:Point2f(coord.x, coord.y)] firstObject];
            }
        }
    }

    return selectedObject ? [self.datasource maplyViewController:vc
              previewViewControllerForSelection:selectedObject] : nil;
}


- (void)previewingContext:(id<UIViewControllerPreviewing> _Nonnull)previewingContext
     commitViewController:(UIViewController * _Nonnull)viewControllerToCommit
{
    [self.datasource maplyViewController:viewC
               showPreviewViewController:viewControllerToCommit];
}

@end
