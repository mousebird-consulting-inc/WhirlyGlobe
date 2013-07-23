/*
 *  MaplyActiveVectorObject.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/3/13.
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

#import "MaplyActiveObject.h"
#import "MaplyCoordinate.h"
#import "MaplyShape.h"
#import "MaplyVectorObject.h"

/** An Active Linear Vector Object is updated on the main thread.
    This means changes show up in the next frame.  You can use
    these for animation.
  */
@interface MaplyActiveLinearVectorObject : MaplyActiveObject

/// Replace the current set of points with what's in the vector.
/// We're expecting one linear feature and that's it.
- (void)setWithVector:(MaplyVectorObject *)newVecObj desc:(NSDictionary *)inDesc eps:(float)inEps;

@end
