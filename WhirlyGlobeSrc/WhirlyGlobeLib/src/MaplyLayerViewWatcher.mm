/*
 *  MaplyLayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/14/12.
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

#import "MaplyLayerViewWatcher.h"
#import "LayerThread.h"

using namespace WhirlyKit;

@implementation MaplyViewState

- (id)initWithView:(MaplyView *)mapView renderer:(WhirlyKitSceneRendererES *)renderer
{
    self = [super initWithView:mapView renderer:renderer];
    
    return self;
}

@end


@implementation MaplyLayerViewWatcher

- (id)initWithView:(MaplyView *)inView thread:(WhirlyKitLayerThread *)inLayerThread;
{
    self = [super initWithView:inView thread:inLayerThread];
    if (self)
    {
        [inView addWatcherDelegate:self];
        super.viewStateClass = [MaplyViewState class];
    }
    
    return self;
}

@end
