/*
 *  WGScreenLabel.h
 *  WhirlyGlobeComponent
 *
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobe-MaplyComponent/src/locker/MaplyScreenLabel.m
 *  Created by Steve Gifford on 7/24/12.
=======
 *  Created by Steve Gifford on 3/28/12.
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/LayerViewWatcher.mm
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

<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobe-MaplyComponent/src/locker/MaplyScreenLabel.m
#import "MaplyScreenLabel.h"

@implementation MaplyScreenLabel
=======
#import "LayerViewWatcher.h"
#import "LayerThread.h"
#import "SceneRendererES.h"

using namespace Eigen;
using namespace WhirlyKit;

// Keep track of what our watchers are up to
@interface LocalWatcher : NSObject
{
@public
    id __weak target;
    SEL selector;
    NSTimeInterval minTime,maxLagTime;
    Point3d lastEyePos;
    float minDist;
    NSTimeInterval lastUpdated;
}
@end

@implementation LocalWatcher
@end

@implementation WhirlyKitLayerViewWatcher
{
    /// Layer we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    /// The view we're following for upates
    WhirlyKitView * __weak view;
    /// Watchers we'll call back for updates
    NSMutableArray *watchers;
    
    /// When the last update was run
    NSTimeInterval lastUpdate;
    
    /// You should know the type here.  A globe or a map view state.
    WhirlyKitViewState *lastViewState;
    
    WhirlyKitViewState *newViewState;
    bool kickoffScheduled;
    bool sweepLaggardsScheduled;
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/LayerViewWatcher.mm

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    _selectable = true;
    _layoutImportance = MAXFLOAT;
    _layoutPlacement = kMaplyLayoutRight | kMaplyLayoutLeft | kMaplyLayoutAbove | kMaplyLayoutBelow;
    
    return self;
}

<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobe-MaplyComponent/src/locker/MaplyScreenLabel.m
=======
- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(NSTimeInterval)minTime minDist:(float)minDist maxLagTime:(NSTimeInterval)maxLagTime
{
    LocalWatcher *watch = [[LocalWatcher alloc] init];
    watch->target = target;
    watch->selector = selector;
    watch->minTime = minTime;
    watch->minDist = minDist;
    watch->maxLagTime = maxLagTime;
    [watchers addObject:watch];
    
    // Note: This is running in the layer thread, yet we're accessing the view.  Might be a problem.
    if (!lastViewState && layerThread.renderer.framebufferWidth != 0)
    {
        WhirlyKitViewState *viewState = [[_viewStateClass alloc] initWithView:view renderer:layerThread.renderer ];
        lastViewState = viewState;
    }
    
    // Make sure it gets a starting update
    // The trick here is we need to let the main thread finish setting up first
    if (lastViewState)
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
        [watchers removeObject:found];
    }
}

// This is called in the main thread
- (void)viewUpdated:(WhirlyKitView *)inView
{
    WhirlyKitViewState *viewState = [[_viewStateClass alloc] initWithView:inView renderer:layerThread.renderer];

    // The view has to be valid first
    if (layerThread.renderer.framebufferWidth <= 0.0)
    {
        // Let's check back every so often
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(viewUpdated:) object:inView];
        [self performSelector:@selector(viewUpdated:) withObject:inView afterDelay:0.1];
        return;
    }
    
//    lastViewState = viewState;
    @synchronized(self)
    {
        newViewState = viewState;
        if (!kickoffScheduled)
        {
            kickoffScheduled = true;
            [self performSelector:@selector(kickoffViewUpdated) onThread:layerThread withObject:nil waitUntilDone:NO];
        }
    }
}

// This is called in the layer thread
// We kick off the update here
- (void)kickoffViewUpdated
{
    @synchronized(self)
{
    lastViewState = newViewState;
        kickoffScheduled = false;
    }
    [self viewUpdateLayerThread:lastViewState];
    lastUpdate = CFAbsoluteTimeGetCurrent();
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
    // Make sure the thing we're watching is still valid.
    // This can happen with dangling selectors
    if (![watchers containsObject:watch])
    {
//        NSLog(@"Whoa! Tried to call a watcher that's no longer there.");
        return;
    }
    
    if (lastViewState)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
        [watch->target performSelector:watch->selector withObject:lastViewState];
#pragma clang diagnostic pop
        watch->lastUpdated = CFAbsoluteTimeGetCurrent();
        watch->lastEyePos = [lastViewState eyePos];
    } else
        NSLog(@"Missing last view state");
}

// Used to order updates
class LayerPriorityOrder
{
public:
    bool operator < (const LayerPriorityOrder &that) const { return sinceLastUpdate > that.sinceLastUpdate; }
    LayerPriorityOrder(NSTimeInterval sinceLastUpdate,LocalWatcher *watch) : sinceLastUpdate(sinceLastUpdate), watch(watch) { }
    LayerPriorityOrder(const LayerPriorityOrder &that) : sinceLastUpdate(that.sinceLastUpdate), watch(that.watch) { }
    NSTimeInterval sinceLastUpdate;
    LocalWatcher *watch;
};

// This version is called in the layer thread
// We can dispatch things from here
- (void)viewUpdateLayerThread:(WhirlyKitViewState *)viewState
{
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    
    // Look for anything that hasn't been updated in a while
    std::set<LayerPriorityOrder> orderedLayers;
    NSTimeInterval minNextUpdate = 100;
    NSTimeInterval maxLayerDelay = 0.0;
    for (LocalWatcher *watch in watchers)
    {
        NSTimeInterval minTest = curTime - watch->lastUpdated;
        if (minTest > watch->minTime)
        {
            bool runUpdate = false;
            
            // Check the distance, if that's set
            if (watch->minDist > 0.0)
            {
                // If we haven't moved past the trigger, don't update this time
                double thisDist2 = ([viewState eyePos] - watch->lastEyePos).squaredNorm();
                if (thisDist2 > watch->minDist*watch->minDist)
                    runUpdate = true;
                else {
                    if (minTest > watch->maxLagTime)
                    {
                        runUpdate = true;
                        minNextUpdate = MIN(minNextUpdate,minTest);
                    }
                }
            } else
                runUpdate = true;

            if (runUpdate)
                orderedLayers.insert(LayerPriorityOrder(minTest,watch));
        } else {
            minNextUpdate = MIN(minNextUpdate,minTest);
        }
        maxLayerDelay = MAX(maxLayerDelay,minTest);
    }
    
//    static int count = 0;
//    if (count++ % 20 == 0)
//        NSLog(@"Max layer delay = %f, %f, layerThread = %x",maxLayerDelay,minNextUpdate,(unsigned int)layerThread);
    
    // Update the layers by priority
    // Note: What happens if this takes a really long time?
    for (std::set<LayerPriorityOrder>::iterator it = orderedLayers.begin();
         it != orderedLayers.end(); ++it)
        [self updateSingleWatcher:it->watch];
    
    @synchronized(self)
    {
        if (!sweepLaggardsScheduled)
        {
            if (minNextUpdate < 100.0)
            {
                sweepLaggardsScheduled = true;
                [self performSelector:@selector(sweepLaggards:) withObject:nil afterDelay:minNextUpdate];
            }
        }
    }
}

// Minimum update times are there to keep the layers from getting inundated
// That does mean they might get old information when the view stops moving
// So we call this at the end of a given update pass to sweep up the remains
- (void)sweepLaggards:(id)sender
{
    @synchronized(self)
    {
        sweepLaggardsScheduled = false;
    }
    
    [self viewUpdateLayerThread:(WhirlyKitViewState *)lastViewState];
}

@end

@implementation WhirlyKitViewState

- (id)initWithView:(WhirlyKitView *)view renderer:(WhirlyKitSceneRendererES *)renderer
{
    self = [super init];
    if (!self)
        return nil;
    
    _modelMatrix = [view calcModelMatrix];
    _invModelMatrix = _modelMatrix.inverse();
    _viewMatrix = [view calcViewMatrix];
    _invViewMatrix = _viewMatrix.inverse();
    _fullMatrix = [view calcFullMatrix];
    _invFullMatrix = _fullMatrix.inverse();
    _projMatrix = [view calcProjectionMatrix:Point2f(renderer.framebufferWidth,renderer.framebufferHeight) margin:0.0];
    _invProjMatrix = _projMatrix.inverse();
    _fullNormalMatrix = _fullMatrix.inverse().transpose();
    
    _fieldOfView = view.fieldOfView;
    _imagePlaneSize = view.imagePlaneSize;
    _nearPlane = view.nearPlane;
    _farPlane = view.farPlane;
    
    // Need the eye point for backface checking
    Vector4d eyeVec4 = _invFullMatrix * Vector4d(0,0,1,0);
    _eyeVec = Vector3d(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
    // Also a version for the model matrix (e.g. just location, not direction)
    eyeVec4 = _invModelMatrix * Vector4d(0,0,1,0);
    _eyeVecModel = Vector3d(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
    // And calculate where the eye actually is
    Vector4d eyePos4 = _invFullMatrix * Vector4d(0,0,0,1);
    _eyePos = Vector3d(eyePos4.x(),eyePos4.y(),eyePos4.z());
    
    _ll.x() = _ur.x() = 0.0;
    
    _coordAdapter = view.coordAdapter;
    
    return self;
}

- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight
{
	_ll.x() = -_imagePlaneSize;
	_ur.x() = _imagePlaneSize;
	float ratio =  ((float)frameHeight / (float)frameWidth);
	_ll.y() = -_imagePlaneSize * ratio;
	_ur.y() = _imagePlaneSize * ratio ;
	_near = _nearPlane;
	_far = _farPlane;
}

- (Point3d)pointUnproject:(Point2d)screenPt width:(unsigned int)frameWidth height:(unsigned int)frameHeight clip:(bool)clip
{
    if (_ll.x() == _ur.x())
        [self calcFrustumWidth:frameWidth height:frameHeight];
	
	// Calculate a parameteric value and flip the y/v
	double u = screenPt.x() / frameWidth;
    if (clip)
    {
        u = std::max(0.0,u);	u = std::min(1.0,u);
    }
	double v = screenPt.y() / frameHeight;
    if (clip)
    {
        v = std::max(0.0,v);	v = std::min(1.0,v);
    }
	v = 1.0 - v;
	
	// Now come up with a point in 3 space between ll and ur
	Point2d mid(u * (_ur.x()-_ll.x()) + _ll.x(), v * (_ur.y()-_ll.y()) + _ll.y());
	return Point3d(mid.x(),mid.y(),-_near);
}

- (CGPoint)pointOnScreenFromDisplay:(const Point3d &)worldLoc transform:(const Eigen::Matrix4d *)transform frameSize:(const Point2f &)frameSize
{
    // Run the model point through the model transform (presumably what they passed in)
    Eigen::Matrix4d modelMat = *transform;
    Vector4d screenPt = modelMat * Vector4d(worldLoc.x(),worldLoc.y(),worldLoc.z(),1.0);
    
    // Intersection with near gives us the same plane as the screen
    Vector3d ray;
    ray.x() = screenPt.x() / screenPt.w();  ray.y() = screenPt.y() / screenPt.w();  ray.z() = screenPt.z() / screenPt.w();
    ray *= -_nearPlane/ray.z();
    
    // Now we need to scale that to the frame
    if (_ll.x() == _ur.x())
        [self calcFrustumWidth:frameSize.x() height:frameSize.y()];
    double u = (ray.x() - _ll.x()) / (_ur.x() - _ll.x());
    double v = (ray.y() - _ll.y()) / (_ur.y() - _ll.y());
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
    if (_fieldOfView != other->_fieldOfView || _imagePlaneSize != other->_imagePlaneSize ||
        _nearPlane != other->_nearPlane || _farPlane != other->_farPlane)
        return false;
    
    // Matrix comparison
    double *floatsA = _fullMatrix.data();
    double *floatsB = other->_fullMatrix.data();
    for (unsigned int ii=0;ii<16;ii++)
        if (floatsA[ii] != floatsB[ii])
            return false;

    return true;
}

- (void)log
{
    NSLog(@"--- ViewState ---");
    NSLog(@"eyeVec = (%f,%f,%f), eyeVecModel = (%f,%f,%f)",_eyeVec.x(),_eyeVec.y(),_eyeVec.z(),_eyeVecModel.x(),_eyeVecModel.y(),_eyeVecModel.z());
    NSMutableString *matStr = [NSMutableString string];
    for (unsigned int ii=0;ii<16;ii++)
        [matStr appendFormat:@" %f",_fullMatrix.data()[ii]];
    NSLog(@"fullMatrix = %@",matStr);
    NSLog(@"---     ---   ---");
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/src/LayerViewWatcher.mm

@end
