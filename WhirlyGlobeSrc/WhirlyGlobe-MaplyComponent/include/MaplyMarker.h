/*
 *  WGMarker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/24/12.
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
#import "MaplyCoordinate.h"

/** 3D Marker.
    Set this is up and hand it over to the WhirlyGlobeViewController for display.
 */
@interface MaplyMarker : NSObject
{
    /// For user data
    NSObject *userObject;    
    /// Location in geographic (lat/lon) in radians
    MaplyCoordinate loc;
    /// Size in 3D.  Remember that the earth is radius = 1.0.
    CGSize size;
    /// If set, this is the image to use for the marker
    UIImage *image;
    /// If set, this marker can be selected.  On by default.
    bool selectable;
}

@property (nonatomic,strong) NSObject *userObject;
@property (nonatomic,assign) MaplyCoordinate loc;
@property (nonatomic,assign) CGSize size;
@property (nonatomic,strong) UIImage *image;
@property (nonatomic,assign) bool selectable;

@end

typedef MaplyMarker WGMarker;
