/*
 *  WGViewTracker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/26/12.
 *  Copyright 2012 mousebird consulting
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
#import "WGCoordinate.h"

/** The View Tracker keeps track of a UIView and a corresponding geographic
    location.  We'll use this to place the view on the screen according to
    the location.
 */
@interface MaplyViewTracker : NSObject
{
    /// The view we want to place
    UIView *view;
    /// Where we want to place it
    MaplyCoordinate loc;
}

@property (nonatomic,strong) UIView *view;
@property (nonatomic,assign) MaplyCoordinate loc;

@end

typedef MaplyViewTracker WGViewTracker;
