/*
 *  WGVectorObject_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 8/2/12.
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

#import "MaplyVectorObject.h"
#import <WhirlyGlobe.h>

@interface MaplyVectorObject()

@property (nonatomic,readonly) WhirlyKit::ShapeSet &shapes;

@end

<<<<<<< HEAD
// Note: Porting
//@interface MaplyVectorDatabase() <WhirlyKitLoftedPolyCache>
//@interface MaplyVectorDatabase()
//
//@end
=======
@interface MaplyVectorDatabase() <WhirlyKitLoftedPolyCache>

@end
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
