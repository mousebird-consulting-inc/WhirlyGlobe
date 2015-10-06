//
//  Maply3dTouchDelegate.mm
//  WhirlyGlobeLib
//
//  Created by Jesse Crocker on 10/4/15.
//
//

#import "Maply3dTouchPreviewDelegate.h"
#import "MaplyBaseInteractionLayer_private.h"
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"

@interface Maply3dTouchPreviewDelegate () {
    MaplyBaseViewController * _Nonnull viewC;
    MaplyBaseInteractionLayer * interactLayer;
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
    NSObject *selectedObject = [interactLayer selectLabelsAndMarkerForScreenPoint:location];
    if(!selectedObject)
    {
        if([viewC isKindOfClass:[MaplyViewController class]])
        {
            MaplyCoordinate coord = [(MaplyViewController*)viewC geoFromScreenPoint:location];
            selectedObject = [[interactLayer findVectorsInPoint:Point2f(coord.x, coord.y)] firstObject];
        }
        else
        { //globe
            MaplyCoordinate coord;
            if([(WhirlyGlobeViewController*)viewC geoPointFromScreen:location geoCoord:&coord])
            {
                selectedObject = [[interactLayer findVectorsInPoint:Point2f(coord.x, coord.y)] firstObject];
            }
        }
    }
    
    if(!selectedObject)
    {
        return nil;
    }
    return [self.datasource maplyViewController:viewC
              previewViewControllerForSelection:selectedObject];
}


- (void)previewingContext:(id<UIViewControllerPreviewing> _Nonnull)previewingContext
     commitViewController:(UIViewController * _Nonnull)viewControllerToCommit
{
    [self.datasource maplyViewController:viewC
               showPreviewViewController:viewControllerToCommit];
}

@end
