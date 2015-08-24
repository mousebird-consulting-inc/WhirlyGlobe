/*
 *  MaplyIconManager.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/11/14.
 *  Copyright 2011-2014 mousebird consulting
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

/** An interface/loader for Icons in general.
 */
@interface MaplyIconManager : NSObject

// Fetch the simple UIImage for the icon with the given name
+ (nullable UIImage *)iconForName:(NSString *__nonnull)name size:(CGSize)size;

// Slightly more complex icon
+ (nullable UIImage *)iconForName:(NSString *__nullable)name size:(CGSize)size color:(UIColor *__nullable)color circleColor:(UIColor *__nullable)circleColor strokeSize:(float)strokeSize strokeColor:(UIColor *__nullable)strokeColor;

@end
