/*  MaplyStarsModel.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/4/15.
 *  Copyright 2011-2021 mousebird consulting
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
#import "MaplyComponentObject.h"
#import "WhirlyGlobeViewController.h"

/** 
    The Stars Model parses and then displays a star field.
    
     This is used to display a star field around the earth.
  */
@interface MaplyStarsModel : NSObject

/// Read stars from the given file
- (nullable instancetype)initWithFileName:(NSString *__nonnull)fileName;

/// Use the given image for each point.
/// The given image will be sampled for individual points.
- (void)setImage:(UIImage *__nonnull)image;

/** 
    Add stars to the given view controller
    
    Turn the star positions into geometry to display.  This object will track the resulting geometry objects.
    
    @param viewC The view controller to add the start geometry to.
    
    @param date The date for the 
    
    @param desc Additional parameters that may related to the geometry.
    
    @param mode Thread mode to use when adding the geometry.
  */
- (bool)addToViewC:(WhirlyGlobeViewController *__nonnull)viewC date:(NSDate *__nonnull)date desc:(NSDictionary *__nullable)desc mode:(MaplyThreadMode)mode;

/** 
    Remove star geometry from the registered view controller.
    
    Removes any objects created for the star geometry.
  */
- (void)removeFromViewC;

@end
