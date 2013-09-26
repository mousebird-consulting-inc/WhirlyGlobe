
#import "MaplyRotateDelegate.h"

using namespace WhirlyKit;

@implementation MaplyRotateDelegate
{
    Maply::RotationType rotType;
    MaplyView *mapView;
    double startRotationAngle;
}

using namespace Maply;

- (id)initWithMapView:(MaplyView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
        rotType = RotNone;
	}
	
	return self;
}


+ (MaplyRotateDelegate *)rotateDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
	MaplyRotateDelegate *rotateDelegate = [[MaplyRotateDelegate alloc] initWithMapView:mapView];
	UIRotationGestureRecognizer *rotationRecognizer = [[UIRotationGestureRecognizer alloc] initWithTarget:rotateDelegate action:@selector(rotateAction:)];
    rotationRecognizer.delegate = rotateDelegate;
    [view addGestureRecognizer:rotationRecognizer];
	return rotateDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

- (void)rotateAction:(id)sender
{
    UIRotationGestureRecognizer *rotate = sender;
    
    // Turn off rotation if we fall below two fingers
    if ([rotate numberOfTouches] < 2)
    {
        return;
    }
    switch (rotate.state)
    {
        case UIGestureRecognizerStateBegan:
            [mapView cancelAnimation];
            rotType = RotFree;
            startRotationAngle = [mapView rotAngle];
            break;
        case UIGestureRecognizerStateChanged:
            [mapView cancelAnimation];
            if (rotType == RotFree)
            {
                [mapView setRotAngle:(startRotationAngle + rotate.rotation)];
	        }
            break;
        case UIGestureRecognizerStateFailed:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateEnded:
            rotType = RotNone;
            break;
        default:
            break;
    }
}

@end
