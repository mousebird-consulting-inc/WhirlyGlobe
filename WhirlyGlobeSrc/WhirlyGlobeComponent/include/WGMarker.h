/*
 *  WGMarker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/24/12.
 *  Copyright 2011 mousebird consulting
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

/** 3D Marker.
    Set this is up and hand it over to the WhirlyGlobeViewController for display.
 */
@interface WGMarker : NSObject
{
    /// Location in geographic (lat/lon) in radians
    WGCoordinate loc;
    /// Size in 3D.  Remember that the earth is radius = 1.0.
    CGSize size;
    /// If set, this is the image to use for the marker
    UIImage *image;
}

@property (nonatomic,assign) WGCoordinate loc;
@property (nonatomic,assign) CGSize size;
@property (nonatomic,strong) UIImage *image;

@end
