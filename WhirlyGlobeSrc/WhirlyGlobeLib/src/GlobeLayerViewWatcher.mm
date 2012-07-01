/*
 *  GlobeLayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
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

#import "GlobeLayerViewWatcher.h"
#import "LayerThread.h"

using namespace WhirlyKit;

// Keep track of what our watchers are up to
@interface LocalWatcher : NSObject
{
@public
    id __unsafe_unretained target;
    SEL selector;
    NSTimeInterval minTime;
    NSTimeInterval lastUpdated;
}
@end

@implementation LocalWatcher
@end

@implementation WhirlyGlobeViewState

- (id)initWithView:(WhirlyGlobeView *)globeView
{
    self = [super init];
    if (self)
    {
        heightAboveGlobe = globeView.heightAboveGlobe;
        rotQuat = [globeView rotQuat];
        modelMatrix = [globeView calcModelMatrix];
        fieldOfView = globeView.fieldOfView;
        imagePlaneSize = globeView.imagePlaneSize;
        nearPlane = globeView.nearPlane;
        farPlane = globeView.farPlane;

        // Need the eye point for backface checking
        Eigen::Matrix4f modelTransInv = modelMatrix.inverse();
        Vector4f eyeVec4 = modelTransInv * Vector4f(0,0,1,0);
        eyeVec = Vector3f(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
    }
    
    return self;
}

- (void)dealloc
{
    
}

- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight ll:(Point2f &)ll ur:(Point2f &)ur near:(float &)near far:(float &)far
{
	ll.x() = -imagePlaneSize;
	ur.x() = imagePlaneSize;
	float ratio =  ((float)frameHeight / (float)frameWidth);
	ll.y() = -imagePlaneSize * ratio;
	ur.y() = imagePlaneSize * ratio ;
	near = nearPlane;
	far = farPlane;
}

- (CGPoint)pointOnScreenFromSphere:(const Point3f &)worldLoc transform:(const Eigen::Matrix4f *)transform frameSize:(const Point2f &)frameSize
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
    Point2f ll,ur;
    float near,far;
    [self calcFrustumWidth:frameSize.x() height:frameSize.y() ll:ll ur:ur near:near far:far];
    float u = (ray.x() - ll.x()) / (ur.x() - ll.x());
    float v = (ray.y() - ll.y()) / (ur.y() - ll.y());
    v = 1.0 - v;
    
    CGPoint retPt;
    retPt.x = u * frameSize.x();
    retPt.y = v * frameSize.y();
    
    return retPt;
}

- (Vector3f)currentUp
{
	Eigen::Matrix4f modelMat = modelMatrix.inverse();
	
	Vector4f newUp = modelMat * Vector4f(0,0,1,0);
	return Vector3f(newUp.x(),newUp.y(),newUp.z());
}

- (Eigen::Vector3f)eyePos
{
	Eigen::Matrix4f modelMat = modelMatrix.inverse();
	
	Vector4f newUp = modelMat * Vector4f(0,0,1,1);
	return Vector3f(newUp.x(),newUp.y(),newUp.z());    
}

@end


@implementation WhirlyGlobeLayerViewWatcher

- (id)initWithView:(WhirlyGlobeView *)inView thread:(WhirlyKitLayerThread *)inLayerThread
{
    self = [super initWithView:inView thread:inLayerThread];
    if (self)
    {
        inView.watchDelegate = self;
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
        WhirlyGlobeViewState *viewState = [[WhirlyGlobeViewState alloc] initWithView:(WhirlyGlobeView *)view];
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
    [self performSelector:@selector(removeWatcherTargetLayer:) onThread:layerThread withObject:toRemove waitUntilDone:YES];    
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
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateSingleWatcher:) object:found];
        [watchers removeObject:found];
    }    
}

// This is called in the main thread
- (void)viewUpdated:(WhirlyGlobeView *)inGlobeView
{
    WhirlyGlobeViewState *viewState = [[WhirlyGlobeViewState alloc] initWithView:inGlobeView];
    lastViewState = viewState;
    [self performSelector:@selector(viewUpdateLayerThread:) onThread:layerThread withObject:viewState waitUntilDone:NO];
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(sweepLaggards:) object:nil];
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
    [watch->target performSelector:watch->selector onThread:layerThread withObject:lastViewState waitUntilDone:NO];
    watch->lastUpdated = lastUpdate;
}

// This version is called in the layer thread
// We can dispatch things from here
- (void)viewUpdateLayerThread:(WhirlyGlobeViewState *)viewState
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
    [self viewUpdateLayerThread:(WhirlyGlobeViewState *)lastViewState];
}

@end
