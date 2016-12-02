/*
 *  MaplyBaseViewController.h
 *  MaplyComponent
 *
 *  Created by Ranen Ghosh on 11/23/16.
 *  Copyright 2012-2016 mousebird consulting
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
#import <CoreLocation/CoreLocation.h>

#define LOC_TRACKER_POS_MARKER_SIZE 32

@class MaplyBaseViewController;

typedef enum {MaplyLocationLockNone, MaplyLocationLockNorthUp, MaplyLocationLockHeadingUp, MaplyLocationLockHeadingUpOffset} MaplyLocationLockType;

@protocol MaplyLocationTrackerDelegate

- (void) locationManager:(CLLocationManager * __nonnull)manager didFailWithError:(NSError * __nonnull)error;

- (void) locationManager:(CLLocationManager * __nonnull)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status;

@end

/* @brief The MaplyLocationTracker class provides support for showing current position and heading on the map or globe.
   @details Be sure to set NSLocationWhenInUseUsageDescription in your app's Info.plist before using.
 */
@interface MaplyLocationTracker : NSObject <CLLocationManagerDelegate>


- (nonnull instancetype)initWithViewC:(MaplyBaseViewController *__nullable)viewC Delegate:(NSObject<MaplyLocationTrackerDelegate> *__nullable)delegate useHeading:(bool)useHeading useCourse:(bool)useCourse lockType:(MaplyLocationLockType)lockType;

- (void) changeLockType:(MaplyLocationLockType)lockType;

- (void) teardown;

@end

