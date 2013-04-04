/*
 *  LayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/12.
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

#import "LayerViewWatcher.h"
#import "LayerThread.h"

using namespace Eigen;
using namespace WhirlyKit;

// Keep track of what our watchers are up to
@interface LocalWatcher : NSObject
{
@public
    id __weak target;
    SEL selector;
    NSTimeInterval minTime;
    NSTimeInterval lastUpdated;
}
@end

@implementation LocalWatcher
@end

@implementation WhirlyKitLayerViewWatcher

- (id)initWithView:(WhirlyKitView *)inView thread:(WhirlyKitLayerThread *)inLayerThread
{
    self = [super init];
    if (self)
    {
        layerThread = inLayerThread;
        view = inView;
        watchers = [NSMutableArray array];
    }
    
    return self;
}

- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(NSTimeInterval)minTime
{
    LocalWatcher *watch = [[LocalWatcher alloc] init];
    watch->target = target;
    watch->selector = selector;
    watch->minTime = minTime;
    [watchers addObject:watch];
    
    if (!lastViewState)
    {
        WhirlyKitViewState *viewState = [[viewStateClass alloc] initWithView:view];
        lastViewState = viewState;
    }
    
    // Make sure it gets a starting update
    // The trick here is we need to let the main thread finish setting up first
    [self performSelectorOnMainThread:@selector(updateSingleWatcherDelay:) withObject:watch waitUntilDone:NO];
}

- (void)removeWatcherTarget:(id)target selector:(SEL)selector
{
    // Call into the layer thread, just to be safe
    LocalWatcher *toRemove = [[LocalWatcher alloc] init];
    toRemove->target = target;
    toRemove->selector = selector;
    if ([NSThread currentThread] == layerThread)
        [self removeWatcherTargetLayer:toRemove];
    else
        [self performSelector:@selector(removeWatcherTargetLayer:) onThread:layerThread withObject:toRemove waitUntilDone:NO];
}

- (void)removeWatcherTargetLayer:(LocalWatcher *)toRemove
{
    LocalWatcher *found = nil;
    
    for (LocalWatcher *watch in watchers)
    {
        if (watch->target == toRemove->target && watch->selector == toRemove->selector)
        {
            found = watch;
            break;
        }
    }
    
    if (found)
    {
        [layerThread.runLoop cancelPerformSelector:@selector(updateSingleWatcher:) target:self argument:found];
        [watchers removeObject:found];
    }
}

// This is called in the main thread
- (void)viewUpdated:(WhirlyKitView *)inView
{
    WhirlyKitViewState *viewState = [[viewStateClass alloc] initWithView:inView];
    lastViewState = viewState;
    [layerThread.runLoop cancelPerformSelectorsWithTarget:self];
    [self performSelector:@selector(kickoffViewUpdated:) onThread:layerThread withObject:viewState waitUntilDone:NO];
}

// This is called in the layer thread
// We kick off the update here
- (void)kickoffViewUpdated:(WhirlyKitViewState *)newViewState;
{
    lastViewState = newViewState;
    [layerThread.runLoop cancelPerformSelectorsWithTarget:self];
    [self viewUpdateLayerThread:lastViewState];
    lastUpdate = [[NSDate date] timeIntervalSinceReferenceDate];
}

// We're in the main thread here
// Now we can kick off the watcher delay on the layer thread
- (void)updateSingleWatcherDelay:(LocalWatcher *)watch
{
    [self performSelector:@selector(updateSingleWatcher:) onThread:layerThread withObject:watch waitUntilDone:NO];
}

// Let the watcher know about an update
// Called in the layer thread
- (void)updateSingleWatcher:(LocalWatcher *)watch
{
    // Note: This should never, ever happen
    // Make sure the thing we're watching is still valid
    if (![watchers containsObject:watch])
    {
        NSLog(@"Whoa! Tried to call a watcher that's no longer there.");
        return;
    }
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"    
    [watch->target performSelector:watch->selector withObject:lastViewState];
#pragma clang diagnostic pop
//    [layerThread.runLoop performSelector:watch->selector target:watch->target argument:lastViewState order:0 modes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
    watch->lastUpdated = lastUpdate;
}

// This version is called in the layer thread
// We can dispatch things from here
- (void)viewUpdateLayerThread:(WhirlyKitViewState *)viewState
{
    NSTimeInterval curTime = [[NSDate date] timeIntervalSinceReferenceDate];
    
    // Look for anything that hasn't been updated in a while
    float minNextUpdate = 100;
    for (LocalWatcher *watch in watchers)
    {
        NSTimeInterval minTest = curTime - watch->lastUpdated;
        if (minTest > watch->minTime)
        {
            [self updateSingleWatcher:watch];
        } else {
            minNextUpdate = MIN(minNextUpdate,minTest);
        }
    }
    
    // Sweep up the laggard watchers
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(sweepLaggards:) object:nil];
//    [layerThread.runLoop cancelPerformSelector:@selector(sweepLaggards:) target:self argument:nil];
    if (minNextUpdate < 100.0)
    {
        [self performSelector:@selector(sweepLaggards:) withObject:nil afterDelay:minNextUpdate];
    }
}

// Minimum update times are there to keep the layers from getting inundated
// That does mean they might get old information when the view stops moving
// So we call this at the end of a given update pass to sweep up the remains
- (void)sweepLaggards:(id)sender
{
    [self viewUpdateLayerThread:(WhirlyKitViewState *)lastViewState];
}

@end

@implementation WhirlyKitViewState

- (id)initWithView:(WhirlyKitView *)view
{
    self = [super init];
    if (!self)
        return nil;
    
    modelMatrix = [view calcModelMatrix];
    viewMatrix = [view calcViewMatrix];
    fullMatrix = [view calcFullMatrix];
    fieldOfView = view.fieldOfView;
    imagePlaneSize = view.imagePlaneSize;
    nearPlane = view.nearPlane;
    farPlane = view.farPlane;
    
    // Need the eye point for backface checking
    Eigen::Matrix4f fullMatrixInv = fullMatrix.inverse();
    Vector4f eyeVec4 = fullMatrixInv * Vector4f(0,0,1,0);
    eyeVec = Vector3f(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
    // Also a version for the model matrix (e.g. just location, not direction)
    Eigen::Matrix4f modelMatInv = modelMatrix.inverse();
    eyeVec4 = modelMatInv * Vector4f(0,0,1,0);
    eyeVecModel = Vector3f(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());    
    
    ll.x() = ur.x() = 0.0;
    
    coordAdapter = view.coordAdapter;
    
    return self;
}

- (Eigen::Vector3f)eyePos
{
	Eigen::Matrix4f modelMat = modelMatrix.inverse();
	
	Vector4f newUp = modelMat * Vector4f(0,0,0,1);
	return Vector3f(newUp.x(),newUp.y(),newUp.z());
}

- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight
{
	ll.x() = -imagePlaneSize;
	ur.x() = imagePlaneSize;
	float ratio =  ((float)frameHeight / (float)frameWidth);
	ll.y() = -imagePlaneSize * ratio;
	ur.y() = imagePlaneSize * ratio ;
	near = nearPlane;
	far = farPlane;
}

- (Point3f)pointUnproject:(Point2f)screenPt width:(unsigned int)frameWidth height:(unsigned int)frameHeight clip:(bool)clip
{
    if (ll.x() == ur.x())
        [self calcFrustumWidth:frameWidth height:frameHeight];
	
	// Calculate a parameteric value and flip the y/v
	float u = screenPt.x() / frameWidth;
    if (clip)
    {
        u = std::max(0.0f,u);	u = std::min(1.0f,u);
    }
	float v = screenPt.y() / frameHeight;
    if (clip)
    {
        v = std::max(0.0f,v);	v = std::min(1.0f,v);
    }
	v = 1.0 - v;
	
	// Now come up with a point in 3 space between ll and ur
	Point2f mid(u * (ur.x()-ll.x()) + ll.x(), v * (ur.y()-ll.y()) + ll.y());
	return Point3f(mid.x(),mid.y(),-near);
}

- (CGPoint)pointOnScreenFromDisplay:(const Point3f &)worldLoc transform:(const Eigen::Matrix4f *)transform frameSize:(const Point2f &)frameSize
{
    // Run the model point through the model transform (presumably what they passed in)
    Eigen::Matrix4f modelTrans = *transform;
    Matrix4f modelMat = modelTrans;
    Vector4f screenPt = modelMat * Vector4f(worldLoc.x(),worldLoc.y(),worldLoc.z(),1.0);
    screenPt.x() /= screenPt.w();  screenPt.y() /= screenPt.w();  screenPt.z() /= screenPt.w();
    
    // Intersection with near gives us the same plane as the screen
    Point3f ray;
    ray.x() = screenPt.x() / screenPt.w();  ray.y() = screenPt.y() / screenPt.w();  ray.z() = screenPt.z() / screenPt.w();
    ray *= -nearPlane/ray.z();
    
    // Now we need to scale that to the frame
    if (ll.x() == ur.x())
        [self calcFrustumWidth:frameSize.x() height:frameSize.y()];
    float u = (ray.x() - ll.x()) / (ur.x() - ll.x());
    float v = (ray.y() - ll.y()) / (ur.y() - ll.y());
    v = 1.0 - v;
    
    CGPoint retPt;
    if (ray.z() < 0.0)
    {
       retPt.x = u * frameSize.x();
       retPt.y = v * frameSize.y();
    } else
        retPt = CGPointMake(-100000, -100000);
    
    return retPt;
}

- (bool)isSameAs:(WhirlyKitViewState *)other
{
    if (fieldOfView != other->fieldOfView || imagePlaneSize != other->imagePlaneSize ||
        nearPlane != other->nearPlane || farPlane != other->farPlane)
        return false;
    
    // Matrix comparison
    float *floatsA = fullMatrix.data();
    float *floatsB = other->fullMatrix.data();
    for (unsigned int ii=0;ii<16;ii++)
        if (floatsA[ii] != floatsB[ii])
            return false;

    return true;
}

@end
