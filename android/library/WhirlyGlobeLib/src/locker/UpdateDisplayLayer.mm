/*
 *  UpdateDisplayLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/19/12.
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

#import "UpdateDisplayLayer.h"
#import "GlobeLayerViewWatcher.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeUpdateDisplayLayer
{
    /// Layer thread we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    
    /// Scene, just for the data source
    WhirlyKit::Scene *scene;
    
    /// Last view state we were given
    WhirlyGlobe::GlobeViewState *viewState;
}

- (id)initWithDataSource:(NSObject<WhirlyGlobeUpdateDataSource> *)inDataSource moveDist:(float)inMoveDist minTime:(float)inMinTime
{
    self = [super init];
    if (self)
    {
        _dataSource = inDataSource;
        _moveDist = inMoveDist;
        _minTime = inMinTime;
    }
    
    return self;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
    
    // We want view updates, but only 1s in frequency
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:_minTime minDist:0.0 maxLagTime:0.0];
}

- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
    
    [_dataSource shutdown];
}

- (void)viewUpdate:(WhirlyGlobe::GlobeViewState *)inViewState
{
    WhirlyGlobe::GlobeViewState *lastViewState = viewState;
    WhirlyGlobe::GlobeViewState *newViewState = inViewState;

    // See how far we've moved
    float dist2 = 0.0;
    if (lastViewState)
    {
        Vector3d eye0 = lastViewState->eyePos;
        Vector3d eye1 = newViewState->eyePos;
        
        dist2 = (eye0-eye1).squaredNorm();
    }

    // If this is the first go, call the data source, otherwise we need to have moved sufficiently
    if (!lastViewState || (dist2 >= _moveDist*_moveDist))
    {
        viewState = newViewState;
        [_dataSource viewerDidUpdate:viewState scene:scene];
    }
}

@end
