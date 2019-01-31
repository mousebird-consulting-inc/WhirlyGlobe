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
#import "ViewState.h"

@class WhirlyKitLayerThread;
@class WhirlyKitViewState;

namespace WhirlyKit {

/** The layer view watcher is a base class.  We subclass it for specific
    view types, such as globe and map.  Each of the subclasses determines
    the criteria for watcher updates.
 */
class LayerViewWatcher : public ViewWatcher
{
    LayerViewWatcher(View *view,WhirlyKitLayerThread *layerThread);
    
    /// Add the given target/selector combo as a watcher.
    /// Will get called at most the given frequency.
    void addWatcherTarget(id target,SEL selector,TimeInterval minTime,float minDist,TimeInterval maxLagTime);

    /// Remove the given target/selector combo
    void removeWatcherTarget(id target,SEL selector);

    /// Called every time the view updates
    virtual void viewUpdated(View *view);
protected:
    
    // Keep track of what our watchers are up to
    class LocalWatcher
    {
    public:
        id __weak target;
        SEL selector;
        TimeInterval minTime,maxLagTime;
        Point3d lastEyePos;
        float minDist;
        TimeInterval lastUpdated;
    };

    /// Layer we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    /// Watchers we'll call back for updates
    std::vector<LocalWatcher> watchers;
    
    /// When the last update was run
    TimeInterval lastUpdate;
    
    /// You should know the type here.  A globe or a map view state.
    ViewStateRef lastViewState;
    ViewStateRef newViewState;
    
    bool kickoffScheduled;
    bool sweepLaggardsScheduled;
};
    
}
