/*
 *  GlobeLayerViewWatcher.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
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
#import "GlobeView.h"
#import "LayerViewWatcher.h"

/** View State related to the Globe view.  This adds
    more parameters relating to the globe.
  */
@interface WhirlyGlobeViewState : WhirlyKitViewState
{
@public
	Eigen::Quaternion<float> rotQuat;
    float heightAboveGlobe;
}

- (id)initWithView:(WhirlyGlobeView *)globeView;

/// Return where up (0,0,1) is after model rotation
- (Eigen::Vector3f)currentUp;

/// Calculate where the eye is in model coordinates
- (Eigen::Vector3f)eyePos;

@end

/** The Globe Layer View watcher is a subclass of the layer view
    watcher that takes globe specific parameters into account.
  */
@interface WhirlyGlobeLayerViewWatcher : WhirlyKitLayerViewWatcher
{
}

/// Initialize with the globe view to watch and the layer thread
- (id)initWithView:(WhirlyGlobeView *)view thread:(WhirlyKitLayerThread *)inLayerThread;

@end
