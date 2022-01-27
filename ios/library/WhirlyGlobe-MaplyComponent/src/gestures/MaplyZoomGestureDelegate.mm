/*  MaplyZoomGestureDelegate.mm
 *
 *  Created by Jesse Crocker on 2/4/14.
 *  Copyright 2011-2022 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#import "gestures/MaplyZoomGestureDelegate.h"
#import "private/MaplyZoomGestureDelegate_private.h"
#import "gestures/MaplyPanDelegate.h"
#import "MaplyAnimateTranslation.h"
#import "ViewWrapper.h"

#import "SceneRenderer.h"

using namespace WhirlyKit;

@implementation MaplyZoomGestureDelegate

- (instancetype)initWithMapView:(Maply::MapView_iOSRef)inView
{
	if ((self = [super init]))
	{
		_mapView = inView;
        _minZoom = _maxZoom = -1.0;
	}
	
	return self;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer
        shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {
    return true;
}

- (const WhirlyKit::Point2dVector &)getBounds {
    return bounds;
}

- (void)setBounds:(const WhirlyKit::Point2d *)inBounds
{
    bounds.clear();
    bounds.insert(bounds.end(), &inBounds[0], &inBounds[4]);
}

// Called for double tap actions
- (void)tapGesture:(id)sender {
    UITapGestureRecognizer *tap = sender;
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)tap.view;
    SceneRenderer *sceneRenderer = wrapView.renderer;

    const Point3d curLoc = _mapView->getLoc();
//    NSLog(@"curLoc x:%f y:%f z:%f", curLoc.x(), curLoc.y(), curLoc.z());
    // Just figure out where we tapped
	Point3d hit;
    const Eigen::Matrix4d theTransform = _mapView->calcFullMatrix();
    const CGPoint touchLoc = [tap locationInView:tap.view];
    const Point2f touchLoc2f(touchLoc.x,touchLoc.y);
    const Point2f frameSize = sceneRenderer->getFramebufferSize();
    if (_mapView->pointOnPlaneFromScreen(touchLoc2f, &theTransform, frameSize/wrapView.contentScaleFactor, &hit, true))
    {
        const double newZ = curLoc.z() - (curLoc.z() - _minZoom)/2.0;
        Point2d newCenter;
        if (_minZoom >= _maxZoom || (_minZoom < newZ && newZ < _maxZoom))
        {
            Maply::MapView testMapView(*_mapView);
            testMapView.setLoc(Point3d(hit.x(),hit.y(),newZ));
            Point3d newCenter;
            if (MaplyGestureWithinBounds(bounds,_mapView->getLoc(),sceneRenderer,&testMapView,&newCenter))
            {
                _mapView->setLoc(newCenter);
            }
        }
    } else {
        // Not expecting this case
    }
}

@end
