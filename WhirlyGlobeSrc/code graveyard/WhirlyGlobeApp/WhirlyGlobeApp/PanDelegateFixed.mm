/*
 *  PanDelegateFixed.m
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 4/28/11.
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

#import "PanDelegateFixed.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

@interface PanDelegateFixed()
@property (nonatomic,strong) NSDate *spinDate;
@property (nonatomic,strong) UITouch *startTouch;
@property (nonatomic,strong) AnimateViewMomentum *animateMomentum;
@end

@implementation PanDelegateFixed

@synthesize spinDate;
@synthesize startTouch;
@synthesize animateMomentum;

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
	if ((self = [super init]))
	{
		view = inView;
        panType = PanNone;
	}
	
	return self;
}


+ (PanDelegateFixed *)panDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
	PanDelegateFixed *panDelegate = [[PanDelegateFixed alloc] initWithGlobeView:globeView];
    UIPanGestureRecognizer *panRecog = [[UIPanGestureRecognizer alloc] initWithTarget:panDelegate action:@selector(panAction:)];
    panRecog.delegate = self;
	[view addGestureRecognizer:panRecog];
	return panDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Save the initial rotation state and let us rotate after this
- (void)startRotateManipulation:(UIPanGestureRecognizer *)pan sceneRender:(WhirlyKitSceneRendererES1 *)sceneRender glView:(WhirlyKitEAGLView *)glView
{
    // Save the first place we touched
    startTransform = [view calcModelMatrix];
    startQuat = view.rotQuat;
    spinQuat = view.rotQuat;
    startPoint = [pan locationOfTouch:0 inView:glView];
    self.spinDate = [NSDate date];
    lastTouch = [pan locationOfTouch:0 inView:glView];
    if ([view pointOnSphereFromScreen:startPoint transform:&startTransform 
                            frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&startOnSphere])
        // We'll start out letting them play with box axes
        panType = PanFree;                
    else
        panType = PanNone;
}

// Called for pan actions
- (void)panAction:(id)sender
{
	UIPanGestureRecognizer *pan = sender;
	WhirlyKitEAGLView *glView = (WhirlyKitEAGLView *)pan.view;
	WhirlyKitSceneRendererES1 *sceneRender = glView.renderer;
    
    // Put ourselves on hold for more than one touch
    if ([pan numberOfTouches] > 1)
    {
        panType = PanSuspended;
        return;
    }
	    
	switch (pan.state)
	{
		case UIGestureRecognizerStateBegan:
		{
			[view cancelAnimation];
            
            [self startRotateManipulation:pan sceneRender:sceneRender glView:glView];
		}
			break;
		case UIGestureRecognizerStateChanged:
		{
            if (panType == PanSuspended)
            {
                [view cancelAnimation];
                
                // We were suspended, probably because the user dropped another finger
                // So now restart the process
                [self startRotateManipulation:pan sceneRender:sceneRender glView:glView];
            }
			if (panType != PanNone)
			{
				[view cancelAnimation];
                
				// Figure out where we are now
				Point3f hit;
                CGPoint touchPt = [pan locationOfTouch:0 inView:pan.view];
                lastTouch = touchPt;
				[view pointOnSphereFromScreen:touchPt transform:&startTransform 
									frameSize:Point2f(sceneRender.framebufferWidth,sceneRender.framebufferHeight) hit:&hit ];                
                                                
				// This gives us a direction to rotate around
				// And how far to rotate
				Eigen::Quaternion<float> endRot;
                endRot = QuatFromTwoVectors(startOnSphere,hit);
                Eigen::Quaternion<float> newRotQuat = startQuat * endRot;

                if (KeepNorthUp)
                {
                    // We'd like to keep the north pole pointed up
                    // So we look at where the north pole is going
                    Vector3f northPole = (newRotQuat * Vector3f(0,0,1)).normalized();
                    if (northPole.y() != 0.0)
                    {
                        // We need to know where up (facing the user) will be
                        //  so we can rotate around that
                        Vector3f newUp = [WhirlyGlobeView prospectiveUp:newRotQuat];
                        
                        // Then rotate it back on to the YZ axis
                        // This will keep it upward
                        float ang = atanf(northPole.x()/northPole.y());
                        // However, the pole might be down now
                        // If so, rotate it back up
                        if (northPole.y() < 0.0)
                            ang += M_PI;
                        Eigen::AngleAxisf upRot(ang,newUp);
                        newRotQuat = newRotQuat * upRot;
                    }
                }
 
                // Keep track of the last rotation
                [view setRotQuat:(newRotQuat)];

                // If our spin sample is too old, grab a new one
                self.spinDate = [NSDate date];
                spinQuat = view.rotQuat;
			}
		}
			break;
        case UIGestureRecognizerStateFailed:
            panType = PanNone;
            break;
		case UIGestureRecognizerStateEnded:
        {
            //  The value we calculated is related to what we want, but it isn't quite what we want
            //   so we scale here and feel dirty.
            float heightScale = (view.heightAboveGlobe-[view minHeightAboveGlobe])/([view maxHeightAboveGlobe]-[view minHeightAboveGlobe]);
            float scale = heightScale*(MaxAngularVelocity-MinAngularVelocity)+MinAngularVelocity;
            // Note: This constant is a hack

            // We'll use this to get two points in model space
            CGPoint vel = [pan velocityInView:pan.view];
            CGPoint touch0 = lastTouch;
            
            Point3f p0 = [view pointUnproject:Point2f(touch0.x,touch0.y) width:sceneRender.framebufferWidth height:sceneRender.framebufferHeight clip:false];
            Point2f touch1(touch0.x+vel.x,touch0.y+vel.y);
            Point3f p1 = [view pointUnproject:touch1 width:sceneRender.framebufferWidth height:sceneRender.framebufferHeight clip:false];
            
            // Now unproject them back to the canonical model
            Eigen::Matrix4f modelMat = [view calcModelMatrix].inverse();
            Vector4f model_p0 = modelMat * Vector4f(p0.x(),p0.y(),p0.z(),1.0);
            Vector4f model_p1 = modelMat * Vector4f(p1.x(),p1.y(),p1.z(),1.0);
            model_p0.x() /= model_p0.w();  model_p0.y() /= model_p0.w();  model_p0.z() /= model_p0.w();
            model_p1.x() /= model_p1.w();  model_p1.y() /= model_p1.w();  model_p1.z() /= model_p1.w();
            
            // The acceleration (to slow it down)
            float drag = -1.5,dot,ang;

            // Now for the direction
            Vector3f upVector(0,0,1);
            if (KeepNorthUp)
            {
                // In this case we just care about movement in X and Y                
                // The angle between them, ignoring z, is what we're after
                model_p0.z() = 0;  model_p0.w() = 0;
                model_p1.z() = 0;  model_p1.w() = 0;
                model_p0.normalize();
                model_p1.normalize();
                
                dot = model_p0.dot(model_p1);
                ang = acosf(dot);

                // Rotate around the Z axis (model space)
                Vector3f cross = Vector3f(model_p0.x(),model_p0.y(),0.0).cross(Vector3f(model_p1.x(),model_p1.y(),0.0));
                if (cross.z() < 0)
                {
                    ang *= -1;
                    drag *= -1;
                }
            } else {
                // In this case we consider the full angle between the points
                model_p0.normalize();  model_p1.normalize();
                
                // Rotate around whatever axis makes sense based on the two touches
                Vector3f cross = Vector3f(model_p0.x(),model_p0.y(),model_p0.z()).cross(Vector3f(model_p1.x(),model_p1.y(),model_p1.z()));
                upVector = cross.normalized();

                dot = model_p0.dot(model_p1);
                ang = acosf(dot);                
            }
            ang *= scale;
            drag *= scale/(MaxAngularVelocity-MinAngularVelocity);
            
            // Keep going in that direction for a while
            self.animateMomentum = [[AnimateViewMomentum alloc] initWithView:view velocity:ang accel:drag axis:upVector];
            view.delegate = animateMomentum;
        }
			break;
        default:
            break;
	}
}

@end
