/*  MaplyTapMessage_private.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/19/11.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import <UIKit/UIKit.h>
#import "WhirlyVector.h"
#import "MapView_iOS.h"

/// This is the notification you get for a tap on the map
#define MaplyTapMsg @"MaplyTap"

@interface MaplyTapMessage ()

/// Lon/Lat
@property (nonatomic,assign) WhirlyKit::GeoCoord whereGeo;
/// 3D coordinates in the view
@property (nonatomic,assign) WhirlyKit::Point3f worldLoc;

@end
