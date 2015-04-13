/*
 *  MaplyUpdateLayer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/13/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MaplyUpdateLayer_private.h"
#import "MaplyViewController_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyViewerState

- (id)initWithViewState:(WhirlyKitViewState *)inViewState
{
    self = [super init];
    if (!self)
        return nil;
    
    _viewState = inViewState;
    
    return self;
}

- (MaplyCoordinate3d) eyePos
{
    Point3d pt = [_viewState eyePos];
    
    MaplyCoordinate3d ret;
    ret.x = pt.x();
    ret.y = pt.y();
    ret.z = pt.z();
    
    return ret;
}

@end

@implementation MaplyUpdateLayer
{
    NSObject<MaplyUpdateDelegate> *delegate;
    WhirlyKitLayerThread __weak *layerThread;
    WhirlyKitViewState *viewState;
}

- (id)initWithDelegate:(NSObject<MaplyUpdateDelegate> *)inDelegate moveDist:(double)moveDist minTime:(double)minTime
{
    self = [super init];
    if (!self)
        return nil;
    
    delegate = inDelegate;
    _moveDist = moveDist;
    _minTime = minTime;
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)inViewC
{
    layerThread = inLayerThread;
    
    // We want view updates, but only occasionally
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:_minTime minDist:0.0 maxLagTime:0.0];
    
    [self performSelector:@selector(startOnThread) onThread:layerThread withObject:nil waitUntilDone:NO];
    
    return true;
}

- (void)startOnThread
{
    [delegate start:self];
}

- (void)viewUpdate:(WhirlyKitViewState *)inViewState
{
    WhirlyKitViewState *lastViewState = viewState;
    WhirlyKitViewState *newViewState = inViewState;
    
    // See how far we've moved
    float dist2 = 0.0;
    if (lastViewState)
    {
        Vector3d eye0 = [lastViewState eyePos];
        Vector3d eye1 = [newViewState eyePos];
        
        dist2 = (eye0-eye1).squaredNorm();
    }
    
    // If this is the first go, call the data source, otherwise we need to have moved sufficiently
    if (!lastViewState || (dist2 >= _moveDist*_moveDist))
    {
        viewState = newViewState;
        [delegate viewerMovedTo:[[MaplyViewerState alloc] initWithViewState:viewState] layer:self];
    }
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
    
    [delegate shutdown:self];
}

@end
