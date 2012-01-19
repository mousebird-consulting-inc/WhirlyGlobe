/*
 *  PanDelegateMap.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
 *  Copyright 2011 mousebird consulting
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

using namespace WhirlyGlobe;

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
	WhirlyMapPanDelegate *panDelegate = [[[WhirlyMapPanDelegate alloc] initWithMapView:mapView] autorelease];
	[view addGestureRecognizer:[[[UIPanGestureRecognizer alloc] initWithTarget:panDelegate action:@selector(panAction:)] autorelease]];
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
	WhirlyGlobeEAGLView  *glView = (WhirlyGlobeEAGLView  *)pan.view;
	WhirlyGlobeSceneRendererES1 *sceneRender = glView.renderer;

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
            [mapView pointOnPlaneFromScreen:[pan locationOfTouch:0 inView:glView] transform:&startTransform
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
                [mapView pointOnPlaneFromScreen:[pan locationOfTouch:0 inView:glView] transform:&startTransform
                                       frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight)
                                             hit:&hit];

                // Note: Just doing a translation for now.  Won't take angle into account
                Point3f newLoc = hit - startOnPlane + startLoc;
                [mapView setLoc:newLoc];
            }
        }
            break;
        case UIGestureRecognizerStateEnded:
            panning = NO;
            break;
        default:
            break;
    }
}

@end
