/*
 *  TestViewController.h
 *  MaplyComponentTester
 *
 *  Created by Steve Gifford on 9/7/12.
 *  Copyright 2012 mousebird consulting
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
#import "MaplyComponent.h"

// Various base layers we can start with
typedef enum {GeographyClassMBTilesLocal,StamenWatercolorRemote,OpenStreetmapRemote,MaxMaplyBaseLayers} MaplyTestBaseLayer;

/** The Test View Controller brings up a maply component
    and allows the user to interact with it.
 */
@interface TestViewController : UIViewController <MaplyViewControllerDelegate,UIPopoverControllerDelegate>
{
    MaplyViewController *mapViewC;
}

// Start with a given base layer
- (id)initWithBaseLayer:(MaplyTestBaseLayer)baseLayer;

@end
