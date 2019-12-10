/*
 *  PanDelegateFixed.m
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 4/28/11.
 *  Copyright 2011-2019 mousebird consulting
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

#import "gestures/GlobePanDelegate.h"
#import <UIKit/UIGestureRecognizerSubclass.h>
#import "ViewWrapper.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

// Kind of panning we're in the middle of
typedef enum {PanNone,PanFree,PanSuspended} PanningType;

@implementation MinDelayPanGestureRecognizer

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    startTime = TimeGetCurrent();
    [super touchesBegan:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    if (TimeGetCurrent() - startTime >= kPanDelegateMinTime)
        [super touchesEnded:touches withEvent:event];
    else
        self.state = UIGestureRecognizerStateFailed;
}

- (void)forceEnd {
    self.state = UIGestureRecognizerStateEnded;
}

@end

@implementation WhirlyGlobePanDelegate
{
    GlobeView_iOSRef globeView;
    UITouch *startTouch;  // The touch we're following
    CGPoint startPoint;
    // Used to keep track of what sort of rotation we're doing
    PanningType panType;
	// The view transform when we started
	Eigen::Matrix4d startTransform;
	// Where we first touched the sphere
    WhirlyKit::Point3d startOnSphere;
    double sphereRadius;
	// Rotation when we started
	Eigen::Quaterniond startQuat;
    
    // Last sample for spinning
    Eigen::Quaterniond spinQuat;
    CFTimeInterval spinDate;
    CGPoint lastTouch;
    AnimateViewMomentum *viewAnimation;
    
    bool runEndMomentum;
}

- (instancetype)initWithGlobeView:(GlobeView_iOSRef)inView
{
	if ((self = [super init]))
	{
		globeView = inView;
        panType = PanNone;
        runEndMomentum = true;
	}
	
	return self;
}


+ (WhirlyGlobePanDelegate *)panDelegateForView:(UIView *)view globeView:(GlobeView_iOSRef)globeView useCustomPanRecognizer:(bool)useCustomPanRecognizer
{
	WhirlyGlobePanDelegate *panDelegate = [[WhirlyGlobePanDelegate alloc] initWithGlobeView:globeView];
    UIPanGestureRecognizer *panRecog;
    if (useCustomPanRecognizer)
        panRecog = [[MinDelayPanGestureRecognizer alloc] initWithTarget:panDelegate action:@selector(panAction:)];
    else
        panRecog = [[UIPanGestureRecognizer alloc] initWithTarget:panDelegate action:@selector(panAction:)];
    panRecog.delegate = panDelegate;
    panDelegate.gestureRecognizer = panRecog;
	[view addGestureRecognizer:panRecog];
	return panDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

// Save the initial rotation state and let us rotate after this
- (void)startRotateManipulation:(UIPanGestureRecognizer *)pan sceneRender:(SceneRenderer *)sceneRender wrapView:(UIView<WhirlyKitViewWrapper> *)wrapView
{
    // Save the first place we touched
    startTransform = globeView->calcFullMatrix();
    startQuat = globeView->getRotQuat();
    spinQuat = globeView->getRotQuat();
    startPoint = [pan locationInView:wrapView];
    Point2f startPt2f(startPoint.x,startPoint.y);
    spinDate = TimeGetCurrent();
    lastTouch = [pan locationInView:wrapView];
    
    IntersectionManager *intManager = (IntersectionManager *)sceneRender->getScene()->getManager(kWKIntersectionManager);

    // Look for an intersection with grabbable objects
    Point3d interPt;
    double interDist;
    auto frameSizeScaled = sceneRender->getFramebufferSizeScaled();
    if (intManager->findIntersection(sceneRender, globeView.get(), frameSizeScaled, Point2f(startPoint.x,startPoint.y), interPt, interDist))
    {
        sphereRadius = interPt.norm();
        startOnSphere = interPt.normalized();
        panType = PanFree;        
    } else {
        sphereRadius = 1.0;
        if (globeView->pointOnSphereFromScreen(startPt2f, startTransform, frameSizeScaled, startOnSphere, true))
        {
            // We'll start out letting them play with both axes
            panType = PanFree;                
        } else
            panType = PanNone;
    }
}

// How long we let the momentum run at the end of a pan
static const float MomentumAnimLen = 1.0;

- (bool)pointOnPlaneFromScreen:(CGPoint)pt transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize hit:(Point3d *)hit
{
    // Back Project the screen point into model space
    Point3d screenPt = globeView->pointUnproject(Point2f(pt.x,pt.y), frameSize.x(), frameSize.y(), false);
    
    screenPt.normalize();
    if (screenPt.z() == 0.0)
        return false;
    float t = - globeView->getHeightAboveGlobe() / screenPt.z();
    
    *hit = screenPt * t;
    
    return true;
}

- (bool)pointOnPlaneFromScreen:(CGPoint)pt transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize height:(float)height hit:(Point3d *)hit
{
    // Back Project the screen point into model space
    Point3d screenPt = globeView->pointUnproject(Point2f(pt.x,pt.y), frameSize.x(), frameSize.y(), false);
    
    screenPt.normalize();
    if (screenPt.z() == 0.0)
        return false;
    float t = - (globeView->getHeightAboveGlobe()-height) / screenPt.z();
    
    *hit = screenPt * t;
    
    return true;
}

// Called for pan actions
- (void)panAction:(id)sender
{
	UIPanGestureRecognizer *pan = sender;
    UIView<WhirlyKitViewWrapper> *wrapView = (UIView<WhirlyKitViewWrapper> *)pan.view;
    SceneRenderer *sceneRender = wrapView.renderer;

    if (pan.state == UIGestureRecognizerStateCancelled)
    {
        if (panType != PanNone)
            [[NSNotificationCenter defaultCenter] postNotificationName:kPanDelegateDidEnd object:globeView->tag];
        panType = PanNone;
        return;
    }
    
    auto frameSize = sceneRender->getFramebufferSizeScaled();

    // End for more than one finger
    if ([pan numberOfTouches] > 1)
    {
        panType = PanSuspended;
        runEndMomentum = false;

        if ([_gestureRecognizer isKindOfClass:[MinDelayPanGestureRecognizer class]]) {
            // Don't cancel if interoperating with a scroll view.  (Otherwise
            // the globe view would be paged away.)
            MinDelayPanGestureRecognizer *minDelayPanGestureRecognizer = (MinDelayPanGestureRecognizer *)_gestureRecognizer;
            [minDelayPanGestureRecognizer forceEnd];
        } else {
            // Cancel gesture
            _gestureRecognizer.enabled = false;
            _gestureRecognizer.enabled = true;
        }
        return;
    }
	    
	switch (pan.state)
	{
		case UIGestureRecognizerStateBegan:
		{
            globeView->cancelAnimation();
            runEndMomentum = true;
            
            [self startRotateManipulation:pan sceneRender:sceneRender wrapView:wrapView];

            // Cancel gesture if touched within globe view but outside of the globe itself.
            // When interoperating with a scroll view, this allows a horizontal pan gesture
            // to be interpreted as a swipe, and thus trigger paging the scroll view.
            if (panType == PanNone) {
                self.gestureRecognizer.enabled = NO;
                self.gestureRecognizer.enabled = YES;
                return;
            }
            [[NSNotificationCenter defaultCenter] postNotificationName:kPanDelegateDidStart object:globeView->tag];
		}
			break;
		case UIGestureRecognizerStateChanged:
		{
            if (panType == PanSuspended)
            {
                globeView->cancelAnimation();
                
                // We were suspended, probably because the user dropped another finger
                // So now restart the process
                [self startRotateManipulation:pan sceneRender:sceneRender wrapView:wrapView];

                CGPoint touchPt = [pan locationInView:wrapView];
                lastTouch = touchPt;
            }
			if (panType != PanNone)
			{
                globeView->cancelAnimation();
                
				// Figure out where we are now
				Point3d hit;
                CGPoint touchPt = [pan locationInView:wrapView];
                Point2f touchPt2f(touchPt.x,touchPt.y);
                lastTouch = touchPt;
                bool onSphere = globeView->pointOnSphereFromScreen(touchPt2f, startTransform, frameSize, hit, true, sphereRadius);
                hit.normalize();
                
                // The math breaks down when we have a significant tilt
                // Cancel when they do that
                if (!onSphere && globeView->getTilt() != 0.0)
                {
                    self.gestureRecognizer.enabled = NO;
                    self.gestureRecognizer.enabled = YES;
                    return;
                }
                    
				// This gives us a direction to rotate around
				// And how far to rotate
				Eigen::Quaterniond endRot;
                endRot = QuatFromTwoVectors(startOnSphere,hit);
                Eigen::Quaterniond newRotQuat = startQuat * endRot;

                if (_northUp)
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
                        Eigen::AngleAxisd upRot(ang,newUp);
                        newRotQuat = newRotQuat * upRot;
                    }
                }
 
                // Keep track of the last rotation
                globeView->setRotQuat(newRotQuat);

                // If our spin sample is too old, grab a new one
                spinDate = TimeGetCurrent();
                spinQuat = globeView->getRotQuat();
			}
		}
			break;
        case UIGestureRecognizerStateFailed:
            if (panType != PanNone)
                [[NSNotificationCenter defaultCenter] postNotificationName:kPanDelegateDidEnd object:globeView->tag];
            panType = PanNone;
            break;
		case UIGestureRecognizerStateEnded:
        {
            bool doNotifyEnd = (panType != PanNone);

            if (panType == PanFree && runEndMomentum)
            {
                // We'll use this to get two points in model space
                CGPoint vel = [pan velocityInView:wrapView];
                CGPoint touch0 = lastTouch;
                CGPoint touch1 = touch0;  touch1.x += MomentumAnimLen*vel.x; touch1.y += MomentumAnimLen*vel.y;
                Point3d p0 = globeView->pointUnproject(Point2f(touch0.x,touch0.y), frameSize.x(), frameSize.y(), false);
                Point3d p1 = globeView->pointUnproject(Point2f(touch1.x,touch1.y), frameSize.x(), frameSize.y(), false);
                Eigen::Matrix4d modelMat = globeView->calcFullMatrix();
                Eigen::Matrix4d invModelMat = modelMat.inverse();
                Vector4d model_p0 = invModelMat * Vector4d(p0.x(),p0.y(),p0.z(),1.0);
                Vector4d model_p1 = invModelMat * Vector4d(p1.x(),p1.y(),p1.z(),1.0);
                model_p0.x() /= model_p0.w();  model_p0.y() /= model_p0.w();  model_p0.z() /= model_p0.w();
                model_p1.x() /= model_p1.w();  model_p1.y() /= model_p1.w();  model_p1.z() /= model_p1.w();
                
                Point3d hit0,hit1;
                if ([self pointOnPlaneFromScreen:touch0 transform:&modelMat frameSize:frameSize height:sphereRadius-1.0 hit:&hit0] &&
                    [self pointOnPlaneFromScreen:touch1 transform:&modelMat frameSize:frameSize height:sphereRadius-1.0 hit:&hit1])
                {
                    
                    float len = (hit1-hit0).norm();
                    float modelVel = len / MomentumAnimLen;
                    float angVel = modelVel;
                    
                    // Rotate around whatever axis makes sense based on the two touches
                    // Note: No longer taking NorthUp into account
                    model_p0.normalize();  model_p1.normalize();
                    Vector3f cross = Vector3f(model_p0.x(),model_p0.y(),model_p0.z()).cross(Vector3f(model_p1.x(),model_p1.y(),model_p1.z()));
                    Vector3f upVector = cross.normalized();

                    // If we're doing north up, just rotate around the Z axis
//                    if (_northUp) {
//                        Vector3f oldUpVector = upVector;
//                        upVector = Vector3f(0,0,(oldUpVector.z() > 0.0 ? 1 : -1));
//                        angVel *= upVector.dot(oldUpVector);
//                    }

                    // Calculate the acceleration based on how far we'd like it to go
                    float accel = - angVel / (MomentumAnimLen * MomentumAnimLen);
                    
                    // Keep going in that direction for a while
                    if (angVel > 0.0)
                    {
                        AnimateViewMomentum *anim = new AnimateViewMomentum(globeView,angVel,accel,upVector,_northUp);
                        globeView->setDelegate(GlobeViewAnimationDelegateRef(anim));
                    }
                }
               
            }
            
            if (doNotifyEnd)
                [[NSNotificationCenter defaultCenter] postNotificationName:kPanDelegateDidEnd object:globeView->tag];
        }
			break;
        default:
            break;
	}
}

@end
