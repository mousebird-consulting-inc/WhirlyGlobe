/*
 *  LayerViewWatcher_private.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/23/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import <WhirlyGlobe.h>
#import "DataLayer_private.h"

@class WhirlyKitLayerThread;

/** The layer view watcher is a base class.  We subclass it for specific
    view types, such as globe and map.  Each of the subclasses determines
    the criteria for watcher updates.
 */
@interface WhirlyKitLayerViewWatcher : NSObject

/// The sublcass of WhirlyKit::ViewState we'll use
@property (nonatomic) WhirlyKit::ViewStateFactory *viewStateFactory;

/// Initialize with a view and layer thread
- (id)initWithView:(WhirlyKit::View *)view thread:(WhirlyKitLayerThread *)layerThread;

/// Add the given target/selector combo as a watcher.
/// Will get called at most the given frequency.
- (void)addWatcherTarget:(NSObject<WhirlyKitLayer> *)target minTime:(TimeInterval)minTime minDist:(float)minDist maxLagTime:(TimeInterval)maxLagTime;

/// Remove the given target/selector combo
- (void)removeWatcherTarget:(id)target selector:(SEL)selector;

// This is called in the main thread by the view (indirectly)
- (void)viewUpdated:(WhirlyKit::View *)inView;

@end

///** The Globe Layer View watcher is a subclass of the layer view
//    watcher that takes globe specific parameters into account.
//  */
@interface WhirlyGlobeLayerViewWatcher : WhirlyKitLayerViewWatcher

/// Initialize with the globe view to watch and the layer thread
- (id)initWithView:(WhirlyGlobe::GlobeView *)view thread:(WhirlyKitLayerThread *)inLayerThread;

@end

/// The Map Layer View Watcher is a subclass of the layer view
///  that handles map specific parameters.
@interface MaplyLayerViewWatcher : WhirlyKitLayerViewWatcher

/// Initialize with the globe view to watch and the layer thread
- (id)initWithView:(Maply::MapView *)view thread:(WhirlyKitLayerThread *)inLayerThread;

@end
