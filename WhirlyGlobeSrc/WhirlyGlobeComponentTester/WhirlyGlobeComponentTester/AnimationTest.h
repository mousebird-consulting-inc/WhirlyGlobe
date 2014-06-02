//
//  AnimationTest.h
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 7/31/13.
//  Copyright (c) 2013 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>
<<<<<<< HEAD
#import "MaplyComponent.h"
=======
#import "WhirlyGlobeComponent.h"
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

/** The animation test object runs a sphere around the globe
    over a defined time period.
  */
@interface AnimatedSphere : MaplyActiveObject

/// Initialize with period (amount of time for one orbit), radius and color of the sphere and a starting point
- (id)initWithPeriod:(float)period radius:(float)radius color:(UIColor *)color viewC:(MaplyBaseViewController *)viewC;

@end
