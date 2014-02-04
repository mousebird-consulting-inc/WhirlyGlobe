//
//  MaplyDoubleTapDelegate.h
//  WhirlyGlobeLib
//
//  Created by Jesse Crocker on 2/3/14.
//
//

#import <Foundation/Foundation.h>
#import "MaplyView.h"

@interface MaplyDoubleTapDelegate : NSObject <UIGestureRecognizerDelegate>

/// Minimum allowable zoom level
@property (nonatomic,assign) float minZoom;
/// Maximum allowable zoom level
@property (nonatomic,assign) float maxZoom;
//The gesture recognizer
@property (nonatomic,strong) UIGestureRecognizer *gestureRecognizer;

/// Create a pinch gesture and a delegate and wire them up to the given UIView
+ (MaplyDoubleTapDelegate *)doubleTapDelegateForView:(UIView *)view mapView:(MaplyView *)mapView;

/// Set the bounding rectangle
- (void)setBounds:(WhirlyKit::Point2f *)bounds;

@end
