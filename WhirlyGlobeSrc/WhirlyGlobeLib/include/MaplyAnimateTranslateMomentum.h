/*
 *  MaplyAnimateTranslateMomentum.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/20/12.
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

#import <Foundation/Foundation.h>
#import "MaplyView.h"
#import "SceneRendererES.h"

/** Animate Translate Momentum is a Maply animation delegate
    that will animate from a starting point forward in time with
    an acceleration.  We use this to simulate momentum.  Giving it
    a negative acceleration will slow it down.
  */
@interface MaplyAnimateTranslateMomentum : NSObject<MaplyAnimationDelegate>

/// Initialize with a velocity and negative acceleration (to slow down)
- (id)initWithView:(MaplyView *)inMapView velocity:(float)inVel accel:(float)inAcc dir:(WhirlyKit::Point3f)inDir bounds:(std::vector<WhirlyKit::Point2f> &)inBounds view:(UIView *)inView renderer:(WhirlyKitSceneRendererES *)inSceneRenderer;

@end
