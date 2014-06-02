/*
 *  MaplyAnimateTranslation.h
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

#import <UIKit/UIKit.h>
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "MaplyView.h"

/// Maply translation from one location to another.
@interface MaplyAnimateViewTranslation : NSObject<MaplyAnimationDelegate>

/// When to start the animation.  Can be in the past
@property (nonatomic,assign) CFTimeInterval startDate;
/// When to finish the animation.
@property (nonatomic,assign) CFTimeInterval endDate;
/// Where to start the translation.  This is probably where you are when you starting.
@property (nonatomic,assign) WhirlyKit::Point3d startLoc;
/// Where to end the translation.  We'll interpolate from the start to here.
@property (nonatomic,assign) WhirlyKit::Point3d endLoc;

/// Kick off a translate to the given position over the given time
/// Assign this to the globe view's delegate and it'll do the rest
- (id)initWithView:(MaplyView *)globeView translate:(WhirlyKit::Point3f &)newLoc howLong:(float)howLong;

@end
