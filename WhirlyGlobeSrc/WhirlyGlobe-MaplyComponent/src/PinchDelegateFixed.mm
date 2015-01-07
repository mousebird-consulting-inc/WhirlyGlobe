/*
 *  PinchDelegateFixed.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/22/12.
 *  Copyright 2012 mousebird consulting
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

#import "PinchDelegateFixed.h"
#import "EAGLView.h"
#import "RotateDelegate.h"
#import "TiltDelegate.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WGStandardTiltDelegate
{
    WhirlyGlobeView *globeView;
    bool active;
    double outsideTilt;
    // Tilt parameters
    float minTilt,maxTilt,minTiltHeight,maxTiltHeight;
}

- (id)initWithGlobeView:(WhirlyGlobeView *)inGlobeView
{
    self = [super init];
    globeView = inGlobeView;
    
    return self;
}

- (void)setMinTilt:(float)inMinTilt maxTilt:(float)inMaxTilt minHeight:(float)inMinHeight maxHeight:(float)inMaxHeight
{
    active = true;
    minTilt = inMinTilt;
    maxTilt = inMaxTilt;
    minTiltHeight = inMinHeight;
    maxTiltHeight = inMaxHeight;
}

- (void)getMinTilt:(float *)retMinTilt maxTilt:(float *)retMaxTilt minHeight:(float *)retMinHeight maxHeight:(float *)retMaxHeight
{
    *retMinTilt = minTilt;
    *retMaxTilt = maxTilt;
    *retMinHeight = minTiltHeight;
    *retMaxHeight = maxTiltHeight;
}

- (double)tiltFromHeight:(double)height
{
    double maxValidTilt = [self maxTilt];
    if (!active)
    {
        return std::min(outsideTilt,maxValidTilt);
    }
    
    double newTilt = 0.0;
    
    // Now the tilt, if we're in that mode
    double newHeight = height;
    if (newHeight <= minTiltHeight)
        newTilt = minTilt;
    else if (newHeight >= maxTiltHeight)
        newTilt = maxTilt;
    else {
        float t = (newHeight-minTiltHeight)/(maxTiltHeight - minTiltHeight);
        if (t != 0.0)
            newTilt = t * (maxTilt - minTilt) + minTilt;
    }
    
    return std::min(newTilt,maxValidTilt);
}

/// Return the maximum allowable tilt
- (double)maxTilt
{
    return asin(1.0/(1.0+globeView.heightAboveGlobe));
}

/// Called by an actual tilt gesture.  We're setting the tilt as given
- (void)setTilt:(double)newTilt
{
    active = false;
    outsideTilt = newTilt;
}

@end

@implementation WGPinchDelegateFixed
{
    /// If we're in the process of zooming in, where we started
	float startZ;
	/// The view transform when we started
	Eigen::Matrix4d startTransform;
	/// Where we first touched the sphere
	WhirlyKit::Point3d startOnSphere;
	/// Rotation when we started
	Eigen::Quaterniond startQuat;
    bool valid;
	WhirlyGlobeView *globeView;
    double startRot;
}

- (id)initWithGlobeView:(WhirlyGlobeView *)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
		startZ = 0.0;
        _minHeight = globeView.minHeightAboveGlobe;
        _maxHeight = globeView.maxHeightAboveGlobe;
        _zoomAroundPinch = true;
        _doRotation = false;
        _northUp = false;
        valid = false;
	}
	
	return self;
}

+ (WGPinchDelegateFixed *)pinchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
	WGPinchDelegateFixed *pinchDelegate = [[WGPinchDelegateFixed alloc] initWithGlobeView:globeView];
    UIPinchGestureRecognizer *pinchRecog = [[UIPinchGestureRecognizer alloc] initWithTarget:pinchDelegate action:@selector(pinchGesture:)];
    pinchRecog.delegate = pinchDelegate;
    pinchDelegate.gestureRecognizer = pinchRecog;
	[view addGestureRecognizer:pinchRecog];
	return pinchDelegate;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    if ([otherGestureRecognizer.delegate isKindOfClass:[TiltDelegate class]] && valid)
        return FALSE;
    
    return TRUE;
}

// Called for pinch actions
- (void)pinchGesture:(id)sender
{
	UIPinchGestureRecognizer *pinch = sender;
	WhirlyKitEAGLView *glView = (WhirlyKitEAGLView  *)pinch.view;
	UIGestureRecognizerState theState = pinch.state;
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;
    
    if (theState == UIGestureRecognizerStateCancelled)
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidEnd object:globeView];
        valid = false;
        return;
    }
    
    if (pinch.numberOfTouches != 2)
        valid = false;
	
	switch (theState)
	{
		case UIGestureRecognizerStateBegan:
//            NSLog(@"Pinch started");
			startTransform = [globeView calcFullMatrix];
			startQuat = [globeView rotQuat];
			// Store the starting Z and pinch center for comparison
			startZ = globeView.heightAboveGlobe;
            if (_zoomAroundPinch)
            {
                if ([globeView pointOnSphereFromScreen:[pinch locationInView:glView] transform:&startTransform
                                             frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor)
                                                   hit:&startOnSphere normalized:true])
                    valid = true;
                else
                    valid = false;
                
                if (valid)
                    [globeView cancelAnimation];
                
            } else
                valid = true;
            
            // Calculate a starting rotation
            if (_doRotation)
            {
                CGPoint center = [pinch locationInView:glView];
                CGPoint touch0 = [pinch locationOfTouch:0 inView:glView];
                float dx = touch0.x-center.x,dy=touch0.y-center.y;
                startRot = atan2(dy, dx);
            }

            [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidStart object:globeView];
            
			break;
		case UIGestureRecognizerStateChanged:
            if (valid)
            {
                bool onSphere = true;
//                NSLog(@"Pinch updated");
                [globeView cancelAnimation];
                
                // And adjust the height too
                float newH = startZ/pinch.scale;
                if (_minHeight <= newH && newH <= _maxHeight)
                    [globeView setHeightAboveGlobe:newH updateWatchers:false];

                Eigen::Quaterniond newRotQuat = globeView.rotQuat;
                Point3d axis = [globeView currentUp];
                Eigen::Quaterniond oldQuat = globeView.rotQuat;
                if (_zoomAroundPinch)
                {
                    // Figure out where we are now
                    // We have to roll back to the original transform with the current height
                    //  to get the rotation we want
                    Point3d hit;
                    [globeView setRotQuat:startQuat updateWatchers:false];
                    Eigen::Matrix4d curTransform = [globeView calcFullMatrix];
                    if ([globeView pointOnSphereFromScreen:[pinch locationInView:glView] transform:&curTransform
                                                 frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor)
                                                       hit:&hit normalized:true])
                    {
                        // This gives us a direction to rotate around
                        // And how far to rotate
                        Eigen::Quaterniond endRot;
                        endRot.setFromTwoVectors(startOnSphere,hit);
                        axis = hit.normalized();
                        newRotQuat = startQuat * endRot;
                    } else {
                        onSphere = false;
                        newRotQuat = oldQuat;
                    }
                }

                // And do a rotation around the pinch
                if (_doRotation)
                {
                    CGPoint center = [pinch locationInView:glView];
                    CGPoint touch0 = [pinch locationOfTouch:0 inView:glView];
                    float dx = touch0.x-center.x,dy=touch0.y-center.y;
                    double curRot = atan2(dy, dx);
                    double diffRot = curRot-startRot;
                    Eigen::AngleAxisd rotQuat(-diffRot,axis);
                    newRotQuat = newRotQuat * rotQuat;
                }
                
                // Keep the pole up if necessary
                if (_northUp)
                {
                    // We'd like to keep the north pole pointed up
                    // So we look at where the north pole is going
                    Vector3d northPole = (newRotQuat * Vector3d(0,0,1)).normalized();
                    if (northPole.y() != 0.0)
                    {
                        // We need to know where up (facing the user) will be
                        //  so we can rotate around that
                        Vector3d newUp = [WhirlyGlobeView prospectiveUp:newRotQuat];
                        
                        // Then rotate it back on to the YZ axis
                        // This will keep it upward
                        float ang = atan(northPole.x()/northPole.y());
                        // However, the pole might be down now
                        // If so, rotate it back up
                        if (northPole.y() < 0.0)
                            ang += M_PI;
                        Eigen::AngleAxisd upRot(ang,newUp);
                        newRotQuat = newRotQuat * upRot;
                    }
                }
                
                // This does strange things when we've got a serious tilt and we're off the globe
                if (!onSphere && globeView.tilt != 0.0)
                {
                    globeView.rotQuat = oldQuat;
                    self.gestureRecognizer.enabled = NO;
                    self.gestureRecognizer.enabled = YES;
                    return;
                }
                
                [globeView setRotQuat:(newRotQuat) updateWatchers:false];
                if (_tiltDelegate)
                {
                    float newTilt = [_tiltDelegate tiltFromHeight:newH];
                    [globeView setTilt:newTilt];
                }

                if (_rotateDelegate)
                    [_rotateDelegate updateWithCenter:[pinch locationInView:glView] touch:[pinch locationOfTouch:0 inView:glView ] glView:glView];
                
                [globeView runViewUpdates];
            }
			break;
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateEnded:
//            NSLog(@"Pinch ended");
            [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidEnd object:globeView];
            valid = false;
            
			break;
        default:
            break;
	}
}

@end
