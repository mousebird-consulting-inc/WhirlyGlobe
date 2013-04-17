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

/** An active vector object allows for changes
  */
@interface MaplyActiveLinearVectorObject : MaplyActiveObject

/// Replace the current set of points in the linear
- (void)setPoints:(MaplyCoordinate3d *)coords numPts:(int)numPts closed:(bool)isClosed;

/// Replace the current set of points with what's in the vector.
/// We're expecting one linear feature and that's it.
- (void)setFromVector:(MaplyVectorObject *)vecObj;

/// How close we're trying to get to the globe when subdividing.
/// 0 means don't bother breaking up the line
@property (nonatomic) float globeEps;

@end
