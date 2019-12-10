/*
 *  PinchDelegateFixed.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/22/12.
 *  Copyright 2012-2019 mousebird consulting
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

#import "gestures/GlobePinchDelegate.h"
#import "gestures/GlobeRotateDelegate.h"
#import "gestures/GlobeTiltDelegate.h"
#import "ViewWrapper.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobePinchDelegate
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
	GlobeView_iOSRef globeView;
    double startRot;
    Point3d startRotAxis;
    double sphereRadius;
    bool startRotAxisValid;
    bool _trackUp;
    double trackUpRot;
    bool sentRotStartMsg;
}

- (instancetype)initWithGlobeView:(GlobeView_iOSRef)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
		startZ = 0.0;
        _minHeight = globeView->minHeightAboveGlobe();
        _maxHeight = globeView->maxHeightAboveGlobe();
        _zoomAroundPinch = true;
        _doRotation = false;
        _northUp = false;
        _trackUp = false;
        _allowPan = false;
        sphereRadius = 1.0;
        valid = false;
	}
	
	return self;
}

+ (WhirlyGlobePinchDelegate *)pinchDelegateForView:(UIView *)view globeView:(GlobeView_iOSRef)globeView
{
	WhirlyGlobePinchDelegate *pinchDelegate = [[WhirlyGlobePinchDelegate alloc] initWithGlobeView:globeView];
    UIPinchGestureRecognizer *pinchRecog = [[UIPinchGestureRecognizer alloc] initWithTarget:pinchDelegate action:@selector(pinchGesture:)];
    pinchRecog.delegate = pinchDelegate;
    pinchDelegate.gestureRecognizer = pinchRecog;
	[view addGestureRecognizer:pinchRecog];
	return pinchDelegate;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    if ([otherGestureRecognizer.delegate isKindOfClass:[WhirlyGlobeTiltDelegate class]] && valid)
        return FALSE;
    
    return TRUE;
}

- (void)setTrackUp:(double)inTrackUp
{
    _northUp = false;
    _trackUp = true;
    trackUpRot = inTrackUp;
}

- (void)clearTrackUp
{
    _trackUp = false;
}

- (void)setNorthUp:(bool)newVal
{
    _northUp = newVal;
}

- (void)setDoRotation:(bool)newVal
{
    _doRotation = newVal;
}

- (void)setZoomAroundPinch:(bool)newVal
{
    _zoomAroundPinch = newVal;
}

// Called for pinch actions
- (void)pinchGesture:(id)sender
{
	UIPinchGestureRecognizer *pinch = sender;
	UIGestureRecognizerState theState = pinch.state;
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)pinch.view;
    SceneRenderer *sceneRender = wrapView.renderer;

    if (theState == UIGestureRecognizerStateCancelled)
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidEnd object:globeView->tag];
        valid = false;
        return;
    }

    IntersectionManager *intManager = (IntersectionManager *)sceneRender->getScene()->getManager(kWKIntersectionManager);
    if (!intManager)
        return;
    
    if (pinch.numberOfTouches != 2)
        valid = false;
	
    auto frameSizeScaled = sceneRender->getFramebufferSizeScaled();
	switch (theState)
	{
		case UIGestureRecognizerStateBegan:
        {
//            NSLog(@"Pinch started");
            startRotAxisValid = false;
            sentRotStartMsg = false;
			startTransform = globeView->calcFullMatrix();
			startQuat = globeView->getRotQuat();
			// Store the starting Z and pinch center for comparison
			startZ = globeView->getHeightAboveGlobe();
            CGPoint startPoint = [pinch locationInView:wrapView];
            Point2f startPoint2f(startPoint.x,startPoint.y);
            
            if (_zoomAroundPinch)
            {
                // Look for an intersection with grabbable objects
                Point3d interPt;
                double interDist;
                if (intManager->findIntersection(sceneRender, globeView.get(), frameSizeScaled, Point2f(startPoint.x,startPoint.y), interPt, interDist))
                {
                    sphereRadius = interPt.norm();
                    startOnSphere = interPt.normalized();
                    valid = true;
                } else {
                    sphereRadius = 1.0;
                    if (globeView->pointOnSphereFromScreen(startPoint2f, startTransform, frameSizeScaled, startOnSphere, true))
                        valid = true;
                    else
                        valid = false;
                }
                
                if (valid)
                    globeView->cancelAnimation();
                
            } else
                valid = true;
            
            // Calculate a starting rotation
            if (valid && _doRotation)
            {
                CGPoint center = [pinch locationInView:wrapView];
                Point2f center2f(center.x,center.y);
                CGPoint touch0 = [pinch locationOfTouch:0 inView:wrapView];
                float dx = touch0.x-center.x,dy=touch0.y-center.y;
                startRot = atan2(dy, dx);
                Point3d hit;
                Point3d interPt;
                double interDist;
                if (intManager->findIntersection(sceneRender, globeView.get(), frameSizeScaled, Point2f(startPoint.x,startPoint.y), interPt, interDist))
                {
                    sphereRadius = interPt.norm();
                    startOnSphere = interPt.normalized();
                    startRotAxisValid = true;
                    startRotAxis = hit;
                } else {
                    if (globeView->pointOnSphereFromScreen(center2f, startTransform, frameSizeScaled, hit, true))
                    {
                        startRotAxisValid = true;
                        startRotAxis = hit;
                    }
                }
            }

            [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidStart object:globeView->tag];
        }
			break;
		case UIGestureRecognizerStateChanged:
            if (valid)
            {
                bool onSphere = true;
//                NSLog(@"Pinch updated");
                globeView->cancelAnimation();
                
                float heightRun = (startZ+1.0)-sphereRadius;
                
                // And adjust the height too
                float newH = heightRun/pinch.scale + sphereRadius - 1.0;
                
                if (_minHeight <= newH && newH <= _maxHeight)
                    globeView->setHeightAboveGlobe(newH, false);
                
                Eigen::Quaterniond newRotQuat = globeView->getRotQuat();
                Point3d axis = globeView->currentUp();
                Eigen::Quaterniond oldQuat = globeView->getRotQuat();
                if (_doRotation && startRotAxisValid && !(_northUp || _trackUp))
                    newRotQuat = startQuat;
                if (_allowPan || _zoomAroundPinch)
                {
                    if (_zoomAroundPinch)
                    {
                        // Figure out where we are now
                        // We have to roll back to the original transform with the current height
                        //  to get the rotation we want
                        Point3d hit;
                        globeView->setRotQuat(startQuat, false);
                        Eigen::Matrix4d curTransform = globeView->calcFullMatrix();
                        CGPoint pinchPt = [pinch locationInView:wrapView];
                        Point2f pinchPt2f(pinchPt.x,pinchPt.y);
                        if (globeView->pointOnSphereFromScreen(pinchPt2f, curTransform, frameSizeScaled, hit, true, sphereRadius))
                        {
                            // This gives us a direction to rotate around
                            // And how far to rotate
                            Eigen::Quaterniond endRot;
                            endRot.setFromTwoVectors(startOnSphere,hit);
                            axis = hit.normalized();
                            newRotQuat = startQuat * endRot;
                        } else {
                            onSphere = false;
                            newRotQuat = startQuat;
                        }
                    } else {
                        newRotQuat = startQuat;
                    }
                }

                // And do a rotation around the pinch
                if (_doRotation && startRotAxisValid && !(_northUp || _trackUp))
                {
                    CGPoint center = [pinch locationInView:wrapView];
                    CGPoint touch0 = [pinch locationOfTouch:0 inView:wrapView];
                    float dx = touch0.x-center.x,dy=touch0.y-center.y;
                    double curRot = atan2(dy, dx);
                    double diffRot = curRot-startRot;
                    Eigen::AngleAxisd rotQuat(-diffRot,startRotAxis);
                    newRotQuat = newRotQuat * rotQuat;
                    
                    if (curRot != 0.0)
                    {
                        if (!sentRotStartMsg)
                        {
                            sentRotStartMsg = true;
                            [[NSNotificationCenter defaultCenter] postNotificationName:kRotateDelegateDidStart object:globeView->tag];
                        }
                    }
                }
                
                // Keep the pole up if necessary
                if (_northUp || _trackUp)
                {
                    // We'd like to keep the north pole pointed up
                    // So we look at where the north pole is going
                    Vector3d northPole = (newRotQuat * Vector3d(0,0,1)).normalized();
                    if (northPole.y() != 0.0)
                    {
                        // We need to know where up (facing the user) will be
                        //  so we can rotate around that
                        Vector3d newUp = GlobeView::prospectiveUp(newRotQuat);
                        
                        // Then rotate it back on to the YZ axis
                        // This will keep it upward
                        float ang = atan(northPole.x()/northPole.y());
                        // However, the pole might be down now
                        // If so, rotate it back up
                        if (northPole.y() < 0.0)
                            ang += M_PI;
                        
                        // Implement track up rather than north up
                        if (!_northUp && _trackUp)
                        {
                            ang += trackUpRot;
                        }
                        
                        Eigen::AngleAxisd upRot(ang,newUp);
                        newRotQuat = newRotQuat * upRot;
                    }
                }
                
                // This does strange things when we've got a serious tilt and we're off the globe
                if (!onSphere && globeView->getTilt() != 0.0)
                {
                    globeView->setRotQuat(oldQuat);
                    self.gestureRecognizer.enabled = NO;
                    self.gestureRecognizer.enabled = YES;
                    return;
                }
                
                if (_allowPan || _doRotation || _zoomAroundPinch)
                    globeView->setRotQuat(newRotQuat,false);
                if (_tiltDelegate)
                {
                    float newTilt = _tiltDelegate->tiltFromHeight(newH);
                    globeView->setTilt(newTilt);
                }

                if (_rotateDelegate)
                    [_rotateDelegate updateWithCenter:[pinch locationInView:wrapView] touch:[pinch locationOfTouch:0 inView:wrapView ] wrapView:wrapView];
                
                globeView->runViewUpdates();
            }
			break;
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateEnded:
//            NSLog(@"Pinch ended");
            [[NSNotificationCenter defaultCenter] postNotificationName:kPinchDelegateDidEnd object:globeView->tag];
            if (sentRotStartMsg)
            {
                sentRotStartMsg = false;
                [[NSNotificationCenter defaultCenter] postNotificationName:kRotateDelegateDidStart object:globeView->tag];
            }
            valid = false;
            
			break;
        default:
            break;
	}
}

@end
