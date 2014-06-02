/*
 *  AnimateRotation.h
 *  WhirlyGlobeLib
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

#import <UIKit/UIKit.h>
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "GlobeView.h"

/** Animate View Rotation is WhirlyGlobe Animation Delegate
    that will animate rotation from one point to another over
    time.
 */
@interface AnimateViewRotation : NSObject<WhirlyGlobeAnimationDelegate>

/// When to start the animation.  Can be in the past
@property (nonatomic,assign) NSTimeInterval startDate;
/// When to finish the animation.
@property (nonatomic,assign) NSTimeInterval endDate;
/// Where to start rotating.  This is probably where you are when you start
@property (nonatomic,assign) Eigen::Quaterniond startRot;
/// Where to end the rotation.  We'll interpolate from the start to here
@property (nonatomic,assign) Eigen::Quaterniond endRot;

/// Kick off a rotate to the given position over the given time
/// Assign this to the globe view's delegate and it'll do the rest
<<<<<<< HEAD:WhirlyGlobeSrc/WhirlyGlobeLib/include/locker/AnimateRotation.h
- (id)initWithView:(WhirlyGlobe::GlobeView *)globeView rot:(Eigen::Quaterniond &)newRot howLong:(float)howLong;
=======
- (id)initWithView:(WhirlyGlobeView *)globeView rot:(Eigen::Quaterniond &)newRot howLong:(float)howLong;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b:WhirlyGlobeSrc/WhirlyGlobeLib/include/AnimateRotation.h

@end
