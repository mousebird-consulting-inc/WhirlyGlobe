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

using namespace Eigen;
using namespace WhirlyKit;

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
    bool _doRotation;
    // Tilt parameters
    bool tiltZoom;
    float minTilt,maxTilt,minTiltHeight,maxTiltHeight;
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
        _doRotation = true;
        tiltZoom = false;
        valid = false;
	}
	
	return self;
}

- (void)setMinTilt:(float)inMinTilt maxTilt:(float)inMaxTilt minHeight:(float)inMinHeight maxHeight:(float)inMaxHeight
{
    tiltZoom = true;
    minTilt = inMinTilt;
    maxTilt = inMaxTilt;
    minTiltHeight = inMinHeight;
    maxTiltHeight = inMaxHeight;
}

- (bool)getMinTilt:(float *)retMinTilt maxTilt:(float *)retMaxTilt minHeight:(float *)retMinHeight maxHeight:(float *)retMaxHeight
{
    *retMinTilt = minTilt;
    *retMaxTilt = maxTilt;
    *retMinHeight = minTiltHeight;
    *retMaxHeight = maxTiltHeight;
    
    return tiltZoom;
}

- (void)clearTiltZoom
{
    tiltZoom = false;
}

- (float)calcTilt
{
    float newTilt = 0.0;

    // Now the tilt, if we're in that mode
    if (tiltZoom)
    {
        float newHeight = globeView.heightAboveGlobe;
        if (newHeight <= minTiltHeight)
            newTilt = minTilt;
        else if (newHeight >= maxTiltHeight)
            newTilt = maxTilt;
        else {
            float t = (newHeight-minTiltHeight)/(maxTiltHeight - minTiltHeight);
            if (t != 0.0)
                newTilt = t * (maxTilt - minTilt) + minTilt;
        }
    }
    
    return newTilt;
}


+ (WGPinchDelegateFixed *)pinchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView
{
	WGPinchDelegateFixed *pinchDelegate = [[WGPinchDelegateFixed alloc] initWithGlobeView:globeView];
    UIPinchGestureRecognizer *pinchRecog = [[UIPinchGestureRecognizer alloc] initWithTarget:pinchDelegate action:@selector(pinchGesture:)];
    pinchRecog.delegate = pinchDelegate;
	[view addGestureRecognizer:pinchRecog];
	return pinchDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Called for pinch actions
- (void)pinchGesture:(id)sender
{
	UIPinchGestureRecognizer *pinch = sender;
	WhirlyKitEAGLView *glView = (WhirlyKitEAGLView  *)pinch.view;
	UIGestureRecognizerState theState = pinch.state;
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;
    
    if (pinch.numberOfTouches != 2)
        valid = false;
	
	switch (theState)
	{
		case UIGestureRecognizerStateBegan:
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
                
                // Calculate a starting rotation
                if (_doRotation)
                {
                    CGPoint center = [pinch locationInView:glView];
                    CGPoint touch0 = [pinch locationOfTouch:0 inView:glView];
                    float dx = touch0.x-center.x,dy=touch0.y-center.y;
                    startRot = atan2(dy, dx);
                }
            } else
                valid = true;
            [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidStart object:globeView];
            
			break;
		case UIGestureRecognizerStateChanged:
            if (valid)
            {
                [globeView cancelAnimation];
                
                // And adjust the height too
                [globeView setHeightAboveGlobe:startZ/pinch.scale updateWatchers:false];
                
                if (_zoomAroundPinch)
                {
                    // Figure out where we are now
                    // We have to roll back to the original transform with the current height
                    //  to get the rotation we want
                    Point3d hit;
                    Eigen::Quaterniond oldQuat = globeView.rotQuat;
                    [globeView setRotQuat:startQuat updateWatchers:false];
                    Eigen::Matrix4d curTransform = [globeView calcFullMatrix];
                    Eigen::Quaterniond newRotQuat;
                    Point3d axis = [globeView currentUp];
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
                        newRotQuat = oldQuat;
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
                    
                    [globeView setRotQuat:(newRotQuat) updateWatchers:false];
                    float newTilt = [self calcTilt];
                    [globeView setTilt:newTilt];
                }
                
                [globeView runViewUpdates];
            }
			break;
        case UIGestureRecognizerStateEnded:
            [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidEnd object:globeView];
            valid = false;
            
			break;
        default:
            break;
	}
}

@end
