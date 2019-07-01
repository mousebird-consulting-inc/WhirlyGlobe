/*
 *  GlobeView_iOS.h
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

#import "GlobeView.h"

// Sent when a WhirlyKitView animation starts
#define kWKViewAnimationStarted @"WKViewAnimationStarted"
// Sent when a WhirlyKitView animation is cancelled
#define kWKViewAnimationEnded @"WKViewAnimationEnded"

namespace WhirlyGlobe
{

/// Wrapper around the GlobeView that kicks out iOS Notifications
class GlobeView_iOS : public GlobeView
{
public:
    GlobeView_iOS();
    
    /// Set the change delegate
    virtual void setDelegate(GlobeViewAnimationDelegateRef delegate);
    
    /// Called to cancel a running animation
    virtual void cancelAnimation();
    
    /// Used to mark notifications belonging to this view
    id tag;
    
    WhirlyKit::FakeGeocentricDisplayAdapter fakeGeoC;
};
    
typedef std::shared_ptr<GlobeView_iOS> GlobeView_iOSRef;
    
}
