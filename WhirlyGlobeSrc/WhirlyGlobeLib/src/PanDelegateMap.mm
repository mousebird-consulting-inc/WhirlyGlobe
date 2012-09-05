/*
 *  PanDelegateMap.mm
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
#import "SceneRendererES1.h"
#import "PanDelegateMap.h"
#import "AnimateTranslateMomentum.h"

using namespace WhirlyKit;

@interface WhirlyMapPanDelegate()
{
    AnimateTranslateMomentum *translateDelegate;
}
@end

@implementation WhirlyMapPanDelegate

- (id)initWithMapView:(WhirlyMapView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
	}
	
	return self;
}

+ (WhirlyMapPanDelegate *)panDelegateForView:(UIView *)view mapView:(WhirlyMapView *)mapView
{
	WhirlyMapPanDelegate *panDelegate = [[WhirlyMapPanDelegate alloc] initWithMapView:mapView];
	[view addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:panDelegate action:@selector(panAction:)]];
	return panDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Called for pan actions
- (void)panAction:(id)sender
{
    UIPanGestureRecognizer *pan = sender;
	WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)pan.view;
	WhirlyKitSceneRendererES1 *sceneRender = glView.renderer;

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
            startTransform = [mapView calcModelMatrix];
            [mapView pointOnPlaneFromScreen:[pan locationOfTouch:0 inView:pan.view] transform:&startTransform
                                  frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight)
                                        hit:&startOnPlane];
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
                                             hit:&hit];

                // Note: Just doing a translation for now.  Won't take angle into account
                Point3f newLoc = startOnPlane - hit + startLoc;
                [mapView setLoc:newLoc];
            }
        }
            break;
        case UIGestureRecognizerStateEnded:
            if (panning)
            {
                // We'll use this to get two points in model space
                CGPoint vel = [pan velocityInView:glView];
                CGPoint touch0 = lastTouch;
                CGPoint touch1 = touch0;  touch1.x += vel.x; touch1.y += vel.y;
                Point3f model_p0,model_p1;

                Eigen::Matrix4f modelMat = [mapView calcModelMatrix];
                [mapView pointOnPlaneFromScreen:touch0 transform:&modelMat frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&model_p0];
                [mapView pointOnPlaneFromScreen:touch1 transform:&modelMat frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&model_p1];
                
                // This will give us a direction
                Point2f dir(model_p1.x()-model_p0.x(),model_p1.y()-model_p0.y());
                dir *= -1.0;
                float modelVel = dir.norm();
                dir.normalize();
                
                // The acceleration (to slow it down)
                float drag = -1.5;

                // Kick off a little movement at the end   
                translateDelegate = [[AnimateTranslateMomentum alloc] initWithView:mapView velocity:modelVel accel:drag dir:Point3f(dir.x(),dir.y(),0.0)];
                mapView.delegate = translateDelegate;
                
                panning = NO;
            }
            break;
        default:
            break;
    }
}

@end
