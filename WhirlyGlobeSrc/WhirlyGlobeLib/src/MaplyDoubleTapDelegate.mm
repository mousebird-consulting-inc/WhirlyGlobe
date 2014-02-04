//
//  MaplyDoubleTapDelegate.m
//  WhirlyGlobeLib
//
//  Created by Jesse Crocker on 2/3/14.
//
//

#import "MaplyDoubleTapDelegate.h"

#import "EAGLView.h"
#import "SceneRendererES.h"

using namespace WhirlyKit;

@implementation MaplyDoubleTapDelegate
{
    MaplyView *mapView;
    /// Boundary quad that we're to stay within
    std::vector<WhirlyKit::Point2f> bounds;
}

- (id)initWithMapView:(MaplyView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
        _minZoom = _maxZoom = -1.0;
	}
	
	return self;
}

+ (MaplyDoubleTapDelegate *)doubleTapDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
    MaplyDoubleTapDelegate *tapDelegate = [[MaplyDoubleTapDelegate alloc] initWithMapView:mapView];
    UITapGestureRecognizer *tapGecognizer = [[UITapGestureRecognizer alloc] initWithTarget:tapDelegate action:@selector(tapGesture:)];
    tapGecognizer.numberOfTapsRequired = 2;
    tapGecognizer.numberOfTouchesRequired = 1;
    tapGecognizer.delegate = tapDelegate;
	[view addGestureRecognizer:tapGecognizer];
	return tapDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

- (void)setBounds:(WhirlyKit::Point2f *)inBounds
{
    bounds.clear();
    for (unsigned int ii=0;ii<4;ii++)
        bounds.push_back(inBounds[ii]);
}

// Bounds check on a single point
- (bool)withinBounds:(Point3d &)loc view:(UIView *)view renderer:(WhirlyKitSceneRendererES *)sceneRender
{
    if (bounds.empty())
        return true;
    
    Eigen::Matrix4d fullMatrix = [mapView calcFullMatrix];
    
    // The corners of the view should be within the bounds
    CGPoint corners[4];
    corners[0] = CGPointMake(0,0);
    corners[1] = CGPointMake(view.frame.size.width, 0.0);
    corners[2] = CGPointMake(view.frame.size.width, view.frame.size.height);
    corners[3] = CGPointMake(0.0, view.frame.size.height);
    Point3d planePts[4];
    bool isValid = true;
    for (unsigned int ii=0;ii<4;ii++)
    {
        [mapView pointOnPlaneFromScreen:corners[ii] transform:&fullMatrix
                              frameSize:Point2f(sceneRender.framebufferWidth/view.contentScaleFactor,sceneRender.framebufferHeight/view.contentScaleFactor)
                                    hit:&planePts[ii] clip:false];
        isValid &= PointInPolygon(Point2f(planePts[ii].x(),planePts[ii].y()), bounds);
        //        NSLog(@"plane hit = (%f,%f), isValid = %s",planePts[ii].x(),planePts[ii].y(),(isValid ? "yes" : "no"));
    }
    
    return isValid;
}

// Called for double tap actions
- (void)tapGesture:(id)sender
{
    UITapGestureRecognizer *tap = sender;
    WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)tap.view;
    WhirlyKitSceneRendererES *sceneRenderer = glView.renderer;
	
    Point3d curLoc = mapView.loc;
    double newZ = curLoc.z() - (curLoc.z() - _minZoom)/2.0;
    if (_minZoom >= _maxZoom || (_minZoom < newZ && newZ < _maxZoom))
    {
        [mapView setLoc:Point3d(curLoc.x(),curLoc.y(),newZ)];
        if (![self withinBounds:mapView.loc view:glView renderer:sceneRenderer])
            [mapView setLoc:curLoc];
    }
}


@end
