/*
 *  BaseInfo.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/6/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "WhirlyVector.h"

namespace WhirlyKit
{
class BasicDrawable;
class BasicDrawableInstance;
}

/** Object use as the base for parsing description dictionaries.
  */
@interface WhirlyKitBaseInfo : NSObject

@property (nonatomic,assign) double minVis,maxVis;
@property (nonatomic) double minViewerDist,maxViewerDist;
@property (nonatomic,assign) WhirlyKit::Point3d &viewerCenter;
@property (nonatomic,assign) double drawOffset;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) bool enable;
@property (nonatomic) double fade;
@property (nonatomic) double fadeIn;
@property (nonatomic) double fadeOut;
@property (nonatomic) NSTimeInterval fadeOutTime;
@property (nonatomic) NSTimeInterval startEnable,endEnable;
@property (nonatomic) WhirlyKit::SimpleIdentity programID;

/// Initialize with an NSDictionary
- (id)initWithDesc:(NSDictionary *)desc;

/// Set the various parameters on a basic drawable
- (void)setupBasicDrawable:(WhirlyKit::BasicDrawable *)drawable;

/// Set the various parameters on a basic drawable instance
- (void)setupBasicDrawableInstance:(WhirlyKit::BasicDrawableInstance *)drawable;

@end
