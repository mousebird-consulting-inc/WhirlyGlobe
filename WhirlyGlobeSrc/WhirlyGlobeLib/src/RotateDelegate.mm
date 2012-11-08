/*
 *  RotateDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/10/11.
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

#import "WhirlyVector.h"
#import "RotateDelegate.h"
#import "EAGLView.h"
#import "SceneRendererES.h"

@implementation WhirlyGlobeRotateDelegate

using namespace WhirlyGlobe;

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
        rotType = RotNone;
	}
	
	return self;
}

+ (WhirlyGlobeRotateDelegate *)rotateDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
	WhirlyGlobeRotateDelegate *rotateDelegate = [[WhirlyGlobeRotateDelegate alloc] initWithGlobeView:globeView];
    UIRotationGestureRecognizer *rotateRecog = [[UIRotationGestureRecognizer alloc] initWithTarget:rotateDelegate action:@selector(rotateGesture:)];
    rotateRecog.delegate = rotateDelegate;
	[view addGestureRecognizer:rotateRecog];
	return rotateDelegate;
}

// We'll let other gestures run
// We're expecting to run as well
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Save the current state as the initial rotation state
- (void)startRotationMaipulation:(UIRotationGestureRecognizer *)rotate sceneRender:(WhirlyKitSceneRendererES *)sceneRender glView:(WhirlyKitEAGLView  *)glView
{
    startQuat = [globeView rotQuat];
    axis = [globeView currentUp];
    rotType = RotFree;
}

// Called for rotate actions
- (void)rotateGesture:(id)sender
{
	UIRotationGestureRecognizer *rotate = sender;
	WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)rotate.view;
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;
    
    // Turn off rotation if we fall below two fingers
    if ([rotate numberOfTouches] < 2)
    {
        rotType = RotNone;
        return;
    }
    
	switch (rotate.state)
	{
		case UIGestureRecognizerStateBegan:
            [globeView cancelAnimation];

            [self startRotationMaipulation:rotate sceneRender:sceneRender glView:glView];
			break;
		case UIGestureRecognizerStateChanged:
            [globeView cancelAnimation];

            if (rotType == RotFree)
            {
                Eigen::AngleAxisf rotQuat(-rotate.rotation,axis);
                Eigen::Quaternionf newRotQuat = startQuat * rotQuat;
                [globeView setRotQuat:newRotQuat];
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
