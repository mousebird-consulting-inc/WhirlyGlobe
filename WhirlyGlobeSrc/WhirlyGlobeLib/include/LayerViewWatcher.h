/*
 *  LayerViewWatcher.h
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

#import <UIKit/UIKit.h>
#import "WhirlyKitView.h"

/// @cond
@class WhirlyKitLayerThread;
/// @endcond

/** The layer view watcher is a base class.  We subclass it for specific
    view types, such as globe and map.  Each of the subclasses determines
    the criteria for watcher updates.
 */
@interface WhirlyKitLayerViewWatcher : NSObject
{
    /// Layer we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    /// The view we're following for upates
    WhirlyKitView * __weak view;
    /// Watchers we'll call back for updates
    NSMutableArray *watchers;
    
    /// When the last update was run
    NSTimeInterval lastUpdate;
}

/// Initialize with a view and layer thread
- (id)initWithView:(WhirlyKitView *)view thread:(WhirlyKitLayerThread *)layerThread;

@end
