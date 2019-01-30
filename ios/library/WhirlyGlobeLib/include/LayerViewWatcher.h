/*
 *  LayerViewWatcher.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/12.
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

#import <UIKit/UIKit.h>
#import "WhirlyKitView.h"


@class WhirlyKitLayerThread;
@class WhirlyKitViewState;
@class SceneRendererES;


/** The layer view watcher is a base class.  We subclass it for specific
    view types, such as globe and map.  Each of the subclasses determines
    the criteria for watcher updates.
 */
@interface WhirlyKitLayerViewWatcher : NSObject<WhirlyKitViewWatcherDelegate>

/// The sublcass of WhirlyKitViewState we'll use
@property (nonatomic) Class viewStateClass;

/// Initialize with a view and layer thread
- (id)initWithView:(WhirlyKitView *)view thread:(WhirlyKitLayerThread *)layerThread;

/// Add the given target/selector combo as a watcher.
/// Will get called at most the given frequency.
- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(TimeInterval)minTime minDist:(float)minDist maxLagTime:(TimeInterval)maxLagTime;

/// Remove the given target/selector combo
- (void)removeWatcherTarget:(id)target selector:(SEL)selector;

@end

