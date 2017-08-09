/*
 *  MaplyPanDelegateMap.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
 *  Copyright 2011-2017 mousebird consulting
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
#import "MaplyPanDelegate.h"
#import "MaplyAnimateTranslateMomentum.h"
#import <UIKit/UIGestureRecognizerSubclass.h>

using namespace WhirlyKit;

bool MaplyGestureWithinBounds(const std::vector<WhirlyKit::Point2d> &bounds,const Point3d &loc,UIView *view,WhirlyKitSceneRendererES *sceneRender,MaplyView *testMapView,Point3d *newCenter)
{
    if (newCenter)
        *newCenter = loc;
    
    if (bounds.empty())
        return true;
    
    // The corners of the view should be within the bounds
    CGPoint corners[4];
    corners[0] = CGPointMake(0,0);
    corners[1] = CGPointMake(view.frame.size.width, 0.0);
    corners[2] = CGPointMake(view.frame.size.width, view.frame.size.height);
    corners[3] = CGPointMake(0.0, view.frame.size.height);
    
    bool isValid = false;
    Point2d locOffset(0,0);
    for (unsigned tests=0;tests<4;tests++)
    {
        Point3d newLoc = loc+Point3d(locOffset.x(),locOffset.y(),0.0);
        [testMapView setLoc:newLoc runUpdates:false];
        Eigen::Matrix4d fullMatrix = [testMapView calcFullMatrix];
        
        bool checkOkay = true;
        for (unsigned int ii=0;ii<4;ii++)
        {
            Point3d planePt;
            [testMapView pointOnPlaneFromScreen:corners[ii] transform:&fullMatrix
                                      frameSize:Point2f(sceneRender.framebufferWidth/view.contentScaleFactor,sceneRender.framebufferHeight/view.contentScaleFactor)
                                            hit:&planePt clip:false];
            if (!PointInPolygon(Point2d(planePt.x(),planePt.y()), bounds))
            {
                Point2d closePt;
                ClosestPointToPolygon(bounds, Point2d(planePt.x(),planePt.y()), &closePt);
                Point2d thisOffset = 1.001 * (closePt - Point2d(planePt.x(),planePt.y()));
                // Try to move around, inward
                locOffset += thisOffset;
                checkOkay = false;
                
                break;
            }
        }
        
        if (checkOkay)
        {
            isValid = true;
            if (newCenter)
                *newCenter = newLoc;
            break;
        }
    }
    
    return isValid;
}

@implementation MinDelay2DPanGestureRecognizer

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    startTime = CFAbsoluteTimeGetCurrent();
    [super touchesBegan:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    if (CFAbsoluteTimeGetCurrent() - startTime >= kPanDelegateMinTime)
        [super touchesEnded:touches withEvent:event];
    else
        self.state = UIGestureRecognizerStateFailed;
}

- (void)forceEnd {
    self.state = UIGestureRecognizerStateEnded;
}

@end

@interface MaplyPanDelegate()
{
    MaplyView *mapView;
    /// Set if we're panning
    BOOL panning;
    /// View transform when we started
    Eigen::Matrix4d startTransform;
    /// Where we first touched the plane
    WhirlyKit::Point3d startOnPlane;
    /// Viewer location when we started panning
    WhirlyKit::Point3d startLoc;
    CGPoint lastTouch;
    /// Boundary quad that we're to stay within
    std::vector<WhirlyKit::Point2d> bounds;

    MaplyAnimateTranslateMomentum *translateDelegate;
}
@end

@implementation MaplyPanDelegate

- (id)initWithMapView:(MaplyView *)inView
{
	if ((self = [super init]))
	{
		mapView = inView;
	}
	
	return self;
}

+ (MaplyPanDelegate *)panDelegateForView:(UIView *)view mapView:(MaplyView *)mapView useCustomPanRecognizer:(bool)useCustomPanRecognizer
{
	MaplyPanDelegate *panDelegate = [[MaplyPanDelegate alloc] initWithMapView:mapView];
    UIPanGestureRecognizer *panRecognizer;
    if (useCustomPanRecognizer)
        panRecognizer = [[MinDelay2DPanGestureRecognizer alloc] initWithTarget:panDelegate action:@selector(panAction:)];
    else
        panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:panDelegate action:@selector(panAction:)];
  	panRecognizer.delegate = panDelegate;
    panDelegate.gestureRecognizer = panRecognizer;
	[view addGestureRecognizer:panRecognizer];
    panDelegate.gestureRecognizer = panRecognizer;
	return panDelegate;
}

// We'll let other gestures run
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return TRUE;
}

- (void)setBounds:(WhirlyKit::Point2d *)inBounds
{
    bounds.clear();
    for (unsigned int ii=0;ii<4;ii++)
        bounds.push_back(Point2d(inBounds[ii].x(),inBounds[ii].y()));
}

// Bounds check on a single point
- (bool)withinBounds:(Point3d &)loc view:(UIView *)view renderer:(WhirlyKitSceneRendererES *)sceneRender mapView:(MaplyView *)testMapView newCenter:(Point3d *)newCenter
{
    return MaplyGestureWithinBounds(bounds,loc,view,sceneRender,testMapView,newCenter);
}

// How long we'll animate the gesture ending
static const float AnimLen = 1.0;

// Called for pan actions
- (void)panAction:(id)sender
{
    UIPanGestureRecognizer *pan = sender;
	WhirlyKitEAGLView  *glView = (WhirlyKitEAGLView  *)pan.view;
	WhirlyKitSceneRendererES *sceneRender = glView.renderer;

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
            startTransform = [mapView calcFullMatrix];
            [mapView pointOnPlaneFromScreen:[pan locationInView:pan.view] transform:&startTransform
                                  frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor)
                                        hit:&startOnPlane clip:false];
            startLoc = [mapView loc];
            panning = YES;
            [[NSNotificationCenter defaultCenter] postNotificationName:kPanDelegateDidStart object:mapView];
        }
            break;
        case UIGestureRecognizerStateChanged:
        {
            if (panning)
            {
                [mapView cancelAnimation];
                
                // Figure out where we are now
                Point3d hit;
                CGPoint touchPt = [pan locationInView:glView];
                lastTouch = touchPt;
                [mapView pointOnPlaneFromScreen:touchPt transform:&startTransform
                                       frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor)
                                            hit:&hit clip:false];

                
                // Note: Just doing a translation for now.  Won't take angle into account
                MaplyView *testMapView = [[MaplyView alloc] initWithView:mapView];
                Point3d oldLoc = mapView.loc;
                Point3d newLoc = startOnPlane - hit + startLoc;
                testMapView.loc = newLoc;
                Point3d newCenter;
                bool validLoc = false;
                
                // We'll do a hard stop if we're not within the bounds
                // Note: We're trying this location out, then backing off if it failed.
                if (![self withinBounds:newLoc view:glView renderer:sceneRender mapView:testMapView newCenter:&newCenter])
                {
                    // How about if we leave the x alone?
                    Point3d testLoc = Point3d(oldLoc.x(),newLoc.y(),newLoc.z());
                    [testMapView setLoc:testLoc runUpdates:false];
                    if (![self withinBounds:testLoc view:glView renderer:sceneRender mapView:testMapView newCenter:&newCenter])
                    {
                        // How about leaving y alone?
                        testLoc = Point3d(newLoc.x(),oldLoc.y(),newLoc.z());
                        [testMapView setLoc:testLoc runUpdates:false];
                        if ([self withinBounds:testLoc view:glView renderer:sceneRender mapView:testMapView newCenter:&newCenter])
                            validLoc = true;
                    } else {
                        validLoc = true;
                    }
                } else {
                    validLoc = true;
                }
                
                // Okay, we found a good location, so go
                if (validLoc)
                {
                    [mapView setLoc:newCenter runUpdates:false];
                    [mapView runViewUpdates];
                }
            }
        }
            break;
        case UIGestureRecognizerStateEnded:
            if (panning)
            {
                // We'll use this to get two points in model space
                CGPoint vel = [pan velocityInView:glView];
                if((std::abs(vel.x) + std::abs(vel.y)) > 150) {
                    //if the velocity is to slow, its probably not just a finger up
                    CGPoint touch0 = lastTouch;
                    CGPoint touch1 = touch0;  touch1.x += AnimLen*vel.x; touch1.y += AnimLen*vel.y;
                    Point3d model_p0,model_p1;
                    
                    Eigen::Matrix4d modelMat = [mapView calcFullMatrix];
                    [mapView pointOnPlaneFromScreen:touch0 transform:&modelMat frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor) hit:&model_p0 clip:false];
                    [mapView pointOnPlaneFromScreen:touch1 transform:&modelMat frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor) hit:&model_p1 clip:false];
                    
                    // This will give us a direction
                    Point2f dir(model_p1.x()-model_p0.x(),model_p1.y()-model_p0.y());
                    dir *= -1.0;
                    float len = dir.norm();
                    float modelVel = len / AnimLen;
                    dir.normalize();
                    
                    // Calculate the acceleration based on how far we'd like it to go
                    float accel = - modelVel / (AnimLen * AnimLen);
                    
                    // Kick off a little movement at the end
                    translateDelegate = [[MaplyAnimateTranslateMomentum alloc] initWithView:mapView velocity:modelVel accel:accel dir:Point3f(dir.x(),dir.y(),0.0) bounds:bounds view:glView renderer:sceneRender];
                    mapView.delegate = translateDelegate;
                }
                panning = NO;
                [[NSNotificationCenter defaultCenter] postNotificationName:kPanDelegateDidEnd object:mapView];
            }
        break;
      default:
            break;
    }
}

@end
