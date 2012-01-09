/*
 *  PanDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/18/11.
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

#import "PanDelegate.h"
#import "EAGLView.h"
#import "SceneRendererES1.h"
#import "PanDelegate.h"

@implementation WhirlyGlobePanDelegate

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
	if ((self = [super init]))
	{
		view = inView;
	}
	
	return self;
}

+ (WhirlyGlobePanDelegate *)panDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
	WhirlyGlobePanDelegate *panDelegate = [[[WhirlyGlobePanDelegate alloc] initWithGlobeView:globeView] autorelease];
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
	EAGLView *glView = (EAGLView *)pan.view;
	SceneRendererES1 *sceneRender = glView.renderer;
	
	if (pan.numberOfTouches > 1)
	{
		panning = NO;
		return;
	}
		
	switch (pan.state)
	{
		case UIGestureRecognizerStateBegan:
		{
			[view cancelAnimation];

			// Save the first place we touched
			startTransform = [view calcModelMatrix];
			startQuat = view.rotQuat;
			panning = NO;
			if ([view pointOnSphereFromScreen:[pan locationOfTouch:0 inView:glView] transform:&startTransform 
									frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&startOnSphere])
				panning = YES;
		}
			break;
		case UIGestureRecognizerStateChanged:
		{
			if (panning)
			{
				[view cancelAnimation];

				// Figure out where we are now
				Point3f hit;
				[view pointOnSphereFromScreen:[pan locationOfTouch:0 inView:glView] transform:&startTransform 
									frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&hit ];

				// This gives us a direction to rotate around
				// And how far to rotate
				Eigen::Quaternion<float> endRot;
				endRot.setFromTwoVectors(startOnSphere,hit);
                Eigen::Quaternion<float> newRotQuat = startQuat * endRot;

                [view setRotQuat:(newRotQuat)];
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
