/*
 *  PanDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/18/11.
 *  Copyright 2011-2013 mousebird consulting
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
#import "PanDelegate.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyGlobePanDelegate
{
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/PanDelegate.mm
    WhirlyGlobe::GlobeView *view;
=======
	WhirlyGlobeView * __weak view;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/PanDelegate.mm
    /// Set if we're in the process of panning
	BOOL panning;
	/// The view transform when we started
	Eigen::Matrix4d startTransform;
	/// Where we first touched the sphere
	WhirlyKit::Point3d startOnSphere;
	/// Rotation when we started
	Eigen::Quaterniond startQuat;
}

- (id)initWithGlobeView:(WhirlyGlobe::GlobeView *)inView
{
	if ((self = [super init]))
	{
		view = inView;
	}
	
	return self;
}

+ (WhirlyGlobePanDelegate *)panDelegateForView:(UIView *)view globeView:(WhirlyGlobe::GlobeView *)globeView
{
	WhirlyGlobePanDelegate *panDelegate = [[WhirlyGlobePanDelegate alloc] initWithGlobeView:globeView];
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
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/PanDelegate.mm
	WhirlyKit::SceneRendererES *sceneRender = glView.renderer;
=======
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/PanDelegate.mm
	
	if (pan.numberOfTouches > 1)
	{
		panning = NO;
		return;
	}
		
	switch (pan.state)
	{
		case UIGestureRecognizerStateBegan:
		{
			view->cancelAnimation();

			// Save the first place we touched
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/PanDelegate.mm
			startTransform = view->calcFullMatrix();
			startQuat = view->getRotQuat();
			panning = NO;
            Point2f frameSize = sceneRender->getFramebufferSize();
            if (view->pointOnSphereFromScreen([pan locationInView:glView],&startTransform,
                                    Point2f(frameSize.x()/glView.contentScaleFactor,frameSize.y()/glView.contentScaleFactor),
                                            &startOnSphere,true))
=======
			startTransform = [view calcFullMatrix];
			startQuat = [view rotQuat];
			panning = NO;
            if ([view pointOnSphereFromScreen:[pan locationInView:glView] transform:&startTransform
                                    frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor)
                                            hit:&startOnSphere normalized:true])
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/PanDelegate.mm
				panning = YES;
		}
			break;
		case UIGestureRecognizerStateChanged:
		{
			if (panning)
			{
				view->cancelAnimation();

				// Figure out where we are now
				Point3d hit;
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/src/locker/PanDelegate.mm
                Point2f frameSize = sceneRender->getFramebufferSize();
                view->pointOnSphereFromScreen([pan locationInView:glView],&startTransform,
                                    Point2f(frameSize.x()/glView.contentScaleFactor,frameSize.y()/glView.contentScaleFactor),
                                            &hit,true);
=======
                [view pointOnSphereFromScreen:[pan locationInView:glView] transform:&startTransform
                                    frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor)
                                            hit:&hit normalized:true];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/PanDelegate.mm

				// This gives us a direction to rotate around
				// And how far to rotate
				Eigen::Quaterniond endRot;
				endRot.setFromTwoVectors(startOnSphere,hit);
                Eigen::Quaterniond newRotQuat = startQuat * endRot;

                view->setRotQuat(newRotQuat);
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
