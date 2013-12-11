/*
 *  AnimateViewMomentum.h
 *  WhirlyGlobeApp
 *
 *  Created by Steve Gifford on 5/23/11.
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
#import "GlobeView.h"

/** The animate view momentum message is sent out when the animation starts.
    You can use this to calculate where it will end.
  */
@interface AnimateViewMomentumMessage : NSObject

/// The globe view this related to
@property (nonatomic,readonly) WhirlyGlobeView *globeView;

/// When this animation will end
@property (nonatomic,readonly) NSTimeInterval endTime;

/// Rotation at the end of this animation
@property (nonatomic,readonly) Eigen::Quaterniond rot;

@end

// Sent out when the animation view momentum delegate starts
#define kAnimateViewMomentum @"WKAnimationViewMomentumStarted"

/** Animate View Momentum is a WhirlyGlobe Animation Delegate
    that will animate from a starting point forward in time with
    an acceleration.  Basically, we use this to simulate momentum.
    We might assign it after a tap and drag is finished.
 */
@interface AnimateViewMomentum : NSObject<WhirlyGlobeAnimationDelegate> 

@property (nonatomic,assign) float velocity;
@property (nonatomic,assign) float acceleration;

/// Initialize with an angular velocity and a negative acceleration (to slow down)
- (id)initWithView:(WhirlyGlobeView *)globeView velocity:(float)velocity accel:(float)acceleration axis:(Eigen::Vector3f)axis;

@end
