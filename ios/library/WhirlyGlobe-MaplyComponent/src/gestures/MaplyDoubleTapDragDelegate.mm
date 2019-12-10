/*
 *  MaplyDoubleTapDragDelegate.mm
 *
 *
 *  Created by Steve Gifford on 2/7/14.
 *  Copyright 2011-2019 mousebird consulting
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
 *
 */

#import <Foundation/Foundation.h>
#import "gestures/MaplyZoomGestureDelegate.h"
#import "MaplyZoomGestureDelegate_private.h"
#import "gestures/MaplyDoubleTapDragDelegate.h"
#import "MaplyAnimateTranslation.h"
#import "ViewWrapper.h"

using namespace WhirlyKit;
using namespace Maply;

@implementation MaplyDoubleTapDragDelegate
{
    CGPoint screenPt;
    float startZ;
}

+ (MaplyDoubleTapDragDelegate *)doubleTapDragDelegateForView:(UIView *)view mapView:(MapView_iOSRef)mapView;
{
    MaplyDoubleTapDragDelegate *pressDelegate = [[MaplyDoubleTapDragDelegate alloc] initWithMapView:mapView];
    UILongPressGestureRecognizer *pressRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:pressDelegate action:@selector(pressGesture:)];
    pressRecognizer.numberOfTapsRequired = 1;
    pressRecognizer.minimumPressDuration = 0.1;
    pressRecognizer.delegate = pressDelegate;
    pressDelegate.gestureRecognizer = pressRecognizer;
	[view addGestureRecognizer:pressRecognizer];
	return pressDelegate;
}

// Nothing else can be running
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return FALSE;
}

// Called for double tap actions
- (void)pressGesture:(id)sender
{
    UILongPressGestureRecognizer *press = sender;
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)press.view;
    SceneRenderer *sceneRenderer = wrapView.renderer;

	switch (press.state)
    {
        case UIGestureRecognizerStateBegan:
            screenPt = [press locationInView:wrapView];
            startZ = self.mapView->getLoc().z();
            self.mapView->cancelAnimation();
            [[NSNotificationCenter defaultCenter] postNotificationName:kMaplyDoubleTapDragDidStart object:self.mapView->tag];
            break;
        case UIGestureRecognizerStateFailed:
            [[NSNotificationCenter defaultCenter] postNotificationName:kMaplyDoubleTapDragDidEnd object:self.mapView->tag];
            break;
        case UIGestureRecognizerStateChanged:
        {
            Point3d curLoc = self.mapView->getLoc();
            CGPoint curPt = [press locationInView:wrapView];
            float diffY = screenPt.y-curPt.y;
            float height = sceneRenderer->getFramebufferSizeScaled().y();
            float scale = powf(2.0,2*diffY/(height/2));
            float newZ = startZ * scale;
            Point2d newCenter;
            if (self.minZoom >= self.maxZoom || (self.minZoom < newZ && newZ < self.maxZoom))
            {
                MapView_iOS testMapView(*(self.mapView));
                Point3d newLoc(curLoc.x(),curLoc.y(),newZ);
                Point3d newCenter;
                // Check if we're still within bounds
                
                if (MaplyGestureWithinBounds(bounds,newLoc,sceneRenderer,&testMapView,&newCenter))
                {
                    newLoc = newCenter;
                    self.mapView->setLoc(newLoc);
                }
            }
        }
            break;
        case UIGestureRecognizerStateEnded:
            [[NSNotificationCenter defaultCenter] postNotificationName:kMaplyDoubleTapDragDidEnd object:self.mapView->tag];
            break;
        default:
            break;
    }
}

@end
