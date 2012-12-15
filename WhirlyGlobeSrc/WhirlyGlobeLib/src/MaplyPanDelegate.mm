/*
 *  MaplyPanDelegateMap.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import "EAGLView.h"
#import "SceneRendererES.h"
#import "MaplyPanDelegate.h"
#import "MaplyAnimateTranslateMomentum.h"

using namespace WhirlyKit;

@interface MaplyPanDelegate()
{
    MaplyAnimateTranslateMomentum *translateDelegate;
}
@end

@implementation MaplyPanDelegate

- (id)initWithMapView:(MaplyView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
	}
	
	return self;
}

+ (MaplyPanDelegate *)panDelegateForView:(UIView *)view mapView:(MaplyView *)mapView
{
	MaplyPanDelegate *panDelegate = [[MaplyPanDelegate alloc] initWithMapView:mapView];
	[view addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:panDelegate action:@selector(panAction:)]];
	return panDelegate;
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
- (bool)withinBounds:(Point3f &)loc
{
    if (bounds.empty())
        return true;
    
    return PointInPolygon(Point2f(loc.x(),loc.y()), bounds);
}

// How long we'll animate the gesture ending
static const float AnimLen = 1.0;

// Called for pan actions
- (void)panAction:(id)sender
{
    UIPanGestureRecognizer *pan = sender;
	WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)pan.view;
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;

    if (pan.numberOfTouches > 1)
    {
        panning = NO;
        return;
    }
    
    switch (pan.state)
    {
        case UIGestureRecognizerStateBegan:
        {
            [mapView cancelAnimation];
            
            // Save where we touched
            startTransform = [mapView calcFullMatrix];
            [mapView pointOnPlaneFromScreen:[pan locationOfTouch:0 inView:pan.view] transform:&startTransform
                                  frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight)
                                        hit:&startOnPlane clip:false];
            startLoc = [mapView loc];
            panning = YES;
        }
            break;
        case UIGestureRecognizerStateChanged:
        {
            if (panning)
            {
                [mapView cancelAnimation];
                
                // Figure out where we are now
                Point3f hit;
                CGPoint touchPt = [pan locationOfTouch:0 inView:glView];
                lastTouch = touchPt;
                [mapView pointOnPlaneFromScreen:touchPt transform:&startTransform
                                       frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight)
                                            hit:&hit clip:false];

                // Note: Just doing a translation for now.  Won't take angle into account
                Point3f newLoc = startOnPlane - hit + startLoc;
                
                // We'll do a hard stop if we're not within the bounds
                // Note: Should do an intersection instead
                if ([self withinBounds:newLoc])
                {
                    [mapView setLoc:newLoc];
                }
            }
        }
            break;
        case UIGestureRecognizerStateEnded:
            if (panning)
            {
                // We'll use this to get two points in model space
                CGPoint vel = [pan velocityInView:glView];
                CGPoint touch0 = lastTouch;
                CGPoint touch1 = touch0;  touch1.x += AnimLen*vel.x; touch1.y += AnimLen*vel.y;
                Point3f model_p0,model_p1;

                Eigen::Matrix4f modelMat = [mapView calcFullMatrix];
                [mapView pointOnPlaneFromScreen:touch0 transform:&modelMat frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&model_p0 clip:false];
                [mapView pointOnPlaneFromScreen:touch1 transform:&modelMat frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&model_p1 clip:false];
                
                // This will give us a direction
                Point2f dir(model_p1.x()-model_p0.x(),model_p1.y()-model_p0.y());
                dir *= -1.0;
                float len = dir.norm();
                float modelVel = len / AnimLen;
                dir.normalize();

                // Caluclate the acceleration based on how far we'd like it to go
                float accel = - modelVel / (AnimLen * AnimLen);

                // Kick off a little movement at the end
                translateDelegate = [[MaplyAnimateTranslateMomentum alloc] initWithView:mapView velocity:modelVel accel:accel dir:Point3f(dir.x(),dir.y(),0.0) bounds:bounds];
                mapView.delegate = translateDelegate;
                
                panning = NO;
            }
            break;
        default:
            break;
    }
}

@end
