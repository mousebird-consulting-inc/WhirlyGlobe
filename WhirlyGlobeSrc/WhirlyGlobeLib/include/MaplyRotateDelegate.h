
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "MaplyView.h"

namespace Maply
{
    /// The state of our rotation
    typedef enum {RotNone,RotFree} RotationType;
}

@interface MaplyRotateDelegate : NSObject <UIGestureRecognizerDelegate>

+ (MaplyRotateDelegate *)rotateDelegateForView:(UIView *)view mapView:(MaplyView *)mapView;

@end
