/*
 *  MaplyTapMessage.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/19/11.
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
#import "MaplyView.h"

/// This is the notification you get for a tap on the map
#define MaplyTapMsg @"MaplyTap"

/** Tap Message is an
	indication that the user tapped on the map.
	It's passed as the object in a notification.
 */
@interface MaplyTapMessage : NSObject

/// View that was touched
@property (nonatomic,retain) UIView *view;
//// Touch location on view in 2D
@property (nonatomic,assign) CGPoint touchLoc;
/// Lon/Lat
@property (nonatomic,assign) WhirlyKit::GeoCoord whereGeo;
/// 3D coordinates in the view
@property (nonatomic,assign) WhirlyKit::Point3f worldLoc;
/// Where the eye was.
@property (nonatomic,assign) float heightAboveSurface;

@end
