/*
 *  TapMessage.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/3/11.
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

#import <UIKit/UIKit.h>
#import "WhirlyVector.h"
#import "GlobeView.h"

/// This is the notification you get for a tap on the globe
#define WhirlyGlobeTapMsg @"WhirlyGlobeTap"
/// This is the notification you get for a tap outside the globe
#define WhirlyGlobeTapOutsideMsg @"WhirlyGlobeTapOutside"
/// This is the notification you get from a long press on the globe
#define WhirlyGlobeLongPressMsg @"WhirlyGlobeLongPress"

/** Tap Message is an
	indication that the user tapped on the globe.
	It's passed as the object in a notification.
    Note: Split this up a bit for globe vs. map
 */
@interface WhirlyGlobeTapMessage : NSObject

/// View that was touched
@property (nonatomic,retain) UIView *view;
//// Touch location on view in 2D
@property (nonatomic,assign) CGPoint touchLoc;
/// Lon/Lat
@property (nonatomic,assign) WhirlyKit::GeoCoord whereGeo;
/// 3D coordinates in the view
@property (nonatomic,assign) WhirlyKit::Point3f worldLoc;
/// Where the eye was.  0 is sea level.
@property (nonatomic,assign) float heightAboveSurface;

/// This version of set takes a set of doubles
- (void)setWorldLocD:(WhirlyKit::Point3d)newLoc;

@end
