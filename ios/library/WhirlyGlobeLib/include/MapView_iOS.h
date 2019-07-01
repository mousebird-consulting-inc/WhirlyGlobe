/*
 *  MapView_iOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/30/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "MaplyView.h"

namespace Maply
{
    
/// Wrapper around the MapView that kicks out iOS Notifications
class MapView_iOS : public MapView
{
public:
    MapView_iOS(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter);
    
    /// Set the change delegate
    virtual void setDelegate(MapViewAnimationDelegateRef delegate);
    
    /// Called to cancel a running animation
    virtual void cancelAnimation();
    
    /// Used in notifications for identifying a particular view
    id tag;
};
    
typedef std::shared_ptr<MapView_iOS> MapView_iOSRef;
    
}
