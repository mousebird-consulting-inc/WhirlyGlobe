/*
 *  TestViewController.h
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 7/23/12.
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
<<<<<<< HEAD
#import "MaplyComponent.h"
=======
#import "WhirlyGlobeComponent.h"
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
#import "ConfigViewController.h"

// Map or globe or startup
typedef enum {MaplyGlobe,MaplyGlobeWithElevation,Maply3DMap,Maply2DMap,MaplyNumTypes} MapType;

/** The Test View Controller brings up the WhirlyGlobe Component
    and allows the user to test various functionality.
 */
<<<<<<< HEAD
// Note: Porting
@interface TestViewController : UIViewController <MaplyViewControllerDelegate,UIPopoverControllerDelegate>
//@interface TestViewController : UIViewController <WhirlyGlobeViewControllerDelegate,MaplyViewControllerDelegate,UIPopoverControllerDelegate>
=======
@interface TestViewController : UIViewController <WhirlyGlobeViewControllerDelegate,MaplyViewControllerDelegate,UIPopoverControllerDelegate>
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
{
    /// This is the base class shared between the MaplyViewController and the WhirlyGlobeViewController
    MaplyBaseViewController *baseViewC;
    /// If we're displaying a globe, this is set
<<<<<<< HEAD
    // Note: Porting
//    WhirlyGlobeViewController *globeViewC;
=======
    WhirlyGlobeViewController *globeViewC;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    /// If we're displaying a map, this is set
    MaplyViewController *mapViewC;
    UIPopoverController *popControl;
}

// Fire it up with a particular base layer and map or globe display
- (id)initWithMapType:(MapType)mapType;

@end
