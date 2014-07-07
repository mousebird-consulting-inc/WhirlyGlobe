//
//  MaplyIconManager.h
//  WhirlyGlobe-MaplyComponent
//
//  Created by Steve Gifford on 1/11/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>

/** An interface/loader for Icons in general.
 */
@interface MaplyIconManager : NSObject

// Fetch the simple UIImage for the icon with the given name
+ (UIImage *)iconForName:(NSString *)name size:(CGSize)size;

// Slightly more complex icon
+ (UIImage *)iconForName:(NSString *)name size:(CGSize)size color:(UIColor *)color circleColor:(UIColor *)circleColor strokeSize:(float)strokeSize strokeColor:(UIColor *)strokeColor;

@end
